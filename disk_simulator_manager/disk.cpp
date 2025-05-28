#include "disk.h"


namespace fs = std::filesystem;

fs::path get_executable_path() {
    char buffer[PATH_MAX];

#if defined(_WIN32)
    GetModuleFileNameA(NULL, buffer, PATH_MAX);
#elif defined(__APPLE__)
    uint32_t size = sizeof(buffer);
    _NSGetExecutablePath(buffer, &size);
#else
    ssize_t count = readlink("/proc/self/exe", buffer, PATH_MAX);
    if (count == -1) throw std::runtime_error("Failed to read /proc/self/exe");
    buffer[count] = '\0';
#endif

    return fs::path(buffer).parent_path();
}

Disk::Disk()
{
    cout << get_executable_path() << endl;
    this->file_descriptor = open("../file_image", O_RDWR);
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
