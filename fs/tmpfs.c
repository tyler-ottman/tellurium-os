#include <arch/terminal.h>
#include <fs/tmpfs.h>
#include <libc/kmalloc.h>
#include <libc/vector.h>
#include <memory/pmm.h>

static int tmpfs_mount(vnode_t *mp, vnode_t *device) {
    vnode_t *tmpfs_root = vnode_create(mp, "tmpfs_root", VDIR);
    if (!tmpfs_root) {
        return 0;
    }

    VECTOR_PUSH_BACK(tmpfs_root->v_parent->v_children, tmpfs_root);
    tmpfs_root->v_mp = mp;
    tmpfs_root->vfsops = get_tmpfs_ops();

    return 1;
}

static int tmpfs_open(vnode_t *base, const char *path) {
    return 0;
}

static int tmpfs_close(vnode_t *node) {
    return 0;
}

static int tmpfs_read(void *buff, vnode_t *node, size_t size, size_t offset) {
    if (offset + size > node->stat.st_size) {
        size = node->stat.st_size - offset;
    }

    __memcpy(buff, node->fs_data + offset, size);

    return 1;
}

static int tmpfs_write(void *buff, vnode_t *node, size_t size, size_t offset) {
    size_t new_block_size = (size + offset) / node->stat.st_blksize;
    
    if (new_block_size >= node->stat.st_blksize) {
        void *data = krealloc(node->fs_data, new_block_size + 1);
        if (!data) {
            return 0;
        }
        node->fs_data = data;
        node->stat.st_blocks = new_block_size + 1;
        node->stat.st_size = offset + size;
    }

    __memcpy(node->fs_data + offset, buff, size);
    if (offset + size > node->stat.st_size) {
        node->stat.st_size = offset + size;
    }

    return 1;
}

static int tmpfs_create(vnode_t *node) {
    void *data = palloc(1);
    if (!data) {
        return NULL;
    }

    node->fs_data = data;
    node->stat.st_blksize = PAGE_SIZE_BYTES;
    node->stat.st_blksize = 1;

    return 1;
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