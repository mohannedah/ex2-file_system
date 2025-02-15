#include "buffer_pool.h"

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

MemoryBlock *BufferPoolManager::get_block(int block_number)
{
    ListNode<MemoryBlock *> *list_node;
    if (!is_cached_block(block_number))
    {
        list_node = this->get_victim_space();

        if (list_node->data->block_number != -1)
            this->map.erase(list_node->data->block_number);

        list_node->data->block_number = block_number;

        int status = read_disk(block_number, list_node->data);

        if (status == -1)
            return nullptr;
    }
    else
    {
        list_node = read_memory(block_number);
    };

    this->update_reference(block_number, list_node);
    return list_node->data;
};

void BufferPoolManager::update_reference(int block_number, ListNode<MemoryBlock *> *list_node)
{
    int status = this->linked_list.insert_head(list_node);
    this->map[block_number] = list_node;
};

bool BufferPoolManager::is_cached_block(int block_number)
{
    return this->map.count(block_number);
};

ListNode<MemoryBlock *> *BufferPoolManager::read_memory(int block_number)
{
    ListNode<MemoryBlock *> *node = this->map[block_number];

    return node;
};

ListNode<MemoryBlock *> *BufferPoolManager::get_victim_space()
{
    auto node = this->linked_list.get_tail();

    // if (node == nullptr)
    // {
    //     return
    // };

    // this->linked_list.unlink(node);
    return node;
};

//----------------------------------------------------Utility Functions----------------------------------------------------------

int unlock_and_free(void *block_ptr, size_t region_size)
{
    munlock(block_ptr, region_size);
    free(block_ptr);
    return 0;
};