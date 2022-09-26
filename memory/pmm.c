#include <memory/pmm.h>

static volatile struct limine_memmap_request memory_map_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0
};

// AVL internals
static int avl_size;
static int bitmap_size_bytes;
static int cur_entry_idx;
struct mmap_range_node* base;
struct mmap_range_node* avl_nodes;

// PMM Internals
uint64_t available_frames = 0;

void init_pmm(void) {
    // Get memory map
    struct limine_memmap_response* memory_map_response = memory_map_request.response;
    struct limine_memmap_entry** entries = memory_map_response->entries;

    // Print number of memory map entries
    uint64_t mmap_entries = memory_map_response->entry_count;
    terminal_printf("Limine -> MMap entries: %u\n", mmap_entries);  
    
    // Calculate total memory available for page frame allocation
    for (uint64_t idx = 0; idx < mmap_entries; idx++) {
        struct limine_memmap_entry* mmap_entry = entries[idx];
        uint64_t base = mmap_entry->base;
        uint64_t length = mmap_entry->length;
        uint64_t type = mmap_entry->type;

        // Usable mmap sections used for page frame allocator
        if (type == LIMINE_MEMMAP_USABLE) {
            // Calculate total frame kernel can use
            available_frames += length / PAGE_SIZE_BYTES;
        }

        // Mmap section details
        terminal_printf("Base: %016x, Len: %16x, Type: %u\n", base, length, type);
    }

    terminal_printf("Available page frames: %u\n", available_frames);
    
    init_bitmap_avl(entries, mmap_entries);

    terminal_printf("AVL initialized\n");
    print_avl();
}

void init_bitmap_avl(struct limine_memmap_entry** entries, int size) {
    cur_entry_idx = -1;

    // Calculate size of AVL
    avl_size = 0;
    for (int idx = 0; idx < size; idx++) {
        if (entries[idx]->type == LIMINE_MEMMAP_USABLE) {
            avl_size++;
        }
    }

    // Calculate how much space is needed for avl entries
    uint64_t avl_byte_size = avl_size * sizeof(struct mmap_range_node);
    terminal_printf("%u entries in AVL, total size: %u\n", avl_size, avl_byte_size);

    // Find and allocate space for AVL structure
    for (int idx = 0; idx < size; idx++) {
        struct limine_memmap_entry* mmap_entry = entries[idx];
        if (mmap_entry->type != LIMINE_MEMMAP_USABLE) {
            continue;
        }

        if (avl_byte_size < mmap_entry->length) {
            avl_nodes = (struct mmap_range_node*)(mmap_entry->base);
            mmap_entry->base += avl_byte_size;
            mmap_entry->length -= avl_byte_size;
            break;
        }
    }

    // Calculate total size of all bitmaps
    // bitmap_size_bytes = 0;
    // for (int idx = 0; idx < size; idx++) {
    //     struct limine_memmap_entry* mmap_entry = entries[idx];
    // }

    // // Find and allocate space for bitmap structures
    // bitmap_size_bytes = avl_size * sizeof(struct bitmap);
    // for (int idx = 0; idx < size; idx++) {
    //     struct limine_memmap_entry* mmap_entry = entries[idx];
    //     if (mmap_entry->type != LIMINE_MEMMAP_USABLE) {
    //         continue;
    //     }
    // }

    // Initialize AVL nodes to equilvalent NULL
    struct mmap_range_node* avl_ptr = avl_nodes;
    for (int idx = 0; idx < avl_size; idx++) {
        avl_ptr->interval.base = 0;
        avl_ptr->interval.high = 0;
        avl_ptr->left = NULL;
        avl_ptr->right = NULL;
        avl_ptr->section_mmap = NULL; // Todo: change to point to real bitmap
        avl_ptr++;
    }

    // Insert mmap sections into AVL
    for (int idx = 0; idx < size; idx++) {
        struct limine_memmap_entry* mmap_entry = entries[idx];
        if (mmap_entry->type != LIMINE_MEMMAP_USABLE) {
            continue;
        }

        // Insert usable mmap section into avl
        insert(mmap_entry->base, mmap_entry->base + mmap_entry->length);
    }
}

struct mmap_range_node* allocate_node(uint64_t base_address, uint64_t high_address) {
    // (Failsafe) Check that AVL can support allocation of additional node
    if (cur_entry_idx + 1 >= avl_size) {
        terminal_printf("pmm: Error, AVL node overflow\n");
        return NULL;
    }

    cur_entry_idx++;

