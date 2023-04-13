#include <arch/lock.h>
#include <arch/terminal.h>
#include <libc/kmalloc.h>
#include <libc/string.h>
#include <fs/vfs.h>

static spinlock_t vfs_lock = 0;
static vnode_t *v_root;

// List of active file systems
static fs_t file_systems[FS_MAX];
static int fs_count = 0;

// For debugging VFS directory tree
static void vfs_print_tree_internal(vnode_t *vnode, int cur_depth, int max_depth) {
    if (cur_depth == max_depth) {
        return;
    }

    for (int i = 0; i < cur_depth; i++) {
        kprintf("\t");
    }

    if (vnode == v_root) {
        kprintf("root\n");
    } else {
        kprintf("%s\n", vnode->v_name);
    }
    
    if (vnode->v_type != VDIR) {
        return;
    }

    int num_children = VECTOR_SIZE(vnode->v_children);
    for (int i = 0; i < num_children; i++) {
        vnode_t *child = VECTOR_GET(vnode->v_children, i);
        vfs_print_tree_internal(child, cur_depth + 1, max_depth);
    }
}

static int v_get_leaf_name(char *leaf_name, const char *path) {
    int index = -1;
    int path_len = __strlen(path);
    if (path_len > VNODE_PATH_MAX || path_len <= 0) {
        return 0;
    }
    for (int i = 0; i < path_len; i++) {
        if (path[i] == '/') {
            index = i;
        }
    }

    int leaf_name_len = path_len - (index++ + 1);
    if (leaf_name_len > VNODE_NAME_MAX - 1) {
        return 0;
    }
    __strncpy(leaf_name, &path[index], leaf_name_len);

    return 1;
}

static vnode_t *vnode_from_path(vnode_t *v_base, const char *path) {
    // Prune and verify base vnode
    int index = 0;
    for (size_t i = 0; i < __strlen(path); i++) {
        if (path[i] == '/') {
            index = i;
            break;
        }
    }

    // First directory in path doesn't match base vnode's name
    if (__strncmp(v_base->v_name, path, index) != 0) {
        return NULL;
    }

    if (__strlen(path) == 0) {
        return ((v_base == v_root) ? v_root : NULL);
    }

    vnode_t *cur_vnode = v_base;
    const char *path_base = path + ++index;

    for (;;) {
        // Path traversal complete, terminate
        if (path_base >= (path + __strlen(path))) {
            break;
        }

        // Get next node name
        char v_name[VNODE_NAME_MAX] = {0};
        int index = 0;
        for (size_t i = 0; i < __strlen(path_base); i++) {
            if (path_base[i] == '/') {
                index = i;
                break;
            }
        }

        // Last vnode in path
        if (index == 0) {
            index = __strlen(path_base);
        }

        __memcpy(v_name, path_base, index);

        // Check if name of child is present on current node
        vnode_t *next_child = NULL;
        int num_children = VECTOR_SIZE(cur_vnode->v_children);
        for (int i = 0; i < num_children; i++) {
            vnode_t *child = VECTOR_GET(cur_vnode->v_children, i);
            if (__strncmp(v_name, child->v_name, __strlen(v_name)) == 0) {
                next_child = VECTOR_GET(cur_vnode->v_children, i);
                break;
            }
        }

        if (next_child == NULL) {
            return NULL; // TODO: create directories if they don't exist
        }

        // Update path pointer and current vnode
        path_base += __strlen(v_name) + 1;
        cur_vnode = next_child;
    }

    return cur_vnode;
}

void vfs_print_tree(vnode_t *parent, int max_depth) {
    spinlock_acquire(&vfs_lock);
    vfs_print_tree_internal(parent, 0, max_depth);
    spinlock_release(&vfs_lock);
}

struct vnode *vfs_get_root(void) {
    return v_root;
}

vnode_t *vnode_create(vnode_t *parent, const char *v_name, int v_type) {
    vnode_t *node = kmalloc(sizeof(vnode_t));
    if (!node) {
        return NULL;
    }

    node->v_ref_count = 0;
    node->v_lock_count = 0;
    node->v_type = v_type;
    node->v_mp = NULL;
    node->v_parent = parent;
    if (node->v_type == VDIR) {
        VECTOR_ALLOC(node->v_children);
    }

    int v_name_len = __strlen(v_name);
    node->v_name = kmalloc(v_name_len + 1);
    __strncpy(node->v_name, v_name, v_name_len);

    node->fs_data = NULL;

    return node;
}

