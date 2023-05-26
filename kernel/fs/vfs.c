#include <arch/cpu.h>
#include <arch/lock.h>
#include <arch/terminal.h>
#include <libc/kmalloc.h>
#include <libc/string.h>
#include <sys/misc.h>
#include <fs/vfs.h>

static spinlock_t vfs_lock = 0;
static vnode_t *v_root;

// List of active file systems
static fs_t file_systems[FS_MAX];
static int fs_count = 0;

// For debugging VFS directory tree
static void vfs_print_tree_internal(vnode_t *vnode, int cur_depth, int max_depth) {
    ASSERT_RET(cur_depth != max_depth,);

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

static vnode_t *vfs_get_child(vnode_t *parent, const char *name) {
    int num_children = VECTOR_SIZE(parent->v_children);

    for (int i = 0; i < num_children; i++) {
        vnode_t *node = VECTOR_GET(parent->v_children, i);
        int len = MAX(__strlen(name), __strlen(node->v_name));

        if ((__strlen(name) == __strlen(node->v_name)) &&
            (!__strncmp(name, node->v_name, len))) {
            return node;
        }
    }

    return NULL;
}

static int vfs_path_to_node(vnode_t **ret, vnode_t *v_base, char *path) {
    int err;

    if (__strlen(path) > VNODE_PATH_MAX) {
        return VFS_INVALID_PARAMETERS;
    }

    // Make copy of path
    char buff[VNODE_PATH_MAX];
    __strncpy(buff, path, __strlen(path));
    path = buff;

    // Empty string implies root
    if (__strlen(path) == 0) {
        bool is_root = v_base == v_root;
        if (is_root) {
            *ret = v_root;
            return VFS_SUCCESS;
        }

        return VFS_INVALID_PATH;
    }

    // Paths starting with '/' must beging with root
    if (*path == '/' && v_base != v_root) {
        return VFS_INVALID_PATH;
    }

    // Skip over first directory
    char *tok;
    if (*path != '/') {
        tok = __strtok_r(path, "/", &path);
        int len = MAX(__strlen(tok), __strlen(v_base->v_name));

        // First name in path does not match v_base
        if (__strncmp(tok, v_base->v_name, len)) {
            return VFS_INVALID_PATH;
        }
    }

    vnode_t *cur_vnode = v_base;
    if (!S_ISDIR(cur_vnode->stat.st_mode)) {
        return VFS_INVALID_PATH;
    }

    for (;;) {        
        tok = __strtok_r(path, "/", &path);
        
        // Path traversal complete, terminate
        if (!tok) {
            break; 
        }

        // Verify node is directory before traversing children
        if (!S_ISDIR(cur_vnode->stat.st_mode)) {
            return VFS_INVALID_PATH;
        }

        vnode_t *next_child = vfs_get_child(cur_vnode, tok);

        // Child not in VFS, call fs
        if (!next_child) {
            err = cur_vnode->vfsops->open(cur_vnode, cur_vnode->v_name);
            if (err) {
                return VFS_NODE_MISSING;
            }

            next_child = vfs_get_child(cur_vnode, tok);
        }

        cur_vnode = next_child;
    }

    *ret = cur_vnode;

    return VFS_SUCCESS;
}

void vfs_print_tree(vnode_t *parent, int max_depth) {
    spinlock_acquire(&vfs_lock);
    vfs_print_tree_internal(parent, 0, max_depth);
    spinlock_release(&vfs_lock);
}

int vfs_vtype_to_st_mode(int vtype) {
    switch (vtype) {
    case VREG: return S_IFREG;
    case VDIR: return S_IFDIR;
    case VSKT: return S_IFSOCK;
    case VBLK: return S_IFBLK;
    default: return 0;
    }
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

    // Default stat
    node->stat.st_mode = vfs_vtype_to_st_mode(v_type);

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

int vfs_mount(vnode_t **root, vnode_t *base, const char *path, const char *fs_name) {
    spinlock_acquire(&vfs_lock);

    fs_t *fs = vfs_get_filesystem(fs_name);
    if (fs == NULL) {
        spinlock_release(&vfs_lock);
        return VFS_FS_ERR;
    }
    
    // Get vnode where fs will be mounted
    vnode_t *mountpoint = NULL;
    int err = vfs_path_to_node(&mountpoint, base, (char *)path);
    if (err) {
        spinlock_release(&vfs_lock);
        return VFS_INVALID_PATH;
    }

    if (!S_ISDIR(mountpoint->stat.st_mode)) {
        spinlock_release(&vfs_lock);
        return VFS_INVALID_TYPE;
    }

    if (root) {
        *root = mountpoint;
    }

    // Now mount fs to mountpoint
    fs->vfsops->mount(mountpoint, NULL);
    spinlock_release(&vfs_lock);

    return VFS_SUCCESS;
}

int vfs_open(struct vnode **ret_vnode, vnode_t *base, const char *path) {
    vnode_t *vnode = NULL;

    spinlock_acquire(&vfs_lock);

    int err = vfs_path_to_node(&vnode, base, (char *)path);
    if (err) {
        spinlock_release(&vfs_lock);
        return err;
    }

    vnode->v_ref_count++;
    *ret_vnode = vnode;

    spinlock_release(&vfs_lock);

    return VFS_SUCCESS;
}

int vfs_close(vnode_t *vnode) {
    if (!vnode) {
        return VFS_INVALID_PARAMETERS;
    }

    spinlock_acquire(&vfs_lock);

    vnode->v_ref_count--;

    if (vnode->v_ref_count == 0) {
        vnode->vfsops->close(vnode);
    }

    spinlock_release(&vfs_lock);

    return VFS_SUCCESS;
}

int vfs_read(void *buff, vnode_t *node, size_t size, size_t offset, size_t *bytes_read) {
    if (!buff || !node || !bytes_read) {
        return VFS_INVALID_PARAMETERS;
    }

    spinlock_acquire(&vfs_lock);
    *bytes_read = node->vfsops->read(buff, node, size, offset);
    spinlock_release(&vfs_lock);

    return VFS_SUCCESS;
}

int vfs_write(void *buff, vnode_t *node, size_t size, size_t offset, size_t *bytes_written) {
    if (!buff || !node || !bytes_written) {
        return VFS_INVALID_PARAMETERS;
    }

    spinlock_acquire(&vfs_lock);
    *bytes_written = node->vfsops->write(buff, node, size, offset);
    spinlock_release(&vfs_lock);

    return VFS_SUCCESS;
}

int vfs_create(vnode_t *v_base, const char *vfs_path, int type) {
    ASSERT_RET(v_base && vfs_path, 0);

    spinlock_acquire(&vfs_lock);

    char dir_name[VNODE_NAME_MAX] = {0};
    if (!v_get_leaf_name(dir_name, vfs_path)) {
        spinlock_release(&vfs_lock);
        return VFS_INVALID_PATH;
    }
    
    char v_parent_path[VNODE_NAME_MAX] = {0};
    __strncpy(v_parent_path, vfs_path, __strlen(vfs_path) - __strlen(dir_name) - 1);
    vnode_t *v_parent = NULL;
    int err = vfs_path_to_node(&v_parent, v_base, v_parent_path);
    if (err) {
        spinlock_release(&vfs_lock);
        return err;
    }

    vnode_t *node = vfs_alloc_node(v_parent, dir_name, type);
    if (!node) {
        spinlock_release(&vfs_lock);
        return VFS_NO_MEM;
    }

    VECTOR_PUSH_BACK(v_parent->v_children, node);

    // vnodes not required to be mounted to fs
    if (v_parent->vfsops) {
        node->vfsops = v_parent->vfsops;
        node->vfsops->create(node);
    }

    spinlock_release(&vfs_lock);

    return VFS_SUCCESS; 
}

void vfs_init() {
    v_root = vfs_alloc_node(NULL, "", VDIR);
    ASSERT(v_root, 0, "VFS: could not initialize root\n");

    kprintf(INFO "VFS: Initialized\n");
}