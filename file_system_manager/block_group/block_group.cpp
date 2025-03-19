#include "block_group.h"

static int get_starting_block_inode(int inode_number)
{
    int group_number = (inode_number / BLOCKS_PER_INODE);

    inode_number %= NUM_INODES_PER_GROUP;

    int block_number = (group_number * (BLOCKS_GROUP_RESERVED + DATA_BLOCKS_PER_GROUP + NUM_INODES_PER_GROUP)) + (BLOCKS_GROUP_RESERVED) + (RESERVED_OFFSET_GROUPS) + inode_number;

    return block_number;
}

static int get_starting_block_inode_bitmap(int group_number)
{
    int block_number = (group_number * (BLOCKS_GROUP_RESERVED + NUM_INODES_PER_GROUP + DATA_BLOCKS_PER_GROUP)) + (RESERVED_OFFSET_GROUPS) + 2;

    return block_number;
};

static int get_starting_block_bitmap(int group_number)
{
    int block_number = (group_number * (BLOCKS_GROUP_RESERVED + NUM_INODES_PER_GROUP + DATA_BLOCKS_PER_GROUP)) + (RESERVED_OFFSET_GROUPS) + 1;

    return block_number;
};

static int get_starting_block_group_descriptor(int group_number)
{
    int starting_block = (RESERVED_OFFSET_GROUPS + (group_number * (BLOCKS_GROUP_RESERVED + NUM_INODES_PER_GROUP + DATA_BLOCKS_PER_GROUP)));

    return starting_block;
};

BlockGroupINode::~BlockGroupINode()
{
    MemoryBlock block;

    BlockGroupINode::write_memory_block(&block, this);

    write_block_disk_helper(get_starting_block_inode(this->uid), block.data, disk_manager);
};

BlockGroupINodeBitMap::~BlockGroupINodeBitMap()
{
    MemoryBlock block;

    this->copy_write_memory_block(&block);

    write_block_disk_helper(this->block_number, block.data, disk_manager);
}

BlockGroupBlockBitMap::~BlockGroupBlockBitMap()
{
    MemoryBlock block;

    this->copy_write_memory_block(&block);

    write_block_disk_helper(this->block_number, block.data, disk_manager);
}

BlockGroupDescriptor::~BlockGroupDescriptor()
{
    MemoryBlock block;

    memcpy(block.data, this, BLOCK_SIZE);

    write_block_disk_helper(this->block_number, block.data, disk_manager);
};

int BlockGroupINodeBitMap::copy_write_memory_block(MemoryBlock *block)
{
    int *start_ptr = (int *)block->data;

    *start_ptr = this->size;

    start_ptr += 1;

    *start_ptr = this->block_size;

    start_ptr += 1;

    int cnt_bytes = 8;
    for (int i = 0; i < this->size; i++)
    {
        *start_ptr = this->blocks[i];
        start_ptr += 1;
        cnt_bytes += 4;
    };

    *start_ptr = this->block_number;

    return 0;
};

BlockGroupINodeBitMap BlockGroupINodeBitMap::write_inode_bitmap(MemoryBlock *block)
{
    int *start_ptr = (int *)block->data;

    int size = *start_ptr;

    start_ptr += 2;
    for (int i = 0; i < size; i++)
    {
        start_ptr += 1;
    }

    BlockGroupINodeBitMap bitmap_block(block);

    return bitmap_block;
};

int BlockGroupBlockBitMap::copy_write_memory_block(MemoryBlock *block)
{
    int *start_ptr = (int *)block->data;

    *start_ptr = this->size;

    start_ptr += 1;

    *start_ptr = this->block_size;

    start_ptr += 1;

    for (int i = 0; i < this->size; i++)
    {
        *start_ptr = this->blocks[i];
        start_ptr += 1;
    };

    *start_ptr = this->block_number;

    return 0;
};

BlockGroupBlockBitMap BlockGroupBlockBitMap::write_block_bitmap(MemoryBlock *block)
{
    int *start_ptr = (int *)block->data;

    BlockGroupBlockBitMap bitmap_block(block);

    return bitmap_block;
};

BlockDescriptorManager::BlockDescriptorManager(Disk *disk_manager)
{
    this->disk_manager = disk_manager;
};

int BlockDescriptorManager::find_first_empty_data_block(int inode_number)
{
    int group_number = inode_number / NUM_INODES_PER_GROUP;

    BlockGroupDescriptor block_descriptor;

    this->read_block_descriptor(group_number, &block_descriptor);

    if (block_descriptor.free_blocks_count == 0)
        return -1;

    MemoryBlock block;

    retreive_block_disk_helper(get_starting_block_bitmap(group_number), &block, this->disk_manager);

    BlockGroupBlockBitMap block_bitmap = BlockGroupBlockBitMap::write_block_bitmap(&block);

    int lowest_bit = block_bitmap.get_least_significant_bit(0, DATA_BLOCKS_PER_GROUP - 1);

    block_bitmap.set_bit(lowest_bit);

    return get_starting_block_group_descriptor(group_number) + RESERVED_OFFSET_GROUPS + NUM_INODES_PER_GROUP + lowest_bit;
};

int BlockDescriptorManager::find_first_empty_inode(int block_group_number, BlockGroupDescriptor *block_descriptor)
{
    BlockGroupDescriptor &block_descriptor_d = *block_descriptor;

    if (block_descriptor_d.free_inodes_count == 0)
        return -1;

    MemoryBlock block;

    retreive_block_disk_helper(get_starting_block_inode_bitmap(block_group_number), &block, this->disk_manager);

    BlockGroupINodeBitMap bit_map = BlockGroupINodeBitMap::write_inode_bitmap(&block);

    int lowest_bit = bit_map.get_least_significant_bit(0, NUM_INODES_PER_GROUP - 1);

    bit_map.set_bit(lowest_bit);

    return get_starting_block_group_descriptor(block_group_number) + RESERVED_OFFSET_GROUPS + lowest_bit;
};

int BlockDescriptorManager::read_block_descriptor(int group_number, BlockGroupDescriptor *block_descriptor)
{
    MemoryBlock block;

    retreive_block_disk_helper(get_starting_block_group_descriptor(group_number), &block, this->disk_manager);

    memcpy(block_descriptor, block.data, sizeof(BlockGroupDescriptor));

    return 0;
};

int BlockDescriptorManager::write_inode_info(int inode_number, BlockGroupINode *inode)
{
    MemoryBlock block;

    BlockGroupINode::write_memory_block(&block, inode);

    return write_block_disk_helper(get_starting_block_inode(inode_number), (char *)block.data, this->disk_manager);
}

int BlockDescriptorManager::read_inode_info(int inode_number, BlockGroupINode *inode)
{
    MemoryBlock block;

    retreive_block_disk_helper(get_starting_block_inode(inode_number), &block, this->disk_manager);

    BlockGroupINode::copy_memory_block(&block, inode);

    return 0;
};

static void initialize_inode_helper(int id, BlockGroupINode *inode)
{
    inode->hard_link_count = 0;

    inode->uid = id;

    for (int i = 0; i < BLOCKS_PER_INODE; i++)
    {
        inode->blocks[i] = 0;
    };

    inode->last_deletion_time = 0;

    inode->creation_time = get_current_time();

    inode->last_modification_time = 0;

    inode->last_access_time = 0;

    inode->last_deletion_time = 0;

    inode->inode_mode = READ_BIT | WRITE_BIT;

    inode->file_size = 0;
};

void BlockDescriptorManager::initialize_inodes()
{
    MemoryBlock block;

    for (int inode_number = 0; inode_number < NUM_INODES_PER_GROUP * NUM_GROUPS; inode_number++)
    {
        int block_number = get_starting_block_inode(inode_number);

        BlockGroupINode inode;

        initialize_inode_helper(inode_number, &inode);

        BlockGroupINode::write_memory_block(&block, &inode);

        write_block_disk_helper(block_number, block.data, this->disk_manager);
    }
};

void BlockDescriptorManager::initialize_block_group_bitmaps()
{
    MemoryBlock block;

    BlockGroupINodeBitMap bitmap;

    BlockGroupBlockBitMap data_block_bitmap;

    for (int group_number = 0; group_number < NUM_GROUPS; group_number += 1)
    {
        int block_number = get_starting_block_inode_bitmap(group_number);

        bitmap.block_number = block_number;

        int status = bitmap.copy_write_memory_block(&block);

        if (status == -1)
        {
            perror("Error initializing INode bitmap.");
            exit(-1);
        }

        status = write_block_disk_helper(block_number, block.data, this->disk_manager);

        block_number = get_starting_block_bitmap(group_number);

        data_block_bitmap.block_number = block_number;

        data_block_bitmap.copy_write_memory_block(&block);

        status = write_block_disk_helper(block_number, block.data, this->disk_manager);
    };
};

static void initialize_block_group_descriptor_helper(BlockGroupDescriptor *block_descriptor, int block_number)
{
    block_descriptor->free_blocks_count = DATA_BLOCKS_PER_GROUP;

    block_descriptor->free_inodes_count = NUM_INODES_PER_GROUP;

    block_descriptor->used_dirs_count = 0;

    block_descriptor->block_number = block_number;
};

void BlockDescriptorManager::initialize_block_group_descriptors()
{
    for (int block_group = 0; block_group < NUM_GROUPS; block_group++)
    {
        int block_number = get_starting_block_group_descriptor(block_group);

        BlockGroupDescriptor block_descriptor;

        initialize_block_group_descriptor_helper(&block_descriptor, block_number);

        write_block_disk_helper(block_number, (char *)&block_descriptor, this->disk_manager);
    };
};