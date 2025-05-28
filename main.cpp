#include <bits/stdc++.h>
#include "disk_simulator_manager/disk.h"
#include "file_system_manager/block_group/block_group.h"
#include "file_system_manager/ex2_interface/ex2.h"
#include "global_dependecies.h"

using namespace std;

SuperBlock super_block;

void initialize_super_block()
{
    super_block.blocks_count = NUM_GROUPS * (NUM_INODES_PER_GROUP + DATA_BLOCKS_PER_GROUP);
    super_block.inodes_count = NUM_GROUPS * NUM_INODES_PER_GROUP;
    super_block.free_blocks_count = NUM_GROUPS * DATA_BLOCKS_PER_GROUP;
    super_block.inodes_per_group = NUM_INODES_PER_GROUP;
    super_block.last_mount_time = chrono::duration_cast<chrono::seconds>(chrono::system_clock::now().time_since_epoch()).count();
};

void initialize_helper(Disk *disk_manager)
{
    initialize_super_block();
    write_block_disk_helper(1, (char *)&super_block, disk_manager);
};

Disk *disk_manager = new Disk();

int main()
{
    BlockDescriptorManager descriptor_manager(disk_manager);

    descriptor_manager.initialize_block_group_bitmaps();


    descriptor_manager.initialize_block_group_descriptors();


    descriptor_manager.initialize_inodes();


    initialize_helper(disk_manager);

    EX2FILESYSTEM file_system(&descriptor_manager, disk_manager);

    int vacant_inode = file_system.create_file("Mohanned Ahmed", sizeof("Mohanned Ahmed"), READ_BIT | WRITE_BIT, 0);

    int fd = file_system.my_open(vacant_inode, READ_BIT | WRITE_BIT);

    file_system.my_file_system_write(fd, "Mohanned Ahmed", sizeof("Mohanned Ahmed") - 1);

    file_system.my_file_system_write(fd, " Mohanned Ahmed", sizeof(" Mohanned Ahmed") - 1);
    char *some_string = "Mohanned Ahmed";

    char buffer[strlen(some_string) + strlen(some_string) + 2];

    file_system.my_file_system_read(fd, buffer, strlen("Mohanned Ahmed") * 2 + 1);

    buffer[strlen(some_string)*2 + 1] = '\0';
    cout << buffer << endl;
    return 0;
}
