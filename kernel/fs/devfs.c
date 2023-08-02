#include <arch/lock.h>
#include <flibc/string.h>
#include <fs/devfs.h>
#include <klib/kmalloc.h>
#include <klib/ringbuffer.h>
#include <sys/misc.h>

#define DEVFS_MAX_BUFFER            0x1000

static vnode_t *devfs_root = NULL;
static size_t dev_id = 0;

static int devfs_mount(vnode_t *mp, vnode_t *device) {
    (void)device;
    
    mp->v_mp = mp;
    mp->vfsops = devfs_get_ops();

    return 1;
}

static int devfs_open(vnode_t *parent, const char *name) {
    (void)parent;
    (void)name;

    return VFS_OPEN_FAIL;
}

static int devfs_close(vnode_t *node) {
    (void)node;

    return VFS_CLOSE_FAIL;
}

static int devfs_read(void *buff, vnode_t *node, size_t size, size_t offset) {
    (void)offset;

    // ringbuffer_t *ring = (ringbuffer_t *)node->fs_data;

    // size_t count = RINGBUFFER_READ((*ring), buff, size);

    // node->stat.st_size -= count;

    return 1;
}

static int devfs_write(void *buff, vnode_t *node, size_t size, size_t offset) {
    (void)offset;

    ringbuffer_t *ring = (ringbuffer_t *)node->fs_data;

    size_t count = RINGBUFFER_WRITE((*ring), buff, size);

    node->stat.st_size += count;

    return count;
}

static int devfs_create(vnode_t *node) {
    void *data = kmalloc(PAGE_SIZE_BYTES);
    if (!data) {
        return 0;
    }
    
    node->fs_data = data;
    node->stat.st_blksize = PAGE_SIZE_BYTES;
    node->stat.st_blocks = 1;
    node->stat.st_size = node->stat.st_blksize;
    node->stat.st_mode = vfs_vtype_to_st_mode(node->v_type);

    return 1;
}

static vfsops_t devfs_ops = {
    devfs_mount,
    devfs_open,
    devfs_close,
    devfs_read,
    devfs_write,
    devfs_create
};

void devfs_init() {
    if (devfs_root) {
        return;
    }

    vfs_add_fs_meta("devfs", &devfs_ops);

    vfs_create(vfs_get_root(), "/dev", VDIR);

    vfs_mount(&devfs_root, vfs_get_root(), "/dev", "devfs");
}

vfsops_t *devfs_get_ops() {
    return &devfs_ops;
}

int devfs_new_device(vnode_t **device, const char *name, vfsops_t *devops) {
    ASSERT_RET(name && devfs_root && devops, DEVFS_DEVICE_INIT_ERR);

    size_t name_len = __strlen(name);
    ASSERT_RET((name_len < VNODE_NAME_MAX) && (name_len > 0),
               DEVFS_DEVICE_INIT_ERR);

    char dev_path[VNODE_PATH_MAX] = "dev/";
    __strncpy(&dev_path[__strlen(dev_path)], name, __strlen(name));

    vnode_t *dev;
    int err = vfs_open(&dev, devfs_root, dev_path);
    if (!err) {
        return VFS_NODE_EXISTS;
    }

    err = vfs_create(devfs_root, dev_path, VREG);
    if (err) {
        return err;
    }
    
    err = vfs_open(&dev, devfs_root, dev_path);
    if (err) {
        return err;
    }
    
    if (device) {
        *device = dev;
    }

    fs_node_t *fs;
    err = vfs_get_fs_node(&fs, devfs_root);
    if (err) {
        return err;
    }

    dev->vfsops = devops;

    dev->stat.st_ino = fs->inode_count++;
    dev->stat.st_dev = fs->fs_id;
    dev->stat.st_rdev = dev_id++;

    return 0;
}