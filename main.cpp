#include <bits/stdc++.h>
#include "buffer_pool_manager/buffer_pool.h"
#include "disk_simulator_manager/disk.h"
#include "file_system_manager/block_group/block_group.h"
#include "file_system_manager/ex2_interface/ex2.h"

using namespace std;

SuperBlock super_block;
// EX2FILESYSTEM file_system;

void initialize_super_block()
{
    super_block.blocks_count = NUM_GROUPS * (NUM_INODES_PER_GROUP + DATA_BLOCKS_PER_GROUP);
    super_block.inodes_count = NUM_GROUPS * NUM_INODES_PER_GROUP;
    super_block.free_blocks_count = NUM_GROUPS * DATA_BLOCKS_PER_GROUP;
    super_block.inodes_per_group = NUM_INODES_PER_GROUP;
    super_block.last_mount_time = chrono::duration_cast<chrono::seconds>(chrono::system_clock::now().time_since_epoch()).count();
};

void initialize_helper()
{
    initialize_super_block();
};

int main()
{
    Disk disk_manager;
    BufferPoolManager buffer_pool = BufferPoolManager(&disk_manager);
    BlockDescriptorManager descriptor_manager(&disk_manager, &buffer_pool);

    // descriptor_manager.initialize_block_group_descriptors();

    // descriptor_manager.initialize_inodes();

    // descriptor_manager.initialize_block_group_bitmaps();

    initialize_helper();
    EX2FILESYSTEM file_system(&buffer_pool, &descriptor_manager, &disk_manager);
    int fd = file_system.my_open(100, READ_BIT | WRITE_BIT);
    return 0;
}
