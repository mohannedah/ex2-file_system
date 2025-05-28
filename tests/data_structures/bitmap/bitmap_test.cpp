#include <gtest/gtest.h>
#include <bitmap.h>
#include "../../../shared_types.h"

class BitMapTest : public testing::Test {
    protected:
        BitMap<NUM_INODES_PER_GROUP> *bitmap;
        BitMapTest() {
            this->bitmap = new BitMap<NUM_INODES_PER_GROUP>();
        };

        ~BitMapTest() {
            delete bitmap;
        };
};


int get_random_number(int left, int right) {
    int rand_number = rand();
    int length = right - left + 1;
    return left + (rand_number % length);
};


pair<int, int> get_random_range(int left, int right) {
    left = get_random_number(left, right);

    right = get_random_number(left, right);

    return {left, right};
}

TEST_F(BitMapTest, correct_initialization_of_bitmap) {
    ASSERT_EQ(this->bitmap->size, 255);

    ASSERT_EQ(this->bitmap->block_size, 31);

    for(int i = 0; i < this->bitmap->size; i++) {
        ASSERT_EQ(this->bitmap->blocks[i], 0);
    };

};


TEST_F(BitMapTest, check_bondaries_test) {
    for(int i = 0; i < NUM_INODES_PER_GROUP; i++) {
        ASSERT_EQ(this->bitmap->check_boundaries(i), true) << "Failed on " << i << endl;
    };

    ASSERT_EQ(this->bitmap->check_boundaries(8064), false);

    ASSERT_EQ(this->bitmap->check_boundaries(-1), false);
};


TEST_F(BitMapTest, testing_set_bit_valid_index) {
    for(int i = 0; i < 1e5; i++) {
        int random_bit = get_random_number(0, NUM_INODES_PER_GROUP - 1);

        int status = this->bitmap->set_bit(random_bit);

        ASSERT_EQ(status, 0);

        ASSERT_NE(this->bitmap->is_set(random_bit), 0);

        status = this->bitmap->unset_bit(random_bit);

        ASSERT_EQ(status, 0);

        ASSERT_EQ(this->bitmap->is_set(random_bit) == 0, 1);
    };
};


TEST_F(BitMapTest, testing_set_bit_invalid_index) {
    for(int i = 0; i < 1e5; i++) {
        int random_bit = get_random_number(0, NUM_INODES_PER_GROUP - 1) + NUM_INODES_PER_GROUP;

        int status = this->bitmap->set_bit(random_bit);

        ASSERT_EQ(status, -1);

        random_bit *= -1;

        status = this->bitmap->unset_bit(random_bit);

        ASSERT_EQ(status, -1);
    };
};

TEST_F(BitMapTest, testing_least_significant_valid_range) {
    for(int i = 0; i < 1e5; i++) {
        int left = get_random_number(0, NUM_INODES_PER_GROUP - 1);
        
        int right = get_random_number(left, NUM_INODES_PER_GROUP - 1);

        int ins = this->bitmap->get_least_significant_bit(left, right);
        
        ASSERT_EQ(ins, left);

        for(int i = left; i <= right; i++) {
            int status = this->bitmap->set_bit(i);

            ASSERT_EQ(status, 0);
        };

        ins = this->bitmap->get_least_significant_bit(left, right);

        ASSERT_EQ(ins, -1);

        for(int i = left; i <= right; i++) {
            int status = this->bitmap->unset_bit(i);

            ASSERT_EQ(status, 0);
        }
    };
};

TEST_F(BitMapTest, testing_least_significant_invalid_range_flipped) {
    for(int i = 0; i < 1e5; i++) {
        pair<int, int> p = get_random_range(0, NUM_INODES_PER_GROUP - 1);

        int left = p.second, right = p.first;
        
        if(left <= right) continue;

        int status = bitmap->get_least_significant_bit(left, right);

        ASSERT_EQ(status, -1);
    };
};

TEST_F(BitMapTest, testing_least_significant_invalid_range_negative) {
    for(int i = 0; i < 1e5; i++) {
        pair <int, int> p = get_random_range(0, NUM_INODES_PER_GROUP - 1);

        int left = -p.first, right = p.second;

        if(left == 0) continue;

        int status = bitmap->get_least_significant_bit(left, right);

        ASSERT_EQ(status, -1);
    }
};
