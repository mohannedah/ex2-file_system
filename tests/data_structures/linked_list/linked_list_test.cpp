#include <gtest/gtest.h>
#include <bits/stdc++.h>
#include <linked_list.h>

using namespace std;


int get_random_number(int left, int right) {
    int rand_number = rand();
    int length = right - left + 1;
    return (rand_number % length) + left;
};

TEST(linked_list_test1, inserting_random_nodes_test) {
    vector<int> curr_random_numbers;
    LinkedList<int> curr_list;
    for(int i = 0; i < 1e5; i++) {
        int random_number = get_random_number(0, 1e5);
        curr_random_numbers.push_back(random_number);
        curr_list.insert_tail(new ListNode<int>(random_number));
    };

    int curr_idx = 0;

    for(auto node : curr_list) {
        int curr_random_number = node.data;
        ASSERT_EQ(curr_random_number, curr_random_numbers[curr_idx]);
        curr_idx += 1;
    }
};


TEST(linked_list_test_2, unlink_after_insertion) {
    LinkedList<int> linked_list;

    ListNode<int>* list_node = new ListNode<int>(2);

    linked_list.insert_tail(list_node);
    
    linked_list.unlink(list_node);

    ASSERT_EQ(list_node->prev, nullptr);
    ASSERT_EQ(list_node->next, nullptr);

    for(auto node : linked_list) {
        ASSERT_NE(node.data, list_node->data);
    }
}
