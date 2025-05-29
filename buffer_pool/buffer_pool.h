#include "../shared_types.h"
#include "../helpers/helpers.h"
#include "../data_structures/linked_list/linked_list.h"

class BufferPool {
    private:
    int read_memory_block_helper(int block_number, MemoryBlock* block);
    public:
    LinkedList<MemoryBlock*> free_list;

    map<int, BlockListNode<MemoryBlock*>*> mp_block_list_node;

    BufferPool();
    
    ~BufferPool();
    
    MemoryBlock* get(int block_number);

    int lock(int block_number);

    int free(int block_number);

    ListNode<MemoryBlock*>* get_victim();
};
