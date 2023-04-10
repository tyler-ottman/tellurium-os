#include <arch/lock.h>
#include <libc/kmalloc.h>
#include <libc/string.h>
#include <vfs/vfs.h>

static vnode_t *v_root;
spinlock_t vfs_lock = 0;

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
    __memcpy(node->v_name, v_name, v_name_len);

    node->fs_data = NULL;

    return node;
}

vnode_t *vfs_create(vnode_t *parent, const char *vfs_name) {
    vnode_t *vfs_node = vnode_create(parent, vfs_name, VDIR);
    
    spinlock_acquire(&vfs_lock);

    vfs_node->v_parent = parent;
    VECTOR_PUSH_BACK(parent->v_children, vfs_node);

    return vfs_node;
}

void vfs_init() {
    v_root = vnode_create(NULL, "", VDIR);
}