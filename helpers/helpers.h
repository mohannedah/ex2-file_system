#pragma once

#include "../shared_types.h"
#include "../disk_simulator_manager/disk.h"

inline int write_block_disk_helper(int block_number, char *data, Disk *disk_manager)
{
    char *start_ptr = data;

    int num_sectors = BLOCK_SIZE / SECTOR_SIZE;

    Sector disk_sector;

    int sector_number = block_number * num_sectors;

    for (int i = 0; i < num_sectors; i++)
    {
        memcpy(disk_sector.buffer, start_ptr, SECTOR_SIZE);

        int status = disk_manager->write_sector(sector_number, &disk_sector);

        if (status == -1)
        {
            return -1;
        }

        sector_number += 1;

        start_ptr += SECTOR_SIZE;
    };

    return 0;
};

inline int retreive_block_disk_helper(int block_number, MemoryBlock *block, Disk *disk_manager)
{
    int num_sectors = BLOCK_SIZE / SECTOR_SIZE;

    int sector_number = block_number * num_sectors;

    Sector disk_sector;

    char buffer[BLOCK_SIZE];

    for (int i = 0; i < num_sectors; i++)
    {
        int status = disk_manager->populate_sector(sector_number, &disk_sector);

        if (status == -1)
            return -1;

        memcpy(buffer + (i * SECTOR_SIZE), disk_sector.buffer, SECTOR_SIZE);

        sector_number += 1;
    };

    memcpy(block->data, buffer, BLOCK_SIZE);

    return 0;
};

inline int64_t get_current_time()
{
    return chrono::duration_cast<chrono::seconds>(chrono::system_clock::now().time_since_epoch()).count();
};