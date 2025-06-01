#include "linked_list.h"
using namespace std;

template <typename T>
LinkedList<T>::LinkedList()
{
    this->head = new ListNode<T>(T{});
    this->tail = new ListNode<T>(T{});

    this->head->next = this->tail;
    this->tail->prev = this->head;
}

template <typename T>
LinkedList<T>::~LinkedList()
{
    ListNode<T> *curr_node = this->head;

    ListNode<T> *next_node;
    while (curr_node != nullptr)
    {
        next_node = curr_node->next;
        
        free(curr_node);

        curr_node = next_node;
    }
};

template <typename T>
ListNode<T> *LinkedList<T>::get_head()
{
    if (this->head->next == this->tail)
    {
        return nullptr;
    }

    return this->head->next;
};

template <typename T>
ListNode<T> *LinkedList<T>::get_tail()
{
    if (this->tail->prev == this->head)
    {
        return nullptr;
    }

    return this->tail->prev;
};

template <typename T>
int LinkedList<T>::size()
{
    return this->curr_size;
}

template <typename T>
int LinkedList<T>::unlink(ListNode<T> *node)
{
    if (node == this->head || node == this->tail)
        return -1;

    ListNode<T> *prev_node = node->prev, *next_node = node->next;

    if (next_node != nullptr)
        next_node->prev = prev_node;

    if (prev_node != nullptr)
        prev_node->next = next_node;

    node->prev = nullptr, node->next = nullptr;

    return 1;
};

template <typename T>
int LinkedList<T>::insert_helper(ListNode<T> *prev_node, ListNode<T> *curr_node, ListNode<T> *next_node)
{
    if (prev_node == nullptr || curr_node == nullptr || next_node == nullptr)
        return -1;

    prev_node->next = curr_node;
    curr_node->prev = prev_node;
    curr_node->next = next_node;
    next_node->prev = curr_node;

    return 0;
};

template <typename T>
int LinkedList<T>::insert_head(ListNode<T> *node)
{
    if (node == this->head || node == this->tail)
        return -1;

    int status = unlink(node);

    if (status == -1)
        return -1;

    return insert_helper(this->head, node, this->head->next);
};

template <typename T>
int LinkedList<T>::insert_tail(ListNode<T> *node)
{
    if (node == this->head || node == this->tail)
        return -1;

    int status = unlink(node);

    if (status == -1)
        return -1;

    return insert_helper(this->tail->prev, node, this->tail);
};

template <typename T>
typename LinkedList<T>::Iterator LinkedList<T>::begin()
{
    auto node = this->get_head();

    if(node == nullptr) return LinkedList<T>::Iterator(this->tail);

    return LinkedList<T>::Iterator(node);
};

template <typename T>
typename LinkedList<T>::Iterator LinkedList<T>::end()
{
    return LinkedList<T>::Iterator(this->tail);
};

template <typename T>
typename LinkedList<T>::Iterator &LinkedList<T>::Iterator::operator++()
{
    if (curr_node)
        this->curr_node = curr_node->next;
    return *this;
};

template <typename T>
typename LinkedList<T>::Iterator LinkedList<T>::Iterator::operator++(int)
{

    auto temp = *this;

    if (curr_node)
        this->curr_node = curr_node->next;

    return temp;
};

template <typename T>
bool LinkedList<T>::Iterator::operator!=(const Iterator &other_iterator)
{
    return this->curr_node != other_iterator.curr_node;
}

template <typename T>
bool LinkedList<T>::Iterator::operator==(const Iterator &other_iterator) { return this->curr_node == other_iterator.curr_node; }
