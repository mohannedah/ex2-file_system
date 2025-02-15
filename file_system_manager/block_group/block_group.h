#ifndef BLOCK_GROUP_H
#define BLOCK_GROUP_H
#include "../../buffer_pool_manager/buffer_pool.h"
#include "../../data_structures/bitmap/bitmap.h"
#include "../../helpers/helpers.h"
#include <bits/stdc++.h>

struct BlockGroupDescriptor
{
    uint32_t block_bitmap;
    uint32_t inode_bitmap;
    uint32_t inode_table;
    uint16_t free_blocks_count;
    uint16_t free_inodes_count;
    uint16_t user_dirs_count;
    uint16_t padding;
    byte reserved[12];
};

struct BlockGroupINodeTable
{
    uint32_t i_nodes_offsets[NUM_INODES_PER_GROUP];
};

struct BlockGroupINode
{
    uint16_t inode_mode;
    uint16_t uid;
    uint32_t file_size; // File size is limited by the number of data blocks, it will be (no.blocks.inode * block_size)
    uint64_t last_access_time;
    uint64_t creation_time;
    uint64_t last_modification_time;
    uint64_t last_deletion_time;
    uint16_t access_groups;
    uint32_t hard_link_count;
    uint32_t blocks[BLOCKS_PER_INODE];
    char file_name[50];
};

class BlockGroupINodeBitMap : public BitMap<NUM_INODES_PER_GROUP>
{
public:
    BlockGroupINodeBitMap(int *size, int *block_size, int *blocks) : BitMap(size, block_size, blocks) {};

    int copy_write_memory_block(MemoryBlock *block);

    static BlockGroupINodeBitMap write_inode_bitmap(MemoryBlock *block);
};

class BlockGroupBlockBitMap : public BitMap<DATA_BLOCKS_PER_GROUP>
{
public:
    BlockGroupBlockBitMap(int *size, int *block_size, int *blocks) : BitMap(size, block_size, blocks) {};

    int copy_write_memory_block(MemoryBlock *block);

    static BlockGroupBlockBitMap write_block_bitmap(MemoryBlock *block);
};

class BlockDescriptorManager
{
private:
    BufferPoolManager *buffer_pool_manager;
    Disk *disk_manager;

public:
    BlockDescriptorManager(Disk *disk_manager, BufferPoolManager *buffer_pool);
    int read_inode_info(int inode_number, BlockGroupINode *inode);
    int write_inode_info(int inode_number, BlockGroupINode *inode);
    int find_first_empty_inode(int block_group_number, BlockGroupDescriptor *block_descriptor);
    int find_first_empty_data_block(int inode_number);
    int read_block_descriptor(int block_number, BlockGroupDescriptor *block_descriptor);
};

#endif
