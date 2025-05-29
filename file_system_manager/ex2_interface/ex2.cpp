#include "ex2.h"

static inline void read_super_block(SuperBlock *super_block)
{
    MemoryBlock block;

    retreive_block_disk_helper(1, &block);

    memcpy(super_block, block.data, sizeof(SuperBlock));
};

EX2FILESYSTEM::EX2FILESYSTEM(BlockDescriptorManager *descriptor_manager)
{
    this->block_manager = descriptor_manager;

    read_super_block(&this->super_block);
};

EX2FILESYSTEM::~EX2FILESYSTEM() {

};

static inline void read_helper(int starting_byte, int ending_byte, char *&destination_buffer, char *src_buffer)
{
    int size = ending_byte - starting_byte;

    src_buffer += starting_byte;

    memcpy(destination_buffer, src_buffer, size);

    destination_buffer += size;
};

static inline void write_helper(int starting_byte, int ending_byte, char *destination_buffer, char *&src_buffer)
{
    int size = ending_byte - starting_byte;

    memcpy(destination_buffer + starting_byte, src_buffer, size);

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
    BlockGroupINode inode;

    MyFile *my_file = (MyFile *)malloc(sizeof(MyFile));

    this->block_manager->read_inode_info(inode_number, &inode);

    if (inode.hard_link_count == 0)
    {
        perror("File with this inode is not found");
        exit(-1);
    };

    check_permissions(mask_permissions, inode);

    my_file->inode_number = inode_number;

    my_file->position = 0;

    my_file->file_size = inode.file_size;

    my_file->is_directory = inode.is_directory;

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

static inline int get_data_block(int inode_number, int block_idx, BlockGroupINode &inode, BlockDescriptorManager *block_manager, MemoryBlock *block)
{
    int is_occupied = inode.blocks[block_idx] != 0;

    if (is_occupied)
        return retreive_block_disk_helper(inode.blocks[block_idx], block);

    int free_block_number = block_manager->find_first_empty_data_block(inode_number);

    inode.blocks[block_idx] = free_block_number;

    return retreive_block_disk_helper(inode.blocks[block_idx], block);
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

    this->block_manager->read_inode_info(my_file_info->inode_number, &inode);

    if (inode.hard_link_count == 0)
    {
        cout << "INode is corrupted" << endl;
        return -1;
    }

    inode.last_access_time = chrono::duration_cast<chrono::seconds>(chrono::system_clock::now().time_since_epoch()).count();

    int starting_block = my_file_info->position / BLOCK_SIZE, ending_block = (my_file_info->position + size - 1) / BLOCK_SIZE;

    MemoryBlock block;

    if (starting_block == ending_block)
    {
        int status = retreive_block_disk_helper(inode.blocks[starting_block], &block);

        if (status == -1)
        {
            perror("Invalid block number!");
            exit(0);
        }

        read_helper(my_file_info->position % BLOCK_SIZE, (my_file_info->position % BLOCK_SIZE) + size, buffer, block.data);
    }
    else
    {
        int status = retreive_block_disk_helper(inode.blocks[starting_block], &block);

        if (status == -1)
        {
            perror("Invalid block number!");
            exit(0);
        }

        read_helper(my_file_info->position % BLOCK_SIZE, BLOCK_SIZE, buffer, block.data);

        for (int curr_block_idx = starting_block + 1; curr_block_idx < ending_block; curr_block_idx += 1)
        {
            int block_number = inode.blocks[curr_block_idx];

            status = retreive_block_disk_helper(block_number, &block);

            if (status == -1)
            {
                perror("Invalid block number!");
                exit(0);
            }

            read_helper(0, BLOCK_SIZE, buffer, block.data);
        };

        int ending_byte = (my_file_info->position + size - 1) % BLOCK_SIZE;

        status = retreive_block_disk_helper(inode.blocks[ending_block], &block);

        read_helper(0, ending_byte + 1, buffer, block.data);
    };

    my_file_info->position += size;

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

    this->block_manager->read_inode_info(my_file_info->inode_number, &inode);

    if (inode.hard_link_count == 0)
    {
        cout << "INode is corrupted." << endl;
        return -1;
    }

    inode.file_size = new_size;

    inode.last_modification_time = chrono::duration_cast<chrono::seconds>(chrono::system_clock::now().time_since_epoch()).count();

    int starting_block = (my_file_info->file_size / BLOCK_SIZE), ending_block = ((my_file_info->position + size - 1) / BLOCK_SIZE);

    MemoryBlock block;

    if (starting_block == ending_block)
    {
        get_data_block(my_file_info->inode_number, starting_block, inode, this->block_manager, &block);

        int starting_byte = my_file_info->file_size % BLOCK_SIZE, ending_byte = starting_byte + size;

        write_helper(starting_byte, ending_byte, block.data, buffer);

        write_block_disk_helper(inode.blocks[starting_block], block.data);
    }
    else
    {
        get_data_block(my_file_info->inode_number, starting_block, inode, this->block_manager, &block);

        int starting_byte = my_file_info->position % BLOCK_SIZE, ending_byte = BLOCK_SIZE;

        write_helper(starting_byte, ending_byte, block.data, buffer);

        write_block_disk_helper(inode.blocks[starting_block], block.data);

        for (int curr_block_idx = starting_block; curr_block_idx < ending_block; curr_block_idx++)
        {
            get_data_block(my_file_info->inode_number, curr_block_idx, inode, this->block_manager, &block);

            write_helper(0, BLOCK_SIZE, block.data, buffer);

            write_block_disk_helper(inode.blocks[curr_block_idx], block.data);
        };

        get_data_block(my_file_info->inode_number, ending_block, inode, this->block_manager, &block);

        ending_byte = (my_file_info->position + size - 1) % BLOCK_SIZE;

        write_helper(0, ending_byte + 1, block.data, buffer);

        write_block_disk_helper(inode.blocks[ending_block], block.data);
    };

    my_file_info->file_size = new_size;

    return 0;
};

int EX2FILESYSTEM::my_file_system_write_at(int file_descriptor, int start_position, char *buffer, int size)
{
    if (!mp.count(file_descriptor))
    {
        cout << "Invalid file descriptor" << endl;
        return -1;
    }

    MyFile *my_file = this->mp[file_descriptor];

    if (my_file->permissions & (1 << WRITE_BIT))
    {
        cout << "File has no write permissions" << endl;
        return -1;
    }

    BlockGroupINode inode;

    int status = this->block_manager->read_inode_info(my_file->inode_number, &inode);

    if (status == -1)
    {
        cout << "INode is not found" << endl;
        return -1;
    }

    inode.last_modification_time = chrono::duration_cast<chrono::seconds>(chrono::system_clock::now().time_since_epoch()).count();

    int end_position = start_position + size;

    int starting_block = start_position / BLOCKS_PER_INODE, ending_block = (end_position - 1) / BLOCKS_PER_INODE;

    MemoryBlock block;

    if (starting_block == ending_block)
    {
        int status = get_data_block(my_file->inode_number, starting_block, inode, this->block_manager, &block);

        write_helper(start_position % BLOCK_SIZE, end_position, block.data, buffer);

        write_block_disk_helper(inode.blocks[starting_block], block.data);
    }
    else
    {
        int status = get_data_block(my_file->inode_number, starting_block, inode, this->block_manager, &block);

        write_helper(start_position % BLOCK_SIZE, BLOCK_SIZE, block.data, buffer);

        write_block_disk_helper(inode.blocks[starting_block], block.data);

        for (int block_idx = starting_block + 1; block_idx <= ending_block - 1; block_idx += 1)
        {
            status = get_data_block(my_file->inode_number, block_idx, inode, this->block_manager, &block);

            write_helper(0, BLOCK_SIZE, block.data, buffer);

            write_block_disk_helper(inode.blocks[block_idx], block.data);
        }

        status = get_data_block(my_file->inode_number, ending_block, inode, this->block_manager, &block);

        write_helper(0, end_position % BLOCK_SIZE, block.data, buffer);

        write_block_disk_helper(inode.blocks[ending_block], block.data);
    };

    return 1;
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

static inline void set_vacant_inode(BlockGroupINode *vacant_inode, char *file_name, int file_name_size, int file_permissions, bool is_directory)
{
    vacant_inode->hard_link_count += 1;

    vacant_inode->file_size = 0;

    vacant_inode->creation_time = get_current_time();

    vacant_inode->last_access_time = get_current_time();

    vacant_inode->inode_mode = file_permissions;

    memcpy(vacant_inode->file_name, file_name, file_name_size);

    vacant_inode->is_directory = is_directory;
};

int EX2FILESYSTEM::create_file(char *file_name, int file_name_size, int file_permissions, bool is_directory)
{
    BlockGroupDescriptor block_descriptor;

    int vacant_inode_number = -1, curr_block_number = 0;

    while (vacant_inode_number == -1 && curr_block_number < NUM_GROUPS)
    {
        int status = this->block_manager->read_block_descriptor(curr_block_number, &block_descriptor);

        vacant_inode_number = this->block_manager->find_first_empty_inode(curr_block_number, &block_descriptor);

        curr_block_number += 1;
    };

    if (vacant_inode_number == -1)
    {
        cout << "No vacant Inodes were found!" << endl;
        return -1;
    }

    BlockGroupINode vacant_inode;

    this->block_manager->read_inode_info(vacant_inode_number, &vacant_inode);

    set_vacant_inode(&vacant_inode, file_name, file_name_size, file_permissions, is_directory);

    return vacant_inode_number;
};

int EX2FILESYSTEM::delete_file(int inode_number)
{
    BlockGroupINode inode;

    int status = this->block_manager->read_inode_info(inode_number, &inode);

    if(status == -1) 
    {
        cout << "Inode is not found!" << endl;
        return -1;
    }

    if (inode.hard_link_count == 0)
    {
        cout << "File is already deleted!" << endl;
        return -1;
    }

    inode.hard_link_count -= 1;

    if (inode.hard_link_count == 0)
    {
        for (int i = 0; i < BLOCKS_PER_INODE; i++)
            inode.blocks[i] = 0;

        int group_number = inode_number / NUM_INODES_PER_GROUP;

        BlockGroupDescriptor block_descriptor;

        this->block_manager->read_block_descriptor(group_number, &block_descriptor);

        block_descriptor.free_inodes_count -= 1;
    };
    return 1;
};


int EX2FILESYSTEM::rename_file(int inode_number, char *file_name, int file_name_size)
{
    BlockGroupINode inode;

    this->block_manager->write_inode_info(inode_number, &inode);

    if (inode.hard_link_count == 0)
    {
        cout << "This inode is not attached to a file!" << endl;
        return -1;
    }

    memcpy(inode.file_name, file_name, file_name_size);

    return 1;
};

int EX2FILESYSTEM::increment_hard_link_count(int inode_number) {
    BlockGroupINode inode;
    
    int status = this->block_manager->read_inode_info(inode_number, &inode);

    if(status == -1) {
        cout << "Inode with this inode number is not found!" << endl;
        return -1;
    }
    
    inode.hard_link_count += 1;

    status = this->block_manager->write_inode_info(inode_number, &inode);

    if(status == -1) {
        cout << "An error has occured while flushing the inode info!" << endl;
        return -1;
    }

    return 1;
};

MyFile *EX2FILESYSTEM::get_file_info(int file_descriptor)
{
    if (!this->mp.count(file_descriptor))
        return nullptr;

    return this->mp[file_descriptor];
};
