#ifndef SHELL_MANAGER_H
#define SHELL_MANAGER_H
#include "../shared_types.h"
#include "../file_system_manager/ex2_interface/ex2.h"
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
        char* start_ptr = (char*)this;        

        memcpy(buffer, start_ptr, sizeof(file_name));

        buffer += sizeof(this->file_name);

        start_ptr = (char*)(&inode_number);
        
        memcpy(buffer, start_ptr, sizeof(int) + sizeof(uint16_t) + sizeof(is_directory));        
    };

    void copy_char_bytes(char *buffer)
    {
        memcpy(this->file_name, buffer, sizeof(file_name));

        buffer += sizeof(this->file_name);
        
        char* start_ptr = (char*)(&inode_number);

        memcpy(start_ptr, buffer, sizeof(int) + sizeof(uint16_t) + sizeof(is_directory));
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




class ShellManager {
    private:
        void set_vacant_dir_struct(DirectoryStructure *dir_struct, int inode_number, string &file_name, uint16_t mask_permissions, bool is_directory);

        int read_directory_info(int inode_number, vector<DirectoryStructure *> &v, EX2FILESYSTEM *ex2_file_system);

        int extract_permissions(string &permissions);

        int get_file_info(int dir_inode_number, string &file_name, DirectoryStructure *dir_struct, EX2FILESYSTEM *ex2_file_system);

        void split_file_name(string &file_name, vector<string> &v);

        int traverse_hierachy(vector<string> &splitted_file_name, DirectoryStructure &dir_struct, int end, EX2FILESYSTEM *ex2_file_system);
    public:
    char curr_directory_name[50];

    int curr_directory_inode_number = 6;

    EX2FILESYSTEM *ex2_file_system;

    ShellManager(EX2FILESYSTEM *ex2_file_system);

    int change_directory(string &file_name);

    int create_file(string &file_name, string &file_permissions, bool is_directory);

    int remove_file(string &file_name);

    int create_directory(string &file_name);

    int write_file(string &file_name, char *buffer, int buffer_size);

    int read_file(string &file_name, char *buffer, int read_size);

    int create_hard_link(string &file_name, string &corresponding_file_name);

    int create_soft_link(string &file_name, string &corresponding_file_name);

    void print_curr_directory_name();

};

#endif
