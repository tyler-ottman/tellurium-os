#ifndef VFS_H
#define VFS_H

#include <libc/vector.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/stat.h>

#define VNODE_NAME_MAX                  64
#define VNODE_PATH_MAX                  512
#define FS_MAX                          10

#define VFS_SUCCESS                     0
#define VFS_OPEN_FAIL                   1
#define VFS_CREATE_FAIL                 2
#define VFS_INVALID_TYPE                3
#define VFS_INVALID_PARAMETERS          4
#define VFS_INVALID_PATH                5
#define VFS_NODE_MISSING                6
#define VFS_NODE_EXISTS                 7
#define VFS_CLOSE_FAIL                  8
#define VFS_NO_MEM                      9
#define VFS_NODE_UNMOUNTED              10
#define VFS_FS_ERR                      11

typedef struct fs {
    char fs_name[VNODE_NAME_MAX];
    struct vnode *root;
    struct vfsops *vfsops;
} fs_t;

enum VTYPE {VNON, VREG, VDIR, VBLK, VSKT};
typedef struct vnode {
    size_t v_ref_count;
    size_t v_lock_count;
    int v_type;
    struct vnode *v_mp;
    struct vnode *v_parent;
    VECTOR_DECLARE(v_children);
    char *v_name;
    struct vfsops *vfsops;
    struct stat stat;
    void *fs_data;
} vnode_t;

typedef struct vfsops {
    int (*mount)(vnode_t *mp, vnode_t *device);
    int (*open)(vnode_t *parent, const char *name);
    int (*close)(vnode_t *vnode);
    int (*read)(void *buff, vnode_t *node, size_t size, size_t offset);
    int (*write)(void *buff, vnode_t *node, size_t size, size_t offset);
    int (*create)(vnode_t *node);
} vfsops_t;

void vfs_print_tree(vnode_t *parent, int max_depth);
int vfs_vtype_to_st_mode(int vtype);
vnode_t *vfs_get_root(void);
vnode_t *vnode_create(vnode_t *parent, const char *v_name, int v_type);
int vfs_add_filesystem(const char *fs_name, vfsops_t *fs_ops);
fs_t *vfs_get_filesystem(const char *fs_name);

int vfs_mount(vnode_t **root, vnode_t *base, const char *path, const char *fs_name);
int vfs_open(vnode_t **vnode, vnode_t *base, const char *path);
int vfs_close(vnode_t *vnode);
int vfs_read(void *buff, vnode_t *node, size_t size, size_t offset, size_t *bytes_read);
int vfs_write(void *buff, vnode_t *node, size_t size, size_t offset, size_t *bytes_written);
int vfs_create(vnode_t *parent, const char *vfs_name, int type);

void vfs_init(void);

#endif // VFS_H