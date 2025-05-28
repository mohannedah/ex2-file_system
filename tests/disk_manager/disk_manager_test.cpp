#include <gtest/gtest.h>
#include "../../shared_types.h"
#include <disk.h>


int get_random_integer(int left, int right) {
    int rand_number = rand();
    int length = right - left + 1;
    return ((rand_number % length) + left);
}

class DiskManagerTest : public testing::Test {
    protected:
    Disk* disk_manager;
    DiskManagerTest() {
        this->disk_manager = new Disk(); 
    }

    ~DiskManagerTest() {
        delete this->disk_manager;
    };
};


TEST_F(DiskManagerTest, correct_initialization_of_disk_manager) {
    ASSERT_GE(disk_manager->file_descriptor, 0);
};

TEST_F(DiskManagerTest, test_writing_at_random_sector) {
    Sector sector, read_sector;
    for(int i = 0; i < 1e5; i++) {
        int random_sector_number = get_random_integer(0, NUM_SECTORS - 1);
        
        sector.sector_num = random_sector_number;

        for(int i = 0; i < SECTOR_SIZE; i++) {
            byte rand_byte = (byte)get_random_integer(0, 255);
            sector.buffer[i] = (char)rand_byte;
        };

        int status = this->disk_manager->write_sector(sector.sector_num, &sector);

        ASSERT_EQ(status, 0) << "Failed while writing sector with sector num " << sector.sector_num << endl;

        read_sector.sector_num = random_sector_number;

        status = this->disk_manager->populate_sector(random_sector_number, &read_sector);

        ASSERT_EQ(status, 0) << "Failed while reading sector with sector num " << random_sector_number << endl;
        
        status = strcmp(sector.buffer, read_sector.buffer);

        ASSERT_EQ(status, 0) << "Failed while comparing the write sector with the read sector " << random_sector_number << endl;
    };
};
