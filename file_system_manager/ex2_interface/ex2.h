#ifndef EX2_H
#define EX2_H
#include "../../shared_types.h"
#include "../../helpers/helpers.h"
#include "../block_group/block_group.h"
#include "../../disk_simulator_manager/disk.h"
class BlockDescriptorManager;

struct SuperBlock
{
    uint32_t inodes_count;
    uint32_t blocks_count;
    uint32_t reserved_blocks_count;
    uint32_t free_blocks_count;
    uint32_t free_inodes_count;
    uint32_t first_data_block;
    uint32_t log_block_size;
    uint32_t blocks_per_group;
    uint32_t inodes_per_group;
    uint32_t last_mount_time;
    uint32_t last_write_time;
    uint16_t mount_count;
    uint16_t max_mount_count;
    uint16_t magic_number;
};


struct BlockGroupDescriptorTable
{
    uint32_t block_group_descriptors_blocks[NUM_GROUPS];
};

struct MyFile
{
    int inode_number;
    int position;
    int file_size; // I'm caching the file_size here to help not load the inode_structure from the disk again and again to do some checks.
    int permissions;
    bool is_directory;
};


class EX2FILESYSTEM
{
private:
    SuperBlock super_block;
    BlockGroupDescriptorTable descriptor_table;
    unordered_map<int, MyFile *> mp; // mapping file_descriptors to MyFile struct
    int created_file_descriptors = 0;
    BlockDescriptorManager *block_manager;
    Disk *disk_manager;
public:
    EX2FILESYSTEM(BlockDescriptorManager *descriptor_manager, Disk *disk_manager);

    ~EX2FILESYSTEM();

    int create_file(char *file_name, int file_name_size, int file_permissions, bool is_directory);

    int my_open(int inode_number, int mask_permissions);

    int my_close(int file_descriptor);

    int my_file_system_write(int file_descriptor, char *buffer, int size_bytes_to_write);

    int my_file_system_write_at(int file_descriptor, int start_position, char *buffer, int size);

    int my_file_system_read(int file_descriptor, char *buffer, int size_bytes_to_read);

    int my_file_system_seek(int file_desriptor, int position);

    int delete_file(int inode_number);

    int rename_file(int inode_number, char *file_name, int file_name_size);

    int increment_hard_link_count(int inode_number);

    MyFile *get_file_info(int file_descriptor);
};

#endif
