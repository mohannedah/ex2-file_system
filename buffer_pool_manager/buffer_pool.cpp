#include "buffer_pool.h"

static inline int flush_helper(MemoryBlock *memory_block, Disk *disk_manager)
{
    int block_number = memory_block->block_number;

    write_block_disk_helper(block_number, memory_block->data, disk_manager);
};

BufferPoolManager::BufferPoolManager(Disk *disk_manager)
{
    this->disk_manager = disk_manager;

    this->capacity = 100;

    int status = this->init_lru_list();

    if (status == -1)
    {
        perror("Error happened while allocating memory to the Buffer Pool");
        exit(0);
    }
};

BufferPoolManager::~BufferPoolManager()
{
    int status = this->flush_blocks();

    if (status == -1)
    {
        perror("Error flushing memory blocks.");
        exit(0);
    }

    status = this->free_regions();

    if (status == -1)
    {
        perror("Error buffer_pool memory regions.");
        exit(0);
    }
};

int BufferPoolManager::free_regions()
{
    MemoryBlock *block_ptr;

    for (ListNode<MemoryBlock *> node : this->linked_list)
    {
        block_ptr = node.data;

        unlock_and_free(block_ptr, sizeof(MemoryBlock));
    };

    return 0;
};

int BufferPoolManager::flush_blocks()
{
    for (ListNode<MemoryBlock *> node : this->linked_list)
    {
        int block_number = node.data->block_number;

        if (block_number == -1)
            continue;

        int status = this->write_disk(block_number);

        if (status == -1)
            return -1;
    }
    return 0;
};

int BufferPoolManager::init_lru_list()
{

    ListNode<MemoryBlock *> *list_node;

    MemoryBlock *block;

    for (int i = 0; i < this->capacity; i++)
    {
        block = new MemoryBlock();

        list_node = new ListNode<MemoryBlock *>(block);

        this->linked_list.insert_tail(list_node);

        if (mlock(block, sizeof(MemoryBlock)) == -1)
            return -1;

        if (mlock(list_node, sizeof(ListNode<MemoryBlock *>) == -1))
            return -1;
    };

    return 0;
};

int BufferPoolManager::read_disk(int block_number, MemoryBlock *block)
{
    return retreive_block_disk_helper(block_number, block, this->disk_manager);
};

int BufferPoolManager::write_disk(int block_number)
{
    if (!this->is_cached_block(block_number))
        return -1;

    char *buffered_data = this->map[block_number]->data->data;

    return write_block_disk_helper(block_number, buffered_data, this->disk_manager);
};

ListNode<MemoryBlock *> *BufferPoolManager::try_insert(MemoryBlock *memory_block)
{

    ListNode<MemoryBlock *> *list_node = this->get_victim_space();

    if (list_node != nullptr)
    {
        ListNode<MemoryBlock *> *list_node = this->get_victim_space();

        list_node->data = memory_block;

        this->update_reference(memory_block->block_number, list_node);

        this->curr_size += 1;

        return list_node;
    }

    return nullptr;
}

MemoryBlock *BufferPoolManager::get_block(int block_number)
{
    MemoryBlock *memory_block;

    if (!is_cached_block(block_number))
        memory_block = read_memory(block_number);
    else
        read_disk(block_number, memory_block);

    ListNode<MemoryBlock *> *list_node = try_insert(memory_block);

    if (list_node != nullptr)
    {
        this->linked_list.unlink(list_node);
    };
    return memory_block;
};

void BufferPoolManager::update_reference(int block_number, ListNode<MemoryBlock *> *list_node)
{
    this->map[block_number] = list_node;

    this->map_address_to_list_nodes[list_node->data->data] = list_node;

    list_node->data->is_pinned = true;
};

bool BufferPoolManager::is_cached_block(int block_number)
{
    return this->map.count(block_number);
};

MemoryBlock *BufferPoolManager::read_memory(int block_number)
{
    ListNode<MemoryBlock *> *node = this->map[block_number];

    return node->data;
};

ListNode<MemoryBlock *> *BufferPoolManager::get_victim_space()
{
    auto node = this->linked_list.get_tail();

    return node;
};

int BufferPoolManager::retain_memory(MemoryBlock *block)
{
    char *offset_memory_address = (char *)block->data;

    if (!map_address_to_list_nodes.count(offset_memory_address))
    {
        ListNode<MemoryBlock *> *list_node = try_insert(block);

        if (list_node != nullptr)
            return 0;

        return write_block_disk_helper(block->block_number, offset_memory_address, this->disk_manager);
    }

    ListNode<MemoryBlock *> *list_node = map_address_to_list_nodes[offset_memory_address];

    list_node->data->is_pinned = false;

    this->linked_list.insert_head(list_node);

    return 0;
};

//----------------------------------------------------Utility Functions----------------------------------------------------------

int unlock_and_free(void *block_ptr, size_t region_size)
{
    munlock(block_ptr, region_size);

    free(block_ptr);

    return 0;
};