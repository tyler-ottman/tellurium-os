#include <arch/lock.h>
#include <arch/terminal.h>
#include <libc/kmalloc.h>
#include <libc/string.h>
#include <vfs/vfs.h>

static vnode_t *v_root;
spinlock_t vfs_lock = 0;

// For debugging VFS directory tree
static void vfs_print_tree_internal(vnode_t *vnode, int cur_depth, int max_depth) {
    if (cur_depth == max_depth) {
        return;
    }

    for (int i = 0; i < cur_depth; i++) {
        kprintf("\t");
    }

    kprintf("%s\n", vnode->v_name);
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
    __memcpy(leaf_name, &path[index], leaf_name_len);
    leaf_name[leaf_name_len] = '\0';

    return 1;
}

static vnode_t *vnode_from_path(vnode_t *v_base, const char *path) {
    char v_name[VNODE_NAME_MAX] = {0};

    for (;;) {
        // Get next vnode
    }

    return NULL;
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
    __memcpy(node->v_name, v_name, v_name_len);
    node->v_name[v_name_len] = '\0';

    node->fs_data = NULL;

    return node;
}

vnode_t *vfs_create(vnode_t *v_base, const char *vfs_path) {
    spinlock_acquire(&vfs_lock);

    // Get name of VFS from path
    char vfs_name[VNODE_NAME_MAX] = {0};
    if (!v_get_leaf_name(vfs_name, vfs_path)) {
        goto vfs_create_fail;
    }

    // Get vnode from path
    char v_parent_path[VNODE_NAME_MAX] = {0};
    __memcpy(v_parent_path, vfs_path, __strlen(vfs_path) - __strlen(vfs_name) - 1);
    vnode_t *v_parent = vnode_from_path(v_base, v_parent_path);

    vnode_t *vfs_node = vnode_create(v_parent, vfs_name, VDIR);
    VECTOR_PUSH_BACK(v_parent->v_children, vfs_node);
    spinlock_release(&vfs_lock);

    return vfs_node;

vfs_create_fail:
    spinlock_release(&vfs_lock);
    return NULL;
}

void vfs_init() {
    v_root = vnode_create(NULL, "", VDIR);
    if (!v_root) {
        kerror("VFS: could not initialize root\n");
    }
}