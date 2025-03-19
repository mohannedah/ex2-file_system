#include "shell_manager.h"

static inline int get_file_info(int dir_inode_number, string &file_name, DirectoryStructure *dir_struct)
{
    vector<DirectoryStructure *> v;

    int status = read_directory_info(dir_inode_number, v);

    int state = -1;

    for (auto d : v)
    {
        if (strcmp(file_name.c_str(), d->file_name) == 0)
        {
            dir_struct->copy_directory_info(d);
            state = 0;
            break;
        }
    };

    for (auto d : v)
    {
        delete d;
    }

    return state;
};

static inline int extract_permissions(string &permissions)
{
    int permission = 0;

    for (char chr : permissions)
    {
        if (chr == 'R')
        {
            permission |= READ_BIT;
        }
        else if (chr == 'W')
        {
            permission |= WRITE_BIT;
        }
        else
        {
            cout << "Invalid permission " << chr << endl;
            return -1;
        }
    };

    return permission;
};

static inline int read_directory_info(int inode_number, vector<DirectoryStructure *> &v)
{
    int file_descriptor = ex2_file_system->my_open(inode_number, READ_BIT);

    MyFile *my_file = ex2_file_system->get_file_info(file_descriptor);

    int file_size = my_file->file_size;

    char buffer[file_size];

    int status = ex2_file_system->my_file_system_seek(file_descriptor, 0);

    if (status == -1)
    {
        cout << "An error happened while seeking the file position." << endl;
        return -1;
    }

    if (!my_file->is_directory)
    {
        // cout << "File " << my_file->;
        return -1;
    };

    status = ex2_file_system->my_file_system_read(file_descriptor, buffer, file_size);

    if (status == -1)
    {
        cout << "An error happened while reading the directory information" << endl;

        ex2_file_system->my_close(file_descriptor);

        return -1;
    }

    int total_files = file_size / sizeof(DirectoryStructure);

    int files_read = 0;

    while (total_files--)
    {
        DirectoryStructure *dir_struct = new DirectoryStructure();

        dir_struct->copy_char_bytes(buffer + (files_read * sizeof(DirectoryStructure)));

        files_read += 1;

        v.push_back(dir_struct);
    }

    ex2_file_system->my_close(file_descriptor);

    return 1;
};

static inline void split_file_name(string &file_name, vector<string> &v)
{
    int curr_idx = 0;

    while (curr_idx < file_name.size())
    {
        string curr_file_name = "";

        while (curr_idx < file_name.size() && file_name[curr_idx] != '/')
        {
            curr_file_name += file_name[curr_idx];
            curr_idx += 1;
        };
        curr_idx += 1;

        v.push_back(curr_file_name);
    }
}

static inline int traverse_hierachy(vector<string> &splitted_file_name, DirectoryStructure &dir_struct, int end)
{
    int curr_inode_number = curr_directory_inode_number;

    for (int i = 0; i < end; i++)
    {
        int status = get_file_info(curr_inode_number, splitted_file_name[i], &dir_struct);

        if (status == -1)
        {
            cout << "Directory doesn't exist" << endl;
            return -1;
        }

        if (!dir_struct.is_directory)
        {
            cout << "File " << dir_struct.file_name << "is not a directory" << endl;
            return -1;
        }

        curr_inode_number = dir_struct.inode_number;
    };

    return curr_inode_number;
};

static inline void set_vacant_dir_struct(DirectoryStructure *dir_struct, int inode_number, string &file_name, uint16_t mask_permissions, bool is_directory)
{
    dir_struct->inode_number = inode_number;

    memcpy(dir_struct->file_name, file_name.c_str(), strlen(file_name.c_str()));

    dir_struct->mask_permissions = mask_permissions;

    dir_struct->is_directory = is_directory;
};

