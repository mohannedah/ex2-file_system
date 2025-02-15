#ifndef DISK_H
#define DISK_H
#include <bits/stdc++.h>
#include <stdio.h>
#include <iostream>
#include <fcntl.h>
#include <sys/uio.h>
#include "../shared_types.h"

using namespace std;

class Disk
{
public:
    Disk();
    ~Disk();
    int file_descriptor = -1;
    int populate_sector(int sector_number, Sector *sector_info);
    int write_sector(int sector_number, Sector *sector);
};

#endif