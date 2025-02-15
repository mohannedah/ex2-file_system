#include "ex2.h"

static inline void read_super_block(SuperBlock *super_block, Disk *disk_manager)
{
    MemoryBlock block;

    retreive_block_disk_helper(1, &block, disk_manager);

    memcpy(super_block, block.data, sizeof(SuperBlock));
};

EX2FILESYSTEM::EX2FILESYSTEM(BufferPoolManager *buffer_pool, BlockDescriptorManager *descriptor_manager, Disk *disk_manager)
{
    this->buffer_pool_manager = buffer_pool;
    this->block_manager = descriptor_manager;
    this->disk_manager = disk_manager;

    read_super_block(&this->super_block, this->disk_manager);
};

static inline void read_helper(int starting_byte, int ending_byte, char *destination_buffer, char *src_buffer)
{
    int size = ending_byte - starting_byte;

    src_buffer += starting_byte;

    memcpy(destination_buffer, src_buffer, size);

    destination_buffer += size;
};

static inline void write_helper(int starting_byte, int ending_byte, char *destination_buffer, char *src_buffer)
{
    int size = ending_byte - starting_byte;

    memcpy(destination_buffer, src_buffer, size);

    src_buffer += size;
};

static inline void check_permissions(int mask_permissions, BlockGroupINode &inode)
{
    int temp = mask_permissions | (READ_BIT | WRITE_BIT);

    if (temp > (READ_BIT | WRITE_BIT))
    {
        perror("Only valid permissions are read and write!");
        exit(0);
    }

    temp = mask_permissions | (inode.inode_mode);

    if (mask_permissions != inode.inode_mode)
    {
        perror("Invalid permissions");
        exit(0);
    }
};

int EX2FILESYSTEM::my_open(int inode_number, int mask_permissions)
{
    MyFile *my_file = (MyFile *)malloc(sizeof(MyFile));

    BlockGroupINode inode;

    int status = this->block_manager->read_inode_info(inode_number, &inode);

    if (status == -1)
    {
        perror("Inode number is not found");
        exit(0);
    }

    check_permissions(mask_permissions, inode);

    my_file->inode_number = inode_number;

    my_file->position = 0;

    my_file->file_size = inode.file_size;

    int file_descriptor_id = created_file_descriptors++;

    mp[file_descriptor_id] = my_file;

    return file_descriptor_id;
};

int EX2FILESYSTEM::my_close(int file_descriptor)
{
    if (!mp.count(file_descriptor))
        return -1;

    free(mp[file_descriptor]);

    mp.erase(file_descriptor);

    return 0;
};

static inline MemoryBlock *get_data_block(int inode_number, int block_idx, BlockGroupINode &inode, BufferPoolManager *buffer_pool, BlockDescriptorManager *block_manager)
{
    int is_occupied = inode.blocks[block_idx] != -1;

    if (is_occupied)
        return buffer_pool->get_block(inode.blocks[block_idx]);

    int free_block_number = block_manager->find_first_empty_data_block(inode_number);

    inode.blocks[block_idx] = free_block_number;

    return buffer_pool->get_block(inode.blocks[block_idx]);
};

int EX2FILESYSTEM::my_file_system_read(int file_descriptor, char *buffer, int size)
{
    if (!mp.count(file_descriptor))
    {
        perror("Invalid file descriptor.");
        exit(0);
    };

    MyFile *my_file_info = mp[file_descriptor];

    if (size + my_file_info->position > my_file_info->file_size)
    {
        cout << "Cannot read beyond the file size!" << endl;
        return -1;
    }

    BlockGroupINode inode;

    int status = this->block_manager->read_inode_info(my_file_info->inode_number, &inode);

    inode.last_access_time = chrono::duration_cast<chrono::seconds>(chrono::system_clock::now().time_since_epoch()).count();

    if (status == -1)
    {
        perror("Invalid Inode Number.");
        exit(0);
    }

    char destination_buffer[size];

    char *start_ptr = destination_buffer;

    int starting_block = my_file_info->position / BLOCK_SIZE, ending_block = (my_file_info->position + size - 1) / BLOCK_SIZE;

    if (starting_block == ending_block)
    {
        MemoryBlock *block = this->buffer_pool_manager->get_block(inode.blocks[starting_block]);

        if (block == nullptr)
        {
            perror("Invalid block number!");
            exit(0);
        }
        read_helper(my_file_info->position % BLOCK_SIZE, (my_file_info->position % BLOCK_SIZE) + size, destination_buffer, block->data);
    }
    else
    {
        MemoryBlock *block = this->buffer_pool_manager->get_block(inode.blocks[starting_block]);

        if (block == nullptr)
        {
            perror("Invalid block number!");
            exit(0);
        }

        read_helper(my_file_info->position % BLOCK_SIZE, BLOCK_SIZE, destination_buffer, block->data);

        for (int curr_block_idx = starting_block + 1; curr_block_idx < ending_block; curr_block_idx += 1)
        {
            int block_number = inode.blocks[curr_block_idx];

            block = this->buffer_pool_manager->get_block(block_number);

            if (block == nullptr)
            {
                perror("Invalid block number!");
                exit(0);
            }
            read_helper(0, BLOCK_SIZE, destination_buffer, block->data);
        };

        int ending_byte = (my_file_info->position + size - 1) % BLOCK_SIZE;

        block = this->buffer_pool_manager->get_block(inode.blocks[ending_block]);

        read_helper(0, ending_byte + 1, destination_buffer, block->data);
    };

    memcpy(buffer, start_ptr, size);

    return 0;
};

int EX2FILESYSTEM::my_file_system_write(int file_descriptor, char *buffer, int size)
{
    if (!mp.count(file_descriptor))
    {
        perror("Invalid file descriptor");
        exit(0);
    }

    MyFile *my_file_info = mp[file_descriptor];

    int new_size = size + my_file_info->file_size;

    if (new_size > (BLOCKS_PER_INODE * BLOCK_SIZE))
    {
        cerr << "File size has a limit of " << (BLOCKS_PER_INODE * BLOCK_SIZE) << endl;
        exit(0);
    }

    BlockGroupINode inode;

    int status = this->block_manager->read_inode_info(my_file_info->inode_number, &inode);

    inode.last_modification_time = chrono::duration_cast<chrono::seconds>(chrono::system_clock::now().time_since_epoch()).count();

    if (status == -1)
    {
        perror("Invalid Inode number!");
        exit(0);
    }

    int starting_block = (my_file_info->position / BLOCK_SIZE), ending_block = ((my_file_info->position + size - 1) % BLOCK_SIZE);

    if (starting_block == ending_block)
    {
        MemoryBlock *block = get_data_block(my_file_info->inode_number, starting_block, inode, this->buffer_pool_manager, this->block_manager);

        int starting_byte = my_file_info->position % BLOCK_SIZE, ending_byte = starting_byte + size;

        write_helper(starting_byte, ending_byte, block->data, buffer);
    }
    else
    {
        MemoryBlock *block = get_data_block(my_file_info->inode_number, starting_block, inode, this->buffer_pool_manager, this->block_manager);

        int starting_byte = my_file_info->position % BLOCK_SIZE, ending_byte = BLOCK_SIZE;

        write_helper(starting_byte, ending_byte, block->data, buffer);

        for (int curr_block_idx = starting_block; curr_block_idx < ending_block; curr_block_idx++)
        {
            block = get_data_block(my_file_info->inode_number, curr_block_idx, inode, this->buffer_pool_manager, this->block_manager);

            write_helper(0, BLOCK_SIZE, block->data, buffer);
        };

        block = get_data_block(my_file_info->inode_number, ending_block, inode, this->buffer_pool_manager, this->block_manager);

        ending_byte = (my_file_info->position + size - 1) % BLOCK_SIZE;

        write_helper(0, ending_byte + 1, block->data, buffer);
    };
    return 0;
};

int EX2FILESYSTEM::my_file_system_create_file(char *file_name, int file_permissions)
{
    BlockGroupDescriptor block_group;

    int empty_inode_number;

    for (int curr_block_group_idx = 0; curr_block_group_idx < NUM_GROUPS; curr_block_group_idx++)
    {
        this->block_manager->read_block_descriptor(curr_block_group_idx, &block_group);

        empty_inode_number = this->block_manager->find_first_empty_inode(curr_block_group_idx, &block_group);

        if (empty_inode_number != -1)
            break;
    };

    if (empty_inode_number == -1)
    {
        cout << "No empty inodes are available" << endl;
        return -1;
    }

    BlockGroupINode inode;

    int status = this->block_manager->read_inode_info(empty_inode_number, &inode);

    int creation_time = chrono::duration_cast<chrono::seconds>(chrono::system_clock::now().time_since_epoch()).count();

    inode.creation_time = creation_time;

    inode.last_modification_time = creation_time;

    strcpy(inode.file_name, file_name);

    inode.hard_link_count = 1;

    inode.uid = empty_inode_number;

    inode.inode_mode = file_permissions;

    return 0;
};

int EX2FILESYSTEM::my_file_system_seek(int file_descriptor, int position)
{
    if (!mp.count(file_descriptor))
    {
        perror("Invalid file descriptor.");
        exit(0);
    }

    MyFile *my_file_info = mp[file_descriptor];

    if (my_file_info->file_size <= position)
    {
        perror("Seeking position is greater than the file size.");
        exit(0);
    }

    my_file_info->position = position;

    return 0;
};