#include <sys/mman.h>
#include "../shared_types.h"
#include "../disk_simulator_manager/disk.h"
#include "../helpers/helpers.h"
#include "../data_structures/linked_list/linked_list.h"

class BufferPool {
    private:
    int read_memory_block_helper(int block_number, MemoryBlock* block);
    int allocate_memory_regions();
    int num_blocks;
    char* large_memory_region;
    Disk *disk_manager;
    
    public:
    LinkedList<MemoryBlock*> free_list;

    map<int, BlockListNode<MemoryBlock*>*> mp_block_list_node;

    BufferPool(int num_blocks, Disk* disk_manager);
    
    ~BufferPool();
    
    MemoryBlock* get(int block_number);

    int lock(int block_number);

    int free(int block_number);

    int force_flush_memory_block(MemoryBlock *block);

    ListNode<MemoryBlock*>* get_victim();
};