int change_directory(string &file_name)
{
    vector<string> splitted_file_name;

    split_file_name(file_name, splitted_file_name);

    DirectoryStructure dir_struct;

    int ending_inode_number = traverse_hierachy(splitted_file_name, dir_struct, splitted_file_name.size());

    if (ending_inode_number == -1)
    {
        return -1;
    }

    memcpy(curr_directory_name, dir_struct.file_name, sizeof(dir_struct.file_name));

    curr_directory_inode_number = ending_inode_number;

    return 0;
};

int create_file(string file_name, string permissions, bool is_directory)
{
    vector<string> splitted_file_name;

    split_file_name(file_name, splitted_file_name);

    DirectoryStructure dir_struct;

    int curr_inode_number = traverse_hierachy(splitted_file_name, dir_struct, splitted_file_name.size() - 1);

    int permission = extract_permissions(permissions);

    if (permission == -1)
    {
        return -1;
    }

    int created_file_inode_number = ex2_file_system->create_file(const_cast<char *>(splitted_file_name.back().c_str()), splitted_file_name.back().size(), permission, is_directory);

    int file_descriptor = ex2_file_system->my_open(curr_inode_number, WRITE_BIT);

    char buffer[sizeof(DirectoryStructure)];

    set_vacant_dir_struct(&dir_struct, created_file_inode_number, file_name, permission, 0);

    dir_struct.write_char_bytes(buffer);

    ex2_file_system->my_file_system_write(file_descriptor, buffer, sizeof(DirectoryStructure));

    ex2_file_system->my_close(file_descriptor);

    return 0;
};

int remove_file(string file_name)
{
    vector<string> splitted_file_name;

    split_file_name(file_name, splitted_file_name);

    DirectoryStructure dir_struct;

    string &to_be_removed_file_name = splitted_file_name.back();

    int dir_inode_number = traverse_hierachy(splitted_file_name, dir_struct, splitted_file_name.size() - 1);

    if (dir_inode_number == -1)
    {
        return -1;
    }

    int status = get_file_info(dir_inode_number, to_be_removed_file_name, &dir_struct);

    if (status == -1)
    {
        cout << "File not found" << endl;
        return -1;
    }

    status = ex2_file_system->delete_file(dir_struct.inode_number);
};

int create_directory(string file_name)
{
    int mask_permissions = READ_BIT | WRITE_BIT;

    vector<string> splitted_file_name;

    split_file_name(file_name, splitted_file_name);

    int curr_inode_number = curr_directory_inode_number;

    DirectoryStructure dir_struct;

    for (int i = 0; i < splitted_file_name.size() - 1; i++)
    {
        int status = get_file_info(curr_inode_number, splitted_file_name[i], &dir_struct);

        if (status == -1)
        {
            cout << "File " << dir_struct.file_name << " is not found!" << endl;
            return -1;
        };

        if (!dir_struct.is_directory)
        {
            cout << "File " << dir_struct.file_name << "is not a directory" << endl;
            return -1;
        }

        curr_inode_number = dir_struct.inode_number;
    };

    int created_file_inode_number = ex2_file_system->create_file(const_cast<char *>(splitted_file_name[splitted_file_name.size() - 1].c_str()), splitted_file_name[splitted_file_name.size() - 1].size() + 1, mask_permissions, 1);

    int file_descriptor = ex2_file_system->my_open(curr_inode_number, mask_permissions);

    set_vacant_dir_struct(&dir_struct, created_file_inode_number, file_name, mask_permissions, 1);

    char buffer[sizeof(DirectoryStructure)];

    dir_struct.write_char_bytes(buffer);

    int status = ex2_file_system->my_file_system_write(file_descriptor, buffer, sizeof(DirectoryStructure));

    if (status == -1)
    {
        cout << "Cannot create a new file in this drectory" << endl;
        return -1;
    };

    ex2_file_system->my_close(file_descriptor);

    return 0;
};

void print_curr_directory_name()
{
    cout << curr_directory_name << endl;
}