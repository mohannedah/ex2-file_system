#include "bitmap.h"

int extract_least_significant(int num, int start, int end)
{
    for (int i = start; i < end; i++)
    {
        if(num & (1 << i)) continue;
        
        return i;
    }

    return -1;
};

template <int T>
BitMap<T>::BitMap()
{
    this->block_size = (sizeof(int) * 8 - 1);
    this->size = ceil((float)T / this->block_size);

    this->blocks = new int[this->size];
    for(int i = 0; i < size; i++) {
        this->blocks[i] = 0;
    }
}

template <int T>
BitMap<T>::~BitMap(){
    delete[] this->blocks;
};

template <int T>
BitMap<T>::BitMap(int *block_size, int *size, int *my_blocks)
{

    this->size = *size;
    this->block_size = *block_size;
    this->blocks = new int[this->size];
    for (int i = 0; i < this->size; i++)
    {
        this->blocks[i] = my_blocks[i];
    };
};

template <int T>
int BitMap<T>::get_least_significant_bit(int start_range, int end_range)
{
    if(start_range > end_range) return -1;

    if (!(check_boundaries(start_range) && check_boundaries(end_range)))
    {
        return -1;
    }

    int starting_block = (start_range / this->block_size), ending_block = end_range / this->block_size;

    int bound_end = this->block_size, bound_start = start_range % this->block_size;

    if(starting_block == ending_block) {
        bound_end = (end_range % this->block_size) + 1;
    }
    

    int desired_bit = extract_least_significant(this->blocks[starting_block], bound_start, bound_end);

    if (desired_bit != -1) {
        int res = ((starting_block * this->block_size) + desired_bit);        
        return ((starting_block * this->block_size) + desired_bit);
    }
        

    for (int block = starting_block + 1; block < ending_block; block++)
    {
        desired_bit = extract_least_significant(this->blocks[block], 0, this->block_size);

        if (desired_bit != -1)
            return ((block * this->block_size) + desired_bit);
    }

    if(starting_block == ending_block) return -1;

    bound_end = (end_range % this->block_size) + 1;

    desired_bit = extract_least_significant(this->blocks[ending_block], 0, bound_end);

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


    if(is_set(index)) this->blocks[block_idx] ^= (1 << (index % this->block_size));

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

bool BitMap<T>::check_boundaries(int index)
{
    if(index < 0) return 0;

    if (index >= T)
        return 0;

    return 1;
};

template class BitMap<7800>;
