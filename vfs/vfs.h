#ifndef VFS_H
#define VFS_H

#include <libc/vector.h>
#include <stddef.h>
#include <stdint.h>

#define VNODE_NAME_MAX                  64
#define VNODE_PATH_MAX                  512

enum VTYPE {VNON, VREG, VDIR, VBLK};
typedef struct vnode {
    size_t v_ref_count;
    size_t v_lock_count;
    int v_type;
    struct vnode *v_mp;
    struct vnode *v_parent;
    VECTOR_DECLARE(v_children);
    char *v_name;
    void *fs_data;
} vnode_t;

void vfs_print_tree(struct vnode *parent, int max_depth);
struct vnode *vfs_get_root(void);
struct vnode *vnode_create(struct vnode *parent, const char *v_name, int v_type);
struct vnode *vfs_create(struct vnode *parent, const char *vfs_name);

void vfs_init(void);

#endif // VFS_H