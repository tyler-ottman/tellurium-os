#ifndef TMPFS_H
#define TMPFS_H

#include <fs/vfs.h>

void tmpfs_init(void);
vfsops_t *get_tmpfs_ops(void);

#endif // TMPFS_H