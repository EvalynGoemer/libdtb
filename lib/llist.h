#pragma once

#include <stddef.h>

typedef struct libdtb_llist_node libdtb_llist_node_t;
typedef struct libdtb_llist libdtb_llist_t;

struct libdtb_llist {
    libdtb_llist_node_t* head;
    libdtb_llist_node_t* tail;
};

struct libdtb_llist_node {
    libdtb_llist_node_t* next;
    libdtb_llist_node_t* prev;
};

void libdtb_llist_push(libdtb_llist_t* list, libdtb_llist_node_t* node);
libdtb_llist_node_t* libdtb_llist_pop(libdtb_llist_t* list);

void libdtb_llist_push_front(libdtb_llist_t* list, libdtb_llist_node_t* node);
void libdtb_llist_push_back(libdtb_llist_t* list, libdtb_llist_node_t* node);

libdtb_llist_node_t* libdtb_llist_pop_front(libdtb_llist_t* list);
libdtb_llist_node_t* libdtb_llist_pop_back(libdtb_llist_t* list);

void libdtb_llist_node_append(libdtb_llist_t* list, libdtb_llist_node_t* pos, libdtb_llist_node_t* node);
void libdtb_llist_node_prepend(libdtb_llist_t* list, libdtb_llist_node_t* pos, libdtb_llist_node_t* node);
void libdtb_llist_node_delete(libdtb_llist_t* list, libdtb_llist_node_t* node);
