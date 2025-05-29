#include "buffer_pool.h"

BufferPool::BufferPool() {

};


// The lock function here assumes that the block is currently cached in the buffer_pool.

int BufferPool::lock(int block_number) {
    BlockListNode<MemoryBlock*>* list_node = this->mp_block_list_node[block_number];

    return this->free_list.unlink(this->mp_block_list_node[block_number]);
};

int BufferPool::read_memory_block_helper(int block_number, MemoryBlock* block) {
    return retreive_block_disk_helper(block_number, block);
};

MemoryBlock* BufferPool::get(int block_number) {
    if(this->mp_block_list_node.count(block_number)) {
        this->lock(block_number);
        return mp_block_list_node[block_number]->data;
    };
    
    ListNode<MemoryBlock*>* victim_node = this->get_victim();

    // In this case, the buffer_pool is full, so we will have to give him memory that is being operated by the OS, rather than being operated by our eviction_policy :).

    if(victim_node == nullptr) {
        return nullptr;
    };
};


ListNode<MemoryBlock*>* BufferPool::get_victim() {
    return this->free_list.get_tail();
};


int BufferPool::free(int block_number) {
    
}
