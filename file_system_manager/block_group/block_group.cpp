#include "block_group.h"

static int get_starting_block_inode(int inode_number)
{
    int group_number = (inode_number / BLOCKS_PER_INODE);

    int block_number = (BLOCKS_GROUP_RESERVED * (group_number + 1)) + (RESERVED_OFFSET_GROUPS) + inode_number;

    return block_number;
}

static int get_starting_block_inode_bitmap(int group_number)
{
    int block_number = (group_number * (BLOCKS_GROUP_RESERVED + NUM_INODES_PER_GROUP)) + (RESERVED_OFFSET_GROUPS) + 2;

    return block_number;
};

static int get_starting_block_bitmap(int group_number)
{
    int block_number = (group_number * (BLOCKS_GROUP_RESERVED + NUM_INODES_PER_GROUP)) + (RESERVED_OFFSET_GROUPS) + 1;

    return block_number;
};

static int get_starting_block_group_descriptor(int group_number)
{
    int starting_block = (RESERVED_OFFSET_GROUPS + (group_number * (BLOCKS_GROUP_RESERVED + NUM_INODES_PER_GROUP)));

    return starting_block;
};

int BlockGroupINodeBitMap::copy_write_memory_block(MemoryBlock *block)
{
    int *start_ptr = (int *)block;

    *start_ptr = this->size;

    start_ptr += 1;

    *start_ptr = this->block_size;

    start_ptr += 1;

    for (int i = 0; i < this->size; i++)
    {
        *start_ptr = this->blocks[i];
        start_ptr += 1;
    };

    return 0;
};

BlockGroupINodeBitMap BlockGroupINodeBitMap::write_inode_bitmap(MemoryBlock *block)
{
    int *start_ptr = (int *)block;

    BlockGroupINodeBitMap bitmap_block(start_ptr, start_ptr + 1, start_ptr + 2);

    return bitmap_block;
};

int BlockGroupBlockBitMap::copy_write_memory_block(MemoryBlock *block)
{
    int *start_ptr = (int *)block;

    *start_ptr = this->size;

    start_ptr += 1;

    *start_ptr = this->block_size;

    start_ptr += 1;

    for (int i = 0; i < this->size; i++)
    {
        *start_ptr = this->blocks[i];
        start_ptr += 1;
    };

    return 0;
};

BlockGroupBlockBitMap BlockGroupBlockBitMap::write_block_bitmap(MemoryBlock *block)
{
    int *start_ptr = (int *)block;

    BlockGroupBlockBitMap bitmap_block(start_ptr, start_ptr + 1, start_ptr + 2);

    return bitmap_block;
};

int BlockDescriptorManager::find_first_empty_data_block(int inode_number)
{
    int group_number = inode_number / NUM_INODES_PER_GROUP;

    BlockGroupDescriptor *block_descriptor;

    this->read_block_descriptor(group_number, block_descriptor);

    if (block_descriptor->free_blocks_count == 0)
        return -1;

    MemoryBlock *block = this->buffer_pool_manager->get_block(block_descriptor->block_bitmap);

    BlockGroupBlockBitMap block_bitmap = BlockGroupBlockBitMap::write_block_bitmap(block);

    int lowest_bit = block_bitmap.get_least_significant_bit(0, DATA_BLOCKS_PER_GROUP);

    block_bitmap.set_bit(lowest_bit);

    return lowest_bit + (group_number * DATA_BLOCKS_PER_GROUP);
};

int BlockDescriptorManager::find_first_empty_inode(int block_group_number, BlockGroupDescriptor *block_descriptor)
{
    BlockGroupDescriptor &block_descriptor_d = *block_descriptor;

    if (block_descriptor_d.free_inodes_count == 0)
        return -1;

    MemoryBlock *block = this->buffer_pool_manager->get_block(block_descriptor_d.inode_bitmap);

    BlockGroupINodeBitMap bit_map = BlockGroupINodeBitMap::write_inode_bitmap(block);

    int lowest_bit = bit_map.get_least_significant_bit(0, NUM_INODES_PER_GROUP - 1);

    bit_map.set_bit(lowest_bit);

    return lowest_bit + (block_group_number * NUM_INODES_PER_GROUP);
};

int BlockDescriptorManager::read_block_descriptor(int group_number, BlockGroupDescriptor *block_descriptor)
{
    MemoryBlock *block = this->buffer_pool_manager->get_block(get_starting_block_group_descriptor(group_number));

    memcpy(block_descriptor, block->data, sizeof(BlockGroupDescriptor));

    return 0;
};

int BlockDescriptorManager::write_inode_info(int inode_number, BlockGroupINode *inode)
{
    return write_block_disk_helper(get_starting_block_inode(inode_number), (char *)inode, this->disk_manager);
}

int BlockDescriptorManager::read_inode_info(int inode_number, BlockGroupINode *inode)
{
    MemoryBlock *block = this->buffer_pool_manager->get_block(get_starting_block_inode(inode_number));

    memcpy(inode, block->data, sizeof(BlockGroupINode));

    return 0;
};
