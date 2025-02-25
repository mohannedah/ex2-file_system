#include "bitmap.h"

int extract_least_significant(int num, int start, int end)
{
    for (int i = start; i < end; i++)
    {
        if (!(num & (1 << i)))
            return i;
    }

    return -1;
};

template <int T>
BitMap<T>::BitMap()
{
    this->block_size = sizeof(int) * 8;
    this->size = ceil(T / this->block_size);
    this->blocks = new int[this->size];
}

template <int T>
BitMap<T>::~BitMap()
{
    if (this->memory_initialized)
    {
        delete[] this->blocks;
    }
};

template <int T>
BitMap<T>::BitMap(int *block_size, int *size, int *blocks)
{
    this->size = *size;
    this->block_size = *block_size;
    this->blocks = blocks;
};

template <int T>
int BitMap<T>::get_least_significant_bit(int start_range, int end_range)
{
    if (!(check_boundaries(start_range) && check_boundaries(end_range)))
        return -1;

    int starting_block = (start_range / this->block_size), ending_block = end_range / this->block_size;

    int bound_end = this->block_size * 8, bound_start = start_range % this->block_size;

    int desired_bit = extract_least_significant(blocks[starting_block], bound_start, bound_end);

    if (desired_bit != -1)
        return ((starting_block * this->block_size) + desired_bit);

    for (int block = starting_block + 1; block < ending_block; block++)
    {
        desired_bit = extract_least_significant(blocks[block], 0, this->block_size);

        if (desired_bit != -1)
            return ((block * this->block_size) + desired_bit);
    }

    bound_end = (end_range % this->block_size) + 1;

    desired_bit = extract_least_significant(blocks[ending_block], 0, bound_end);

    if (desired_bit != -1)
        return ((ending_block * this->block_size) + desired_bit);

    return -1;
};

template <int T>
int BitMap<T>::set_bit(int index)
{
    int block_idx = index / this->block_size;

    if (!check_boundaries(index))
        return -1;

    this->blocks[block_idx] |= (1 << (index % this->block_size));

    return 0;
};

template <int T>
int BitMap<T>::unset_bit(int index)
{
    int block_idx = index / this->block_size;

    if (!check_boundaries(index))
        return -1;

    this->blocks[block_idx] &= (~(1 << (index % this->block_size)));

    return 0;
}

template <int T>
int BitMap<T>::is_set(int index)
{
    int block_idx = index / this->block_size;

    if (!check_boundaries(index))
        return -1;

    return this->blocks[block_idx] & (1 << (index % this->block_size));
};

template <int T>
int BitMap<T>::flip(int index)
{
    int block_idx = index / this->block_size;

    if (!check_boundaries(index))
        return -1;

    this->blocks[block_idx] ^= (1 << (index % this->block_size));

    return is_set(index);
};

// --------------------------------------------------Utility Functions-----------------------------------------------------

template <int T>

inline bool BitMap<T>::check_boundaries(int index)
{
    int block = index / this->block_size;

    if (block >= this->size || block < 0)
        return 0;

    return 1;
};

template class BitMap<8128>;
