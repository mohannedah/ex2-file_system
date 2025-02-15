#ifndef BUFFER_POOL_H
#define BUFFER_POOL_H
#include "../disk_simulator_manager/disk.h"
#include "../data_structures/linked_list/linked_list.h"
#include "../helpers/helpers.h"
#include <sys/mman.h>

class BufferPoolManager
{
private:
    Disk *disk_manager;

    unordered_map<int, ListNode<MemoryBlock *> *> map; // mapping page_numbers to their linked_list entries

    LinkedList<MemoryBlock *> linked_list;

    int read_disk(int block_number, MemoryBlock *block);

    ListNode<MemoryBlock *> *read_memory(int block_number);

    ListNode<MemoryBlock *> *get_victim_space();

    void update_reference(int block_number, ListNode<MemoryBlock *> *list_node);

    int free_regions();

    int flush_blocks();

    int write_disk(int block_number);

    int init_lru_list();

    int curr_size, capacity;

public:
    BufferPoolManager(Disk *disk_manager);

    ~BufferPoolManager();

    MemoryBlock *get_block(int block_number);

    bool is_cached_block(int block_number);
};

// ---------------------------------------------------Utility Functions---------------------------------------------------

int unlock_and_free(void *block_ptr, size_t region_size);

#endif