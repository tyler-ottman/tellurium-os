#include <arch/terminal.h>
#include <fs/tmpfs.h>
#include <libc/kmalloc.h>
#include <memory/pmm.h>

static int tmpfs_mount(vnode_t *mp, vnode_t *device) {
    fs_t *fs = vfs_get_filesystem("tmpfs");
    if (!fs) {
        return 0;
    }

    vnode_t *tmpfs_root = vnode_create(mp, "tmpfs_root", VDIR);
    if (!tmpfs_root) {
        return 0;
    }

    VECTOR_PUSH_BACK(tmpfs_root->v_parent->v_children, tmpfs_root);
    tmpfs_root->v_mp = mp;
    tmpfs_root->vfsops = fs->vfsops;

    return 1;
}

static int tmpfs_open(vnode_t *base, const char *path) {
    return 0;
}

static int tmpfs_close(vnode_t *node) {
    return 0;
}

static int tmpfs_read(void *buff, vnode_t *node, size_t size, size_t offset) {
    return 0;
}

static int tmpfs_write(void *buff, vnode_t *node, size_t size, size_t offset) {
    size_t new_size = (size + offset) / PAGE_SIZE_BYTES;
    
    return 0;
}

static int tmpfs_create(vnode_t *node) {
    return 0;
}

static vfsops_t tmpfs_ops = {
    tmpfs_mount,
    tmpfs_open,
    tmpfs_close,
    tmpfs_read,
    tmpfs_write,
    tmpfs_create
};

void tmpfs_init() {
    vfs_add_filesystem("tmpfs", &tmpfs_ops);
}

vfsops_t *get_tmpfs_ops() {
    return &tmpfs_ops;
}