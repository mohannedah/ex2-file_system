#ifndef LINKED_LIST_HH
#define LINKED_LIST_HH
#include <bits/stdc++.h>
using namespace std;

template <typename T>
struct ListNode
{
    T data;
    ListNode *next, *prev;

    ListNode(T data)
    {
        this->data = data;
        this->next = nullptr;
        this->prev = nullptr;
    }
};

template <typename T>
class LinkedList
{
public:
    LinkedList();

    ~LinkedList();

    ListNode<T> *get_head();

    ListNode<T> *get_tail();

    int unlink(ListNode<T> *node);

    int insert_head(ListNode<T> *data);

    int insert_tail(ListNode<T> *data);

    int size();

    class Iterator
    {
    public:
        ListNode<T> *curr_node;
        explicit Iterator(ListNode<T> *curr_node) : curr_node(curr_node) {};

        ListNode<T> &operator*()
        {
            return *(this->curr_node);
        }

        Iterator &operator++();

        Iterator operator++(int);

        bool operator!=(const Iterator &other_iterator);

        bool operator==(const Iterator &other_iterator);
    };

    Iterator begin();

    Iterator end();

private:
    int curr_size;

    ListNode<T> *head, *tail;

    int insert_helper(ListNode<T> *prev_node, ListNode<T> *curr_node, ListNode<T> *next_node);
};

#include "linked_list.tpp"
#endif