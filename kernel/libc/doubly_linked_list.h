#ifndef DOUBLY_LINKED_LIST_H
#define DOUBLY_LINKED_LIST_H

#include <stdbool.h>

#define DLL_DECLARE(dll) doubly_linked_list_t dll
#define DLL_PUSH_FRONT(dll, node) push_front(&dll, (void *)node)
#define DLL_PUSH_BACK(dll, node) push_back(&dll, (void *)node)
#define DLL_POP_FRONT(dll) pop_front(&dll)
#define DLL_POP_BACK(dll) pop_back(&dll)

typedef struct node {
    void *data;
    struct node *next;
    struct node *prev;
} node_t;

typedef struct doubly_linked_list {
    struct node *head;
    struct node *tail;
} doubly_linked_list_t;

bool dll_is_empty(doubly_linked_list_t *dll);
void push_front(doubly_linked_list_t *dll, void *data);
void push_back(doubly_linked_list_t *dll, void *data);
void *pop_front(doubly_linked_list_t *dll);
void *pop_back(doubly_linked_list_t *dll);

#endif // DOUBLY_LINKED_LIST_H