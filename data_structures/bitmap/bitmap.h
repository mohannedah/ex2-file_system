#ifndef BITMAP_H
#define BITMAP_H

#include <bits/stdc++.h>

using namespace std;

template <int T>
struct BitMap
{
public:
    bool check_boundaries(int index);

    int *blocks, size, block_size;

    BitMap();

    ~BitMap();

    BitMap(int *size, int *block_size, int *blocks);

    int get_least_significant_bit(int start_range, int end_range);

    int set_bit(int index);

    int unset_bit(int index);

    int is_set(int index);

    int flip(int index);

    bool memory_initialized = 0;
};

#endif
