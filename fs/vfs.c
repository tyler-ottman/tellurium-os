#include <arch/lock.h>
#include <arch/terminal.h>
#include <libc/kmalloc.h>
#include <libc/string.h>
#include <fs/vfs.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))

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

static vnode_t *vfs_path_to_node(vnode_t *v_base, char *path) {
    if (__strlen(path) > VNODE_PATH_MAX) {
        return NULL;
    }

    // Place path on stack
    char buff[VNODE_PATH_MAX];
    __strncpy(buff, path, __strlen(path));
    path = buff;

    if (__strlen(path) == 0) {
        return v_base == v_root ? v_root : NULL;
    }

    if (*path == '/' && v_base != v_root) {
        return NULL;
    }

    // Skip over first directory
    char *tok;
    if (*path != '/') {
        tok = __strtok_r(path, "/", &path);
        int len = MAX(__strlen(tok), __strlen(v_base->v_name));
        if (__strncmp(tok, v_base->v_name, len)) {
            return NULL;
        }
    }

    vnode_t *cur_vnode = v_base;
    for (;;) {
        tok = __strtok_r(path, "/", &path);
        
        // Path traversal complete, terminate
        if (!tok) {
            break; 
        }

        // Check if name of child is present on current node
        vnode_t *next_child = NULL;
        int num_children = VECTOR_SIZE(cur_vnode->v_children);
        for (int i = 0; i < num_children; i++) {
            vnode_t *child = VECTOR_GET(cur_vnode->v_children, i);
            int len = MAX(__strlen(tok), __strlen(child->v_name));
            if ((__strlen(tok) == __strlen(child->v_name)) &&
                (!__strncmp(tok, child->v_name, len))) {
                next_child = VECTOR_GET(cur_vnode->v_children, i);
                break;
            }
        }

        if (!next_child) {
            return NULL; // TODO: create directories if they don't exist
        }

        cur_vnode = next_child;
    }

    return cur_vnode;
}

static vnode_t *vfs_get_mountpoint(vnode_t *v_base, char *path, char **relpath) {
    if (__strlen(path) > VNODE_PATH_MAX) {
        return NULL;
    }

    // Place path on stack
    char buff[VNODE_PATH_MAX];
    __strncpy(buff, path, __strlen(path));
    path = buff;
    if (!__strlen(path) || (*path == '/' && v_base != v_root)) {
        return NULL;
    }

    // Skip over first directory
    char *tok;
    if (*path != '/') {
        tok = __strtok_r(path, "/", &path);
        int len = MAX(__strlen(tok), __strlen(v_base->v_name));
        if (__strncmp(tok, v_base->v_name, len)) {
            return NULL;
        }
    }

    vnode_t *cur_vnode = v_base;
    vnode_t *cur_mp = NULL;
    for (;;) {
        tok = __strtok_r(path, "/", &path);
        
        // Path traversal complete, terminate
        if (!tok) {
            break; 
        }

        // Check if name of child is present on current node
        vnode_t *next_child = NULL;
        int num_children = VECTOR_SIZE(cur_vnode->v_children);
        for (int i = 0; i < num_children; i++) {
            vnode_t *child = VECTOR_GET(cur_vnode->v_children, i);
            int len = MAX(__strlen(tok), __strlen(child->v_name));
            if ((__strlen(tok) == __strlen(child->v_name)) &&
                (!__strncmp(tok, child->v_name, len))) {
                next_child = VECTOR_GET(cur_vnode->v_children, i);
                if (next_child->v_mp) {
                    cur_mp = next_child->v_mp;
                }
                break;
            }
        }

        if (!next_child) {
            break; // TODO: create directories if they don't exist
        }

        cur_vnode = next_child;
    }

    // Copy the rest of the unexplored path to relpath
    __strncpy(*relpath, path, __strlen(path));

    return cur_mp;
}

void vfs_print_tree(vnode_t *parent, int max_depth) {
    spinlock_acquire(&vfs_lock);
    vfs_print_tree_internal(parent, 0, max_depth);
    spinlock_release(&vfs_lock);
}

struct vnode *vfs_get_root(void) {
    return v_root;
}

vnode_t *vfs_alloc_node(vnode_t *parent, const char *v_name, int v_type) {
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

int vfs_mount(vnode_t *base, const char *path, const char *fs_name) {
    spinlock_acquire(&vfs_lock);

    fs_t *fs = vfs_get_filesystem(fs_name);
    if (fs == NULL) {
        goto vfs_mount_fail;
    }
    
    // Get vnode where fs will be mounted
    vnode_t *mountpoint = vfs_path_to_node(base, (char *)path);
    if (!mountpoint || mountpoint->v_type != VDIR) {
        goto vfs_mount_fail;
    }

    // Now mount fs to mountpoint
    fs->vfsops->mount(mountpoint, NULL);
    spinlock_release(&vfs_lock);

    return 1;

vfs_mount_fail:
    spinlock_release(&vfs_lock);
    return 0;
}

int vfs_open(struct vnode **ret_vnode, vnode_t *base, const char *path) {
    spinlock_acquire(&vfs_lock);

    char buff[VNODE_PATH_MAX];
    vnode_t *vnode = vfs_path_to_node(base, (char *)path);
    
    // Node not present in VFS, consult lower level FS to query existence of node
    if (vnode == NULL) {
        char *relpath = buff;
        vnode_t *root = vfs_get_mountpoint(base, (char *)path, &relpath);
        root->vfsops->open(root, path); // TODO: pass relative path to root
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

int vfs_create(vnode_t *v_base, const char *vfs_path, int type) {
    spinlock_acquire(&vfs_lock);

    char dir_name[VNODE_NAME_MAX] = {0};
    if (!v_get_leaf_name(dir_name, vfs_path)) {
        goto vfs_create_fail;
    }
    
    char v_parent_path[VNODE_NAME_MAX] = {0};
    __strncpy(v_parent_path, vfs_path, __strlen(vfs_path) - __strlen(dir_name) - 1);
    vnode_t *v_parent = vfs_path_to_node(v_base, v_parent_path);
    if (!v_parent) {
        goto vfs_create_fail;
    }

    vnode_t *node = vfs_alloc_node(v_parent, dir_name, type);
    if (!node) {
        goto vfs_create_fail;
    }

    VECTOR_PUSH_BACK(v_parent->v_children, node);

    // vnodes not required to be mounted to fs
    if (!v_parent->vfsops) {
        goto vfs_create_fail;
    }

    node->vfsops = v_parent->vfsops;
    node->vfsops->create(node);

    spinlock_release(&vfs_lock);
    return 1;

vfs_create_fail:
    spinlock_release(&vfs_lock);
    return 0;  
}

void vfs_init() {
    v_root = vfs_alloc_node(NULL, "", VDIR);
    if (!v_root) {
        kerror("VFS: could not initialize root\n");
    }
    kprintf(INFO "VFS: Initialized\n");
}