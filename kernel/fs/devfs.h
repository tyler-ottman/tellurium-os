#ifndef DEVFS_H
#define DEVFS_H

#include <fs/vfs.h>

#define DEVFS_SUCCES                            0
#define DEVFS_DEVICE_INIT_ERR                   1

void devfs_init(void);
vfsops_t *devfs_get_ops(void);
int devfs_new_device(vnode_t **device, const char *name);

#endif // DEVFS_H