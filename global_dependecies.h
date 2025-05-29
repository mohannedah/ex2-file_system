#ifndef GLOBAL_DEPENDENCIES_H
#define GLOBAL_DEPENDENCIES_H
#include "./file_system_manager/ex2_interface/ex2.h"
#include "./disk_simulator_manager/disk.h"

class EX2FILESYSTEM;

class Disk;

extern Disk *disk_manager;

extern EX2FILESYSTEM *ex2_file_system;


#endif
