#ifndef BLOCK_GROUP_H
#define BLOCK_GROUP_H
#include "../../data_structures/bitmap/bitmap.h"
#include "../../helpers/helpers.h"
#include "../../global_dependecies.h"
#include <bits/stdc++.h>

class BlockGroupINodeBitMap : public BitMap<NUM_INODES_PER_GROUP>
{
public:
    int block_number;
    BlockGroupINodeBitMap() : BitMap() {};

    ~BlockGroupINodeBitMap();

    BlockGroupINodeBitMap(MemoryBlock *block) : BitMap((int *)(block->data), (int *)block->data + 1, (int *)(block->data) + 2)
    {
        int *start_ptr = (int *)block->data;

        int amount_to_skip = 2 + this->size;

        start_ptr += (2 + this->size);

        this->block_number = *start_ptr;
    };

    int copy_write_memory_block(MemoryBlock *block);

    static BlockGroupINodeBitMap write_inode_bitmap(MemoryBlock *block);
};

class BlockGroupBlockBitMap : public BitMap<DATA_BLOCKS_PER_GROUP>
{
public:
    int block_number;
    BlockGroupBlockBitMap() : BitMap() {};

    ~BlockGroupBlockBitMap();

    BlockGroupBlockBitMap(MemoryBlock *block) : BitMap((int *)(block->data) + 1, (int *)block->data, (int *)(block->data) + 2)
    {
        int *start_ptr = (int *)block->data;

        int amount_to_skip = 2 + this->size;

        start_ptr += (2 + this->size);

        this->block_number = *start_ptr;
    };

    int copy_write_memory_block(MemoryBlock *block);

    static BlockGroupBlockBitMap write_block_bitmap(MemoryBlock *block);
};

struct BlockGroupDescriptor
{
public:
    uint32_t block_bitmap;
    uint32_t inode_bitmap;
    uint32_t inode_table;
    uint16_t free_blocks_count;
    uint16_t free_inodes_count;
    uint16_t used_dirs_count;
    uint16_t padding;
    int block_number;
    ~BlockGroupDescriptor();
};

struct BlockGroupINodeTable
{
    uint32_t i_nodes_offsets[NUM_INODES_PER_GROUP];
};

struct BlockGroupINode
{
    uint16_t inode_mode;
    uint16_t uid;
    uint16_t access_groups;
    uint32_t file_size; // File size is limited by the number of data blocks, it will be (no.blocks.inode * block_size)
    uint32_t hard_link_count;
    uint32_t blocks[BLOCKS_PER_INODE];
    uint64_t creation_time;
    uint64_t last_access_time;
    uint64_t last_modification_time;
    uint64_t last_deletion_time;
    bool is_directory;
    char file_name[50];

    ~BlockGroupINode();

    static void copy_helper(char *dest, char *src)
    {
        memcpy(dest, src, sizeof(uint16_t) * 3);

        dest += sizeof(uint16_t) * 3, src += sizeof(uint16_t) * 3;

        memcpy(dest, src, sizeof(uint32_t) * 11);

        dest += sizeof(uint32_t) * 11, src += sizeof(uint32_t) * 11;

        memcpy(dest, src, sizeof(uint64_t) * 4);

        dest += sizeof(uint64_t) * 4, src += sizeof(uint64_t) * 4;

        memcpy(dest, src, sizeof(is_directory));

        dest += sizeof(is_directory), src += sizeof(is_directory);

        memcpy(dest, src, sizeof(file_name));
    };

    static void copy_memory_block(MemoryBlock *block, BlockGroupINode *inode)
    {
        char *start_ptr_one = (char *)inode, *start_ptr_two = (char *)block->data;

        return copy_helper(start_ptr_one, start_ptr_two);
    };

    static void write_memory_block(MemoryBlock *block, BlockGroupINode *inode)
    {
        char *start_ptr_one = (char *)inode, *start_ptr_two = (char *)block->data;

        return copy_helper(start_ptr_two, start_ptr_one);
    };
};

class BlockDescriptorManager
{
private:
public:
    BlockDescriptorManager();

    int read_inode_info(int inode_number, BlockGroupINode *inode);

    int write_inode_info(int inode_number, BlockGroupINode *inode);

    int find_first_empty_inode(int block_group_number, BlockGroupDescriptor *block_descriptor);

    int find_first_empty_data_block(int inode_number);

    int read_block_descriptor(int block_number, BlockGroupDescriptor *block_descriptor);

    void initialize_inodes();

    void initialize_block_group_descriptors();

    void initialize_block_group_bitmaps();
};

#endif
