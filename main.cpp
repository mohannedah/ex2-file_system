#include <bits/stdc++.h>
#include "disk_simulator_manager/disk.h"
#include "file_system_manager/block_group/block_group.h"
#include "file_system_manager/ex2_interface/ex2.h"
#include "global_dependecies.h"
#include "./shell_manager/shell_manager.h"

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
    BlockDescriptorManager descriptor_manager; 
    
    descriptor_manager.initialize_block_group_bitmaps();
    
    descriptor_manager.initialize_block_group_descriptors();
    
    descriptor_manager.initialize_inodes();

    initialize_helper(disk_manager);

    EX2FILESYSTEM ex2_file_system(&descriptor_manager, disk_manager);

    ShellManager shell_manager(&ex2_file_system);    

    
    string file_name = "mohanned", file_permissions = "RW";

    int status = shell_manager.create_file(file_name, file_permissions, 1);

    
    status = shell_manager.change_directory(file_name);

    shell_manager.print_curr_directory_name();

    
    file_name = "ahmed";
    
    shell_manager.create_file(file_name, file_permissions, 1);

    shell_manager.change_directory(file_name);

    shell_manager.print_curr_directory_name();


    file_name = "fadwa";

    shell_manager.create_file(file_name, file_permissions, 1);

    shell_manager.change_directory(file_name);

    shell_manager.print_curr_directory_name();
    return 0;
}
