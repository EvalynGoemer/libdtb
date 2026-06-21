#include "llist.h"

static void libdtb_llist_initial_node(libdtb_llist_t* llist, libdtb_llist_node_t* node) {
    node->next = NULL;
    node->prev = NULL;
    llist->head = node;
    llist->tail = node;
}

void libdtb_llist_push(libdtb_llist_t* llist, libdtb_llist_node_t* node) {
    libdtb_llist_push_front(llist, node);
}

libdtb_llist_node_t* libdtb_llist_pop(libdtb_llist_t* llist) {
    return libdtb_llist_pop_front(llist);
}

void libdtb_llist_push_front(libdtb_llist_t* llist, libdtb_llist_node_t* node) {
    if (llist->head == NULL) {
        libdtb_llist_initial_node(llist, node);
        return;
    }

    libdtb_llist_node_prepend(llist, llist->head, node);
}

void libdtb_llist_push_back(libdtb_llist_t* llist, libdtb_llist_node_t* node) {
    if (llist->tail == NULL) {
        libdtb_llist_initial_node(llist, node);
        return;
    }

    libdtb_llist_node_append(llist, llist->tail, node);
}


libdtb_llist_node_t* libdtb_llist_pop_front(libdtb_llist_t* llist) {
    libdtb_llist_node_t* node = llist->head;
    if (node != NULL) libdtb_llist_node_delete(llist, node);
    return node;
}

libdtb_llist_node_t* libdtb_llist_pop_back(libdtb_llist_t* llist) {
    libdtb_llist_node_t* node = llist->tail;
    if (node != NULL) libdtb_llist_node_delete(llist, node);
    return node;
}

void libdtb_llist_node_append(libdtb_llist_t* llist, libdtb_llist_node_t* pos, libdtb_llist_node_t* node) {
    node->next = pos->next;
    node->prev = pos;

    pos->next = node;
    if (node->next != NULL) node->next->prev = node;

    if (llist->tail == pos) llist->tail = node;
}

void libdtb_llist_node_prepend(libdtb_llist_t* llist, libdtb_llist_node_t* pos, libdtb_llist_node_t* node) {
    node->next = pos;
    node->prev = pos->prev;

    pos->prev = node;
    if (node->prev != NULL) node->prev->next = node;

    if (llist->head == pos) llist->head = node;
}

void libdtb_llist_node_delete(libdtb_llist_t* llist, libdtb_llist_node_t* node) {
    if (llist->head == node) llist->head = node->next;
    if (llist->tail == node) llist->tail = node->prev;

    if (node->prev != NULL) node->prev->next = node->next;
    if (node->next != NULL) node->next->prev = node->prev;
}
