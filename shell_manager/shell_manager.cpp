#include "shell_manager.h"


int ShellManager::get_file_info(int dir_inode_number, string &file_name, DirectoryStructure *dir_struct, EX2FILESYSTEM *ex2_file_system)
{
    vector<DirectoryStructure *> v;

    int status = read_directory_info(dir_inode_number, v, ex2_file_system);

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

int ShellManager::extract_permissions(string &permissions)
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

int ShellManager::read_directory_info(int inode_number, vector<DirectoryStructure *> &v, EX2FILESYSTEM* ex2_file_system)
{
    int file_descriptor = ex2_file_system->my_open(inode_number, READ_BIT);
    
    MyFile *my_file = ex2_file_system->get_file_info(file_descriptor);
    
    int file_size = my_file->file_size;


    if(!file_size) {
        ex2_file_system->my_close(file_descriptor);
        return 0;
    }

    char buffer[file_size];

    int status = ex2_file_system->my_file_system_seek(file_descriptor, 0);

    if (status == -1)
    {
        cout << "An error happened while seeking the file position." << endl;
        return -1;
    }

    if (!my_file->is_directory)
    {
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

    while (total_files > 0)
    {
        DirectoryStructure *dir_struct = new DirectoryStructure();

        dir_struct->copy_char_bytes(buffer + (files_read * sizeof(DirectoryStructure)));

        files_read += 1;

        v.push_back(dir_struct);

        total_files -= 1;
    }

    ex2_file_system->my_close(file_descriptor);

    return 1;
};

void ShellManager::split_file_name(string &file_name, vector<string> &v)
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

int ShellManager::traverse_hierachy(vector<string> &splitted_file_name, DirectoryStructure &dir_struct, int end, EX2FILESYSTEM *ex2_file_system)
{
    int curr_inode_number = curr_directory_inode_number;

    for (int i = 0; i < end; i++)
    {
        int status = get_file_info(curr_inode_number, splitted_file_name[i], &dir_struct, ex2_file_system);

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

void ShellManager::set_vacant_dir_struct(DirectoryStructure *dir_struct, int inode_number, string &file_name, uint16_t mask_permissions, bool is_directory)
{
    dir_struct->inode_number = inode_number;

    memcpy(dir_struct->file_name, file_name.c_str(), file_name.size() + 1);

    dir_struct->mask_permissions = mask_permissions;

    dir_struct->is_directory = is_directory;
};


ShellManager::ShellManager(EX2FILESYSTEM* ex2_file_system) {
    this->ex2_file_system = ex2_file_system;
};

int ShellManager::change_directory(string &file_name)
{
    vector<string> splitted_file_name;

    split_file_name(file_name, splitted_file_name);

    DirectoryStructure dir_struct;

    int ending_inode_number = traverse_hierachy(splitted_file_name, dir_struct, splitted_file_name.size(), ex2_file_system);

    if (ending_inode_number == -1)
    {
        return -1;
    }

    memcpy(curr_directory_name, dir_struct.file_name, sizeof(dir_struct.file_name));

    curr_directory_inode_number = ending_inode_number;

    return 0;
};

int ShellManager::create_file(string &file_name, string &permissions, bool is_directory)
{
    vector<string> splitted_file_name;
    
    split_file_name(file_name, splitted_file_name);

    DirectoryStructure dir_struct;

    int curr_inode_number = traverse_hierachy(splitted_file_name, dir_struct, splitted_file_name.size() - 1, ex2_file_system);

    if(curr_inode_number == -1) return -1;

    int permission = extract_permissions(permissions);

    if (permission == -1)
    {
        return -1;
    }

    int created_file_inode_number = ex2_file_system->create_file(const_cast<char *>(splitted_file_name.back().c_str()), splitted_file_name.back().size(), permission, is_directory);

    int file_descriptor = ex2_file_system->my_open(curr_inode_number, WRITE_BIT);

    char buffer[sizeof(DirectoryStructure)];

    set_vacant_dir_struct(&dir_struct, created_file_inode_number, file_name, permission, is_directory);

    dir_struct.write_char_bytes(buffer);

    ex2_file_system->my_file_system_write(file_descriptor, buffer, sizeof(DirectoryStructure));

    ex2_file_system->my_close(file_descriptor);

    return 0;
};

int ShellManager::remove_file(string &file_name)
{
    vector<string> splitted_file_name;

    split_file_name(file_name, splitted_file_name);

    DirectoryStructure dir_struct;

    string &to_be_removed_file_name = splitted_file_name.back();

    int dir_inode_number = traverse_hierachy(splitted_file_name, dir_struct, splitted_file_name.size() - 1, ex2_file_system);

    if (dir_inode_number == -1)
    {
        return -1;
    }

    int status = get_file_info(dir_inode_number, to_be_removed_file_name, &dir_struct, ex2_file_system);

    if (status == -1)
    {
        cout << "File not found" << endl;
        return -1;
    }

    status = ex2_file_system->delete_file(dir_struct.inode_number);

    return status;
};

int ShellManager::create_directory(string &file_name)
{
    int mask_permissions = READ_BIT | WRITE_BIT;

    vector<string> splitted_file_name;

    split_file_name(file_name, splitted_file_name);

    int curr_inode_number = curr_directory_inode_number;

    DirectoryStructure dir_struct;

    for (int i = 0; i < splitted_file_name.size() - 1; i++)
    {
        int status = get_file_info(curr_inode_number, splitted_file_name[i], &dir_struct, ex2_file_system);

        if (status == -1)
        {
            cout << "File " << splitted_file_name[i] << " is not found!" << endl;
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

int ShellManager::create_hard_link(string &file_name, string &corresponding_file_name)
{
    vector<string> splitted_file_name;

    split_file_name(file_name, splitted_file_name);

    int curr_inode_number = curr_directory_inode_number;

    DirectoryStructure dir_struct;

    for (int i = 0; i < splitted_file_name.size() - 1, i++;)
    {
        int status = get_file_info(curr_inode_number, splitted_file_name[i], &dir_struct, ex2_file_system);

        if (status == -1)
        {
            cout << "File " << splitted_file_name[i] << " is not found!" << endl;
            return -1;
        }

        if (!dir_struct.is_directory)
        {
            cout << "File " << splitted_file_name[i] << " is not a directory!" << endl;
            return -1;
        }

        curr_inode_number = dir_struct.inode_number;
    };

    DirectoryStructure new_dir_struct;

    string extracted_file_name = splitted_file_name.back();

    memcpy(new_dir_struct.file_name, extracted_file_name.c_str(), extracted_file_name.size() + 1);

    new_dir_struct.file_name[extracted_file_name.size()] = '\0';

    split_file_name(corresponding_file_name, splitted_file_name);

    curr_inode_number = curr_directory_inode_number;

    for (int i = 0; i < splitted_file_name.size() - 1; i++)
    {
        int status = get_file_info(curr_inode_number, splitted_file_name[i], &dir_struct, ex2_file_system);

        if (status == -1)
        {
            cout << "File " << splitted_file_name[i] << " is not found!" << endl;
            return -1;
        }

        if (!dir_struct.is_directory)
        {
            cout << "File " << splitted_file_name[i] << " is not a directory!" << endl;
            return -1;
        }

        curr_inode_number = dir_struct.inode_number;
    };

    string extracted_file_name_corresponding = splitted_file_name.back();

    int status = get_file_info(curr_inode_number, extracted_file_name_corresponding, &dir_struct, ex2_file_system);

    status = ex2_file_system->increment_hard_link_count(dir_struct.inode_number);

    if (status == -1)
    {
        cout << "An error has occured while incrementing the hard link count" << endl;
        return -1;
    }

    return 1;
};

int ShellManager::create_soft_link(string &file_name, string &corresponding_file_name)
{
    vector<string> splitted_file_name;

    split_file_name(file_name, splitted_file_name);

    int curr_inode_number = curr_directory_inode_number;

    DirectoryStructure dir_struct;

    for(int i = 0; i < splitted_file_name.size() - 1; i++) {
        int status = get_file_info(curr_inode_number, splitted_file_name[i], &dir_struct, ex2_file_system);
        
        if(status == -1) {
            cout << "File " << splitted_file_name[i] << " is not found!" << endl;
            return -1;
        }

        if(!dir_struct.is_directory) {
            cout << "File " << splitted_file_name[i] << " is not a directory!" << endl;
            return -1;
        }

        curr_inode_number = dir_struct.inode_number;
    };

    string new_file_name = splitted_file_name.back();

    if(get_file_info(curr_inode_number, file_name, &dir_struct, ex2_file_system) == 0) {
        cout << "File with the name " << splitted_file_name.back() << " already exists" << endl;
        return -1;
    }

    split_file_name(corresponding_file_name, splitted_file_name);

    int curr_corresponding_inode_number = curr_directory_inode_number;

    for(int i = 0; i < splitted_file_name.size(); i++) {
        int status = get_file_info(curr_inode_number, splitted_file_name[i], &dir_struct, ex2_file_system);

        if(status == -1) {
            cout << "File " << splitted_file_name[i] << " is not found!" << endl;
            return -1;
        }

        if((i != splitted_file_name.size() - 1) && !dir_struct.is_directory) {
            cout << "File " << splitted_file_name[i] << " is not a directory" << endl;
            return -1;
        }

        curr_corresponding_inode_number = dir_struct.inode_number;
    }

    DirectoryStructure new_struct(dir_struct);

    memcpy(new_struct.file_name, new_file_name.c_str(), new_file_name.size() + 1);

    int file_descriptor = ex2_file_system->my_open(curr_inode_number, WRITE_BIT);

    char buffer[sizeof(DirectoryStructure)];

    new_struct.write_char_bytes(buffer);

    ex2_file_system->my_file_system_write(file_descriptor, buffer, sizeof(DirectoryStructure));

    ex2_file_system->my_close(file_descriptor);

    return 0;
};

void ShellManager::print_curr_directory_name()
{
    cout << curr_directory_name << endl;
}
