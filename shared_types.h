#ifndef SHARED_TYPES_H
#define SHARED_TYPES_H

#include <bits/stdc++.h>
using namespace std;

const uint16_t SECTOR_SIZE = 512;
const uint64_t DISK_SIZE = (1 << 30) + (3 * 1024);
const uint64_t NUM_SECTORS = DISK_SIZE / SECTOR_SIZE;
const int RESERVED_OFFSET_SIZES = (3 * 1024); // Reserved for the Super Block existing at the beginning and the Group Descriptor Table.
const uint64_t BLOCK_SIZE = (2 * SECTOR_SIZE), NUM_GROUPS = 4,
               RESERVED_OFFSET_GROUPS = 3,
               TOTAL_SIZE_PER_GROUP = (DISK_SIZE - RESERVED_OFFSET_SIZES) / NUM_GROUPS,
               BLOCKS_GROUP_RESERVED = 4,
               RESERVED_GROUP_SIZES = (BLOCKS_GROUP_RESERVED * 1024), // Reserved for the GroupDescriptor, INodeBitmap, INodeTable and BlockBitmap.
    BLOCKS_PER_GROUP = (TOTAL_SIZE_PER_GROUP - RESERVED_GROUP_SIZES) / 1024,
               BLOCKS_PER_INODE = 9, // I've choosen this number to make the BLOCKS_PER_GROUP divisible by this (number + 1) i.e 10.
    NUM_INODES_PER_GROUP = 7900,
               DATA_BLOCKS_PER_GROUP = 7900;

struct Sector
{
    char buffer[512];
    int sector_num;
};

struct MemoryBlock
{
    int block_number = -1;
    bool is_pinned = false;
    char data[BLOCK_SIZE];
};

enum FILE_PERMISSIONS
{
    READ_BIT = (1 << 1),
    WRITE_BIT = (1 << 2)
};

class CacheableInstance {
    protected:
    virtual void free() = 0;
};

#endif