int vfs_create(vnode_t *v_base, const char *vfs_path) {
    spinlock_acquire(&vfs_lock);

    char dir_name[VNODE_NAME_MAX] = {0};
    if (!v_get_leaf_name(dir_name, vfs_path)) {
        goto vfs_create_fail;
    }

    char v_parent_path[VNODE_NAME_MAX] = {0};
    __strncpy(v_parent_path, vfs_path, __strlen(vfs_path) - __strlen(dir_name) - 1);
    vnode_t *v_parent = vnode_from_path(v_base, v_parent_path);
    if (!v_parent) {
        goto vfs_create_fail;
    }


    vnode_t *node = vnode_create(v_parent, dir_name, VDIR);
    if (!node) {
        return 0;
    }

    VECTOR_PUSH_BACK(v_parent->v_children, node);

    // vnodes not required to be mounted to fs
    if (v_parent->vfsops) {
        v_parent->vfsops->create(node);
    }

    spinlock_release(&vfs_lock);
    return 1;

vfs_create_fail:
    spinlock_release(&vfs_lock);
    return 0;  
}

vnode_t *vfs_mount(vnode_t *base, const char *path, const char *fs_name) {
    spinlock_acquire(&vfs_lock);

    fs_t *fs = vfs_get_filesystem(fs_name);
    if (fs == NULL) {
        goto vfs_mount_fail;
    }
    
    // Get vnode where fs will be mounted
    vnode_t *mountpoint = vnode_from_path(base, path);
    if (!mountpoint || mountpoint->v_type != VDIR) {
        goto vfs_mount_fail;
    }

    // Now mount fs to mountpoint
    fs->vfsops->mount(mountpoint, NULL);
    spinlock_release(&vfs_lock);

    return mountpoint;

vfs_mount_fail:
    spinlock_release(&vfs_lock);
    return NULL;
}

int vfs_add_filesystem(const char *fs_name, vfsops_t *fs_ops) {
    if (fs_count == FS_MAX || __strlen(fs_name) > VNODE_NAME_MAX) {
        return 0;
    }

    fs_t *fs = &file_systems[fs_count++];
    __strncpy(fs->fs_name, fs_name, __strlen(fs_name));
    fs->vfsops = fs_ops;

    return 1;
}

fs_t *vfs_get_filesystem(const char *fs_name) {
    for (int i = 0; i < fs_count; i++) {
        fs_t *fs = &file_systems[i];
        if (__strncmp(fs->fs_name, fs_name, __strlen(fs->fs_name)) == 0) {
            return fs;
        }
    }
    return NULL;
}

int vfs_open(struct vnode **ret_vnode, vnode_t *base, const char *path) {
    spinlock_acquire(&vfs_lock);

    vnode_t *vnode = vnode_from_path(base, path);
    if (vnode == NULL) {
        spinlock_release(&vfs_lock);
        return 0;
    }

    vnode->v_ref_count++;
    *ret_vnode = vnode;

    spinlock_release(&vfs_lock);
    return 1;
}

int vfs_close(vnode_t *vnode) {
    if (!vnode) {
        return 0;
    }

    spinlock_acquire(&vfs_lock);

    vnode->v_ref_count--;

    if (vnode->v_ref_count == 0) {
        vnode->vfsops->close(vnode);
    }

    spinlock_release(&vfs_lock);
    return 1;
}

int vfs_read(void *buff, vnode_t *node, size_t size, size_t offset) {
    if (!buff || !node) {
        return 0;
    }

    spinlock_acquire(&vfs_lock);
    int res = node->vfsops->read(buff, node, size, offset);
    spinlock_release(&vfs_lock);

    return res;
}

int vfs_write(void *buff, vnode_t *node, size_t size, size_t offset) {
    if (!buff || !node) {
        return 0;
    }

    spinlock_acquire(&vfs_lock);
    int res = node->vfsops->write(buff, node, size, offset);
    spinlock_release(&vfs_lock);

    return res;
}

void vfs_init() {
    v_root = vnode_create(NULL, "", VDIR);
    if (!v_root) {
        kerror("VFS: could not initialize root\n");
    }
    kprintf(INFO "VFS: Initialized\n");
}