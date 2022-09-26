#include <stdbool.h>
#include <arch/terminal.h>
#include <libc/string.h>

#define max(a,b) ((a > b) ? a : b)

#define PAGE_SIZE_BYTES     4096

void init_pmm(void);

// MMap section bitmap
struct bitmap {
    uint8_t* map;
};

// PMM AVL internals
struct address_interval {
    uint64_t base;
    uint64_t high;
};


struct mmap_range_node {
    struct address_interval interval;   // MMap seciton address interval
    struct mmap_range_node* left;       // Left child of node
    struct mmap_range_node* right;      // Right child of node
    struct bitmap* section_mmap;        // Pointer to bitmap of mmap section
};

/*
Info about AVL trees
https://www.geeksforgeeks.org/avl-tree-set-1-insertion/
Special AVL Tree - Used when Kernel looks up an mmap section defined in bootloader
    - Each node is an interval, (low, high) address pair of mmap section
    - Intervals do not overlap
    - The total size of AVL tree is calculated upon initialization
*/
void init_bitmap_avl(struct limine_memmap_entry** entries, int size);

// Called from insert()
// Find and initialize next available node slot in avl_nodes array
struct mmap_range_node* allocate_node(uint64_t base_address, uint64_t high_address);

int get_node_level(struct mmap_range_node* base);

// Called from insert()
// Calculate balance factor to determine if restructure is necessary
int get_balance_factor(struct mmap_range_node* base);

// Rotate right/left used for rebalancing AVL tree
struct mmap_range_node* rotate_right(struct mmap_range_node* node);
struct mmap_range_node* rotate_left(struct mmap_range_node* node);

// Called from insert()
// Function to internally insert node, recursively traverse layers of tree
struct mmap_range_node* insert_internal(struct mmap_range_node* cur_node, uint64_t base, uint64_t high);

// Insert mmap address interval (no overlap), rebalance tree if necessary
void insert(uint64_t base_address, uint64_t high_address);

// Call from print_avl()
// Recursively print mmap section intervals in sorted order
void print_internal(struct mmap_range_node* base);

// Print AVL structure
void print_avl(void);

// Internal function to recursively find bitmap node in o(logn) time
struct mmap_range_node* get_entry_internal(struct mmap_range_node* base, uint64_t addr);

// Get MMap interval
struct mmap_range_node* get_mmap_entry(uint64_t addr);