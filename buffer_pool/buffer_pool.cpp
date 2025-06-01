#include "buffer_pool.h"

BufferPool::BufferPool(int num_blocks) {
    this->num_blocks = num_blocks;

    int status = this->allocate_memory_regions();

    if(status == -1) {
        cout << "Something wrong happened while initializing the memory blocks" << endl;
        exit(0);
    }
};

int BufferPool::allocate_memory_regions() {
    this->large_memory_region = (char*)malloc(num_blocks*sizeof(MemoryBlock));
    
    int status = mlock(this->large_memory_region, num_blocks*sizeof(MemoryBlock));

    if(status == -1) {
        cout << "Something wrong happened while locking the memory region" << endl;
        return -1;
    }

    char* start_ptr = this->large_memory_region;

    for(int i = 0; i < this->num_blocks; i++) {
        BlockListNode<MemoryBlock*>* list_node = (BlockListNode<MemoryBlock*>*)malloc(sizeof(BlockListNode<MemoryBlock*>));

        list_node->data = (MemoryBlock*)start_ptr;

        start_ptr += BLOCK_SIZE;

        this->free_list.insert_tail(list_node);
    };

    return 0;
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
    if(block_number >= (NUM_SECTORS / (BLOCK_SIZE / SECTOR_SIZE)) || block_number < 0) {
        cout << "Invalid block number passed " << block_number << endl;
        return nullptr;
    };

    if(this->mp_block_list_node.count(block_number)) {
        this->lock(block_number);
        return mp_block_list_node[block_number]->data;
    };
    
    ListNode<MemoryBlock*>* victim_node = this->get_victim();
    
    // In this case, the buffer_pool is full, so we will have to give him memory that is being operated by the OS, rather than being operated by our eviction_policy :).
    if(victim_node == nullptr) {
        MemoryBlock* block = (MemoryBlock*)malloc(sizeof(MemoryBlock));
        
        victim_node = (BlockListNode<MemoryBlock*>*)malloc(sizeof(BlockListNode<MemoryBlock*>));

        victim_node->data = block;

        victim_node->data->os_operated = true; 
    };

    MemoryBlock *block = victim_node->data;

    this->force_flush_memory_block(block); 

    int status = read_memory_block_helper(block_number, block);

    if(status == -1) {
        cout << "Something wrong happened while reading the memory block from disk" << endl;
        return nullptr;
    }

    this->mp_block_list_node[block_number] = (BlockListNode<MemoryBlock*>*)victim_node;

    status = this->lock(block_number);

    if(status == -1) {
        cout << "Something wrong happened while locking the memory block" << endl;
        return nullptr;
    }

    return block;
};


int BufferPool::force_flush_memory_block(MemoryBlock *block) {
    int block_number = block->block_number;

    if(block_number == -1 || (!block->is_dirty)) return 0;

    int status = write_block_disk_helper(block_number, block->data);

    block->is_dirty = false;

    return status;
}

ListNode<MemoryBlock*>* BufferPool::get_victim() {
    return this->free_list.get_tail();
};


int BufferPool::free(int block_number) {
    ListNode<MemoryBlock*>* list_node = this->mp_block_list_node[block_number];

    if(list_node->data->os_operated) {
        int status = force_flush_memory_block(list_node->data);

        if(status == -1) {
            cout << "Something wrong happened while flushing the block to disk" << endl;
            return status;
        };
        
        delete list_node;

        return status;
    } 

    this->free_list.insert_head(list_node);
    
    return 0;
}
