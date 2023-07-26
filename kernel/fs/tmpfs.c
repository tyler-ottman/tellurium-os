#include <arch/terminal.h>
#include <fs/tmpfs.h>
#include <libc/kmalloc.h>
#include <libc/string.h>
#include <libc/vector.h>
#include <memory/pmm.h>
#include <sys/misc.h>

#define TMAGIC              "ustar"
#define TMAGLEN             6

typedef struct tar_header {
    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char chksum[8];
    char typeflag;
    char linkname[100];
    char magic[6];
    char version[2];
    char uname[32];
    char gname[32];
    char devmajor[8];
    char devminor[8];
    char prefix[155];
} tar_header_t;

#define TAR_REGTYPE  '0'

static volatile struct limine_module_request module_request = {
    .id = LIMINE_MODULE_REQUEST,
    .revision = 0
};

static int tmpfs_mount(vnode_t *mp, vnode_t *device) {
    (void)device;

    mp->v_mp = mp;
    mp->vfsops = get_tmpfs_ops();

    return VFS_SUCCESS;
}

static int tmpfs_open(vnode_t *parent, const char *name) {
    (void)parent;
    (void)name;

    return VFS_OPEN_FAIL;
}

static int tmpfs_close(vnode_t *node) {
    (void)node;

    return VFS_CLOSE_FAIL;
}

static int tmpfs_read(void *buff, vnode_t *node, size_t size, size_t offset) {
    if (offset + size >= (size_t)node->stat.st_size) {
        size = node->stat.st_size - offset;
    }
    
    uint64_t addr = (uint64_t)node->fs_data + offset;
    __memcpy(buff, (void *)addr, size);

    return size;
}

static int tmpfs_write(void *buff, vnode_t *node, size_t size, size_t offset) {
    size_t file_head_max = size + offset;
    size_t file_block_size = node->stat.st_blksize;
    size_t new_block_size = file_head_max / file_block_size;
    if (file_head_max % file_block_size != 0) {
        new_block_size++;
    }

    if (new_block_size > (size_t)node->stat.st_blocks) {
        void *data = krealloc(node->fs_data, new_block_size * PAGE_SIZE_BYTES);
        if (!data) {
            return VFS_NO_MEM;
        }

        node->fs_data = data;
        node->stat.st_blocks = new_block_size;
        node->stat.st_size = offset + size;
    }

    uint64_t addr = (uint64_t)node->fs_data + offset;
    __memcpy((void *)addr, buff, size);
    if (offset + size > (size_t)node->stat.st_size) {
        node->stat.st_size = offset + size;
    }

    return VFS_SUCCESS;
}

static int tmpfs_create(vnode_t *node) {
    void *data = kmalloc(PAGE_SIZE_BYTES);
    if (!data) {
        return VFS_NO_MEM;
    }

    node->fs_data = data;
    node->stat.st_blksize = PAGE_SIZE_BYTES;
    node->stat.st_blocks = 1;
    node->stat.st_size = node->stat.st_blksize;
    node->stat.st_mode = vfs_vtype_to_st_mode(node->v_type);

    return VFS_SUCCESS;
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
    vfs_add_fs_meta("tmpfs", &tmpfs_ops);
}

uint64_t octal_to_int(char *src, size_t n) {
    uint64_t ret = 0;
    while (*src && n-- > 0) {
        ret = ret * 8 + (*src++ - '0');
    }
    return ret;
}

void tmpfs_load_userapps() {
    struct limine_module_response *mod_response = module_request.response;
    if (!mod_response) {
        kprintf(INFO "No modules to load\n");
        return;
    }
    
    kprintf(INFO "%d file(s) present\n", mod_response->module_count);

    for (size_t i = 0; i < mod_response->module_count; i++) {
        struct limine_file *file = mod_response->modules[i];
        tar_header_t *tar_file = (tar_header_t *)file->address;
        
        // Check if module is tar file
        if (__strncmp(tar_file->magic, TMAGIC, TMAGLEN - 1)) {
            continue;
        }

        while (!__strncmp(tar_file->magic, TMAGIC, TMAGLEN - 1)) {
            char *app_name = tar_file->name;
            if (!__strncmp(app_name, "./", __strlen("./")) ||
                tar_file->typeflag != TAR_REGTYPE) {
                continue;
            }
            
            const char err_msg[] = "app load failure";

            // Add ELF to tmpfs
            char file_path[VNODE_NAME_MAX] = "/tmp/";
            __strncpy(file_path + __strlen(file_path), app_name, __strlen(app_name));
            int err = vfs_create(vfs_get_root(), file_path, VREG);
            ASSERT(!err, err, err_msg);

            vnode_t *node;
            err = vfs_open(&node, vfs_get_root(), file_path);
            ASSERT(!err, err, err_msg);
            
            int size = octal_to_int(tar_file->size, sizeof(tar_file->size));
            uint64_t addr = (uint64_t)tar_file + 512;

            size_t bytes_written;
            err = vfs_write((void *)addr, node, size, 0, &bytes_written);
            ASSERT(!err, err, err_msg);

            int num_blocks = size / 512;
            if (num_blocks % 512 != 0) {
                num_blocks++;
            }
            addr = (uint64_t)tar_file + 512 * (1 + num_blocks);
            tar_file = (void *)addr;
        }
    }
}

vfsops_t *get_tmpfs_ops() {
    return &tmpfs_ops;
}