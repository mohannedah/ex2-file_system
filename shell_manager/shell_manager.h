#ifndef SHELL_MANAGER_H
#define SHEL_MANAGER_H
#include "../shared_types.h"
#include "../global_dependecies.h"

struct DirectoryStructure
{
public:

    DirectoryStructure() {
        
    };

    char file_name[50];
    int inode_number;
    uint16_t mask_permissions;
    bool is_directory;

    void write_char_bytes(char *buffer)
    {
        memcpy(buffer, this->file_name, sizeof(file_name));

        buffer += sizeof(this->file_name);

        memcpy(buffer, this + sizeof(file_name), sizeof(int) + sizeof(uint16_t));
    };

    void copy_char_bytes(char *buffer)
    {
        memcpy(this->file_name, buffer, sizeof(file_name));

        buffer += sizeof(this->file_name);

        memcpy(this + sizeof(file_name), buffer, sizeof(int) + sizeof(uint16_t));
    };

    void copy_directory_info(DirectoryStructure *dir_struct)
    {
        char buffer[sizeof(DirectoryStructure)];

        dir_struct->write_char_bytes(buffer);

        copy_char_bytes(buffer);
    };

    DirectoryStructure(DirectoryStructure &dir_struct) {
        memcpy(this->file_name, dir_struct.file_name, sizeof(this->file_name));

        this->inode_number = dir_struct.inode_number;

        this->is_directory = dir_struct.is_directory;

        this->mask_permissions = dir_struct.mask_permissions;
    }
};

char curr_directory_name[50];

int curr_directory_inode_number;

int change_directory(string file_name);

int create_file(string file_name);

int remove_file(string file_name);

int create_directory(string file_name);

int write_file(string file_name, char *buffer, int buffer_size);

int read_file(string file_name, char *buffer, int read_size);

void print_curr_directory_name();

#endif
