#include <bits/stdc++.h>
#include "buffer_pool_manager/buffer_pool.h"
#include "disk_simulator_manager/disk.h"
#include "file_system_manager/block_group/block_group.h"
#include "file_system_manager/ex2_interface/ex2.h"

using namespace std;

void initialize_super_block() {

};

void initialize_helper() {

};

int main()
{
    Disk disk_manager;

    BufferPoolManager buffer_pool = BufferPoolManager(&disk_manager);

    BlockDescriptorManager descriptor_manager(&disk_manager, &buffer_pool);

    return 0;
}
