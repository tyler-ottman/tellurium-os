#include <libc/doubly_linked_list.h>
#include <libc/kmalloc.h>
#include <stddef.h>

bool dll_is_empty(doubly_linked_list_t *dll) {
    return dll->head == NULL;
}

void push_front(doubly_linked_list_t *dll, void *data) {
    node_t *new_node = (node_t *)kmalloc(sizeof(node_t));
    new_node->next = NULL;
    new_node->prev = NULL;
    new_node->data = data;

    if (dll_is_empty(dll)) {
        dll->tail = new_node;
    } else {
        dll->head->prev = new_node;
    }

    new_node->next = dll->head;
    dll->head = new_node;
}

void push_back(doubly_linked_list_t *dll, void *data) {
    node_t *new_node = (node_t *)kmalloc(sizeof(node_t));
    new_node->next = NULL;
    new_node->prev = NULL;
    new_node->data = data;

    if (dll_is_empty(dll)) {
        dll->head = new_node;
    } else {
        dll->tail->next = new_node;
        new_node->prev = dll->tail;
    }

    dll->tail = new_node;
}

void *pop_front(doubly_linked_list_t *dll) {
    if (dll_is_empty(dll)) {
        return NULL;
    }

    node_t *temp_node = dll->head;

    if (dll->head->next == NULL) { // 1 node
        dll->tail = NULL;
    } else {
        dll->head->next->prev = NULL;
    }

    dll->head = dll->head->next;

    void *ret_data = temp_node->data;
    kfree(temp_node);
    return ret_data;
}

void *pop_back(doubly_linked_list_t *dll) {
    if (dll_is_empty(dll)) {
        return NULL;
    }

    node_t *temp_node = dll->tail;

    if (dll->head->next == NULL) { // 1 node
        dll->head = NULL;
    } else {
        dll->tail->prev->next = NULL;
    }

    dll->tail = dll->tail->prev;

    void *ret_data = temp_node->data;
    kfree(temp_node);
    return ret_data;
}