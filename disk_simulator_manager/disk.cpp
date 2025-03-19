#include "disk.h"

Disk::Disk()
{
    this->file_descriptor = open("./file_image", O_RDWR);
    // cout << this->file_descriptor << endl;
}

Disk::~Disk()
{
    close(this->file_descriptor);
}

int Disk::populate_sector(int sector_number, Sector *sector_info)
{
    int starting_offset = (sector_number)*SECTOR_SIZE;

    if (starting_offset >= DISK_SIZE)
        return -1;

    int status = lseek(this->file_descriptor, starting_offset, SEEK_SET);

    if (status == -1)
        return -1;

    int bytes_read = read(this->file_descriptor, sector_info->buffer, SECTOR_SIZE);

    if (bytes_read != SECTOR_SIZE)
        return -1;

    return 0;
};

int Disk::write_sector(int sector_number, Sector *sector)
{
    int starting_offset = sector_number * SECTOR_SIZE;

    int status = lseek(this->file_descriptor, starting_offset, SEEK_SET);

    if (status == -1)
        return -1;

    int bytes_written = write(this->file_descriptor, sector->buffer, SECTOR_SIZE);

    if (bytes_written != SECTOR_SIZE)
        return -1;

    return 0;
}