    struct mmap_range_node* avl_node = avl_nodes++;
    avl_node->interval.base = base_address;
    avl_node->interval.high = high_address;
    avl_node->left = NULL;
    avl_node->right = NULL;

    return avl_node;
}

int get_node_level(struct mmap_range_node* base) {
    if (!base) {
        return 0;
    }
    return max(get_node_level(base->left), get_node_level(base->right)) + 1;
}

int get_balance_factor(struct mmap_range_node* base) {
    if (!base) {
        return 0;
    }
    return (get_node_level(base->left) - get_node_level(base->right));
}

/*
    1              2
   / \            / \
  2   S   --->   3   1
 / \                / \
3   S              S   S
*/
struct mmap_range_node* rotate_right(struct mmap_range_node* node) {
    struct mmap_range_node* node_left = node->left;
    struct mmap_range_node* node_left_right_subtree = node_left->right;

    node_left->right = node;
    node->left = node_left_right_subtree;

    return node_left;
}

/* 
  1                2
 / \              / \
S   2     --->   1   3
   / \          / \
  S   3        S   S
*/
struct mmap_range_node* rotate_left(struct mmap_range_node* node) {
    struct mmap_range_node* node_right = node->right;
    struct mmap_range_node* node_right_left_subtree = node_right->left;

    node_right->left = node;
    node->right = node_right_left_subtree;

    return node_right;
}

struct mmap_range_node* insert_internal(struct mmap_range_node* cur_node, uint64_t base, uint64_t high) {
    if (!cur_node) {
        return allocate_node(base, high);
    }
    
    // Check interval of current node to determine if left or right insertion
    if (high < cur_node->interval.base) {
        // Recurse left child
        cur_node->left = insert_internal(cur_node->left, base, high);
        if (!cur_node->left) {
            terminal_printf("pmm: Illegal interval\n");
            return NULL;
        }
    } else if (base > cur_node->interval.high) {
        // Recurse right child
        cur_node->right = insert_internal(cur_node->right, base, high);
        if (!cur_node->right) {
            terminal_printf("pmm: Illegal interval\n");
            return NULL;
        }
    } else {
        // Overlapping interval, illegal
        terminal_printf("pmm: Error, illegal mmap section intervals\n");
        return NULL;
    }

    // Check balance factor to see if node needs to be rotated
    int balance_factor = get_balance_factor(cur_node);

    // Restructure AVL tree if necessary
    if ((balance_factor > 1) && (high < cur_node->left->interval.base)) {
        // R Rotation
        return rotate_right(cur_node);
    } else if ((balance_factor < -1) && (base > cur_node->right->interval.high)) {
        // L Rotation
        return rotate_left(cur_node);
    } else if ((balance_factor < -1) && (high < cur_node->right->interval.base)) {
        // RL Rotation
        cur_node->right = rotate_right(cur_node->right);
        return rotate_left(cur_node);
    } else if ((balance_factor > 1) && (base > cur_node->left->interval.high)) {
        // LR Roation
        cur_node->left = rotate_left(cur_node->left);
        return rotate_right(cur_node);
    }

    return cur_node;
}

void insert(uint64_t base_address, uint64_t high_address) {
    // First node, set base node of AVL tree
    if (!base) {
        base = allocate_node(base_address, high_address);
        return;
    }

    // BST Insertion
    // Add 'insert_mmap_node' node to AVL tree, start looking at 'base' node
    base = insert_internal(base, base_address, high_address);
}

// Traverse left to right when printing nodes
void print_internal(struct mmap_range_node* base) {
    // Traverse left subtree
    if (base->left != NULL) {
        print_internal(base->left);
    }

    // Logic reaches here, everything to the left of subtree print
    // Now print contents of current node
    terminal_printf("Low: %16x High: %16x Balance Factor: %d\n", base->interval.base, base->interval.high, get_balance_factor(base));

    // Traverse right subtree
    if (base->right != NULL) {
        print_internal(base->right);
    }
}

void print_avl(void) {
    print_internal(base);
}

struct mmap_range_node* get_entry_internal(struct mmap_range_node* base, uint64_t addr) {
    if (!base) {
        // Reached null leaf node, addr does not exist in AVL
        return NULL;
    }
    
    if (addr < base->interval.base) {
        // Addres is below current interval, recurse left child
        return get_entry_internal(base->left, addr);
    } else if (addr > base->interval.high) {
        // Address is above current interval, recurse right child
        return get_entry_internal(base->right, addr);
    } else {
        // Found correct interval
        return base;
    }
}

struct mmap_range_node* get_mmap_entry(uint64_t addr) {
    return get_entry_internal(base, addr);
}