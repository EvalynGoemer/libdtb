#include "libdtb.h"

#include <stdbool.h>
#include <stdint.h>

#define LIBDTB_ROOT_NONE 0
#define LIBDTB_ROOT_OPEN 1
#define LIBDTB_ROOT_DONE 2

#define LIBDTB_PHANDLE_BUCKETS 512

#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
_Static_assert((LIBDTB_PHANDLE_BUCKETS * sizeof(libdtb_llist_t*)) <= 4096, "the phandle hash map must be less than or equal to 4096 bytes");
_Static_assert((LIBDTB_PHANDLE_BUCKETS != 0) && ((LIBDTB_PHANDLE_BUCKETS & (LIBDTB_PHANDLE_BUCKETS - 1)) == 0), "LIBDTB_PHANDLE_BUCKETS must be a power of 2");
#endif

static inline uint32_t libdtb_hash32(uint32_t v) {
    v ^= v >> 16;
    v *= 0x85ebca6b;
    v ^= v >> 13;
    v *= 0xc2b2ae35;
    v ^= v >> 16;
    return v;
}

devicetree_t* libdtb_create(const void* dtb, size_t size) {
    if (!dtb) return NULL;

    const dtb_header_t* header = (const dtb_header_t*)dtb;
    if (!libdtb0_validate_header(header, size)) return NULL;

    devicetree_t* tree = (devicetree_t*)LIBDTB_MALLOC(sizeof(devicetree_t));
    if (!tree) return NULL;
    tree->header = header;
    tree->root = NULL;
    tree->phandle_hmap = NULL;

    tree->phandle_hmap = (libdtb_llist_t**)LIBDTB_MALLOC_HMAP(LIBDTB_PHANDLE_BUCKETS * sizeof(libdtb_llist_t*));
    if (!tree->phandle_hmap) goto fail;

    for (int i = 0; i < LIBDTB_PHANDLE_BUCKETS; i++) tree->phandle_hmap[i] = NULL;

    dt_node_t* current = NULL;
    uint64_t cursor = 0;
    int root_state = LIBDTB_ROOT_NONE;

    const char* node_string;
    const char* prop_string;
    const dtb_prop_t* raw_prop;

    while (true) {
        dtb_token_type_t type = libdtb0_get_next_token(header, &node_string, &prop_string, &raw_prop, &cursor);

        if (type == DTB_TOKEN_INVALID) goto fail;

        if (type == DTB_TOKEN_NOP) continue;

        if (type == DTB_TOKEN_END) {
            if (root_state != LIBDTB_ROOT_DONE) goto fail;
            break;
        }

        if (type == DTB_TOKEN_BEGIN_NODE) {
            if (root_state == LIBDTB_ROOT_DONE) goto fail;
            if (!current && root_state == LIBDTB_ROOT_OPEN) goto fail;

            dt_node_t* n = (dt_node_t*)LIBDTB_MALLOC(sizeof(dt_node_t));
            if (!n) goto fail;
            n->name = node_string;
            n->parent = current;
            n->props.head = NULL;
            n->props.tail = NULL;
            n->children.head = NULL;
            n->children.tail = NULL;
            n->llnode.next = NULL;
            n->llnode.prev = NULL;

            if (current) {
                libdtb_llist_push_back(&current->children, &n->llnode);
            } else {
                tree->root = n;
                root_state = LIBDTB_ROOT_OPEN;
            }
            current = n;
        }

        if (type == DTB_TOKEN_PROP) {
            if (!current) goto fail;

            dt_prop_t* p = (dt_prop_t*)LIBDTB_MALLOC(sizeof(dt_prop_t));
            if (!p) goto fail;

            p->name = prop_string;
            p->raw = raw_prop;
            p->llnode.next = NULL;
            p->llnode.prev = NULL;
            libdtb_llist_push_back(&current->props, &p->llnode);

            if (LIBDTB_STRCMP(p->name, "phandle") == 0 || LIBDTB_STRCMP(p->name, "linux,phandle") == 0) {
                uint32_t v;
                if (libdtb0_prop_read_cell(raw_prop, 0, &v)) {
                    dt_phandle_t* ph = (dt_phandle_t*)LIBDTB_MALLOC(sizeof(dt_phandle_t));
                    if (!ph) goto fail;

                    ph->phandle = v;
                    ph->node = current;
                    ph->llnode.next = NULL;
                    ph->llnode.prev = NULL;

                    uint32_t idx = libdtb_hash32(v) & (LIBDTB_PHANDLE_BUCKETS - 1);
                    if (!tree->phandle_hmap[idx]) {
                        tree->phandle_hmap[idx] = (libdtb_llist_t*)LIBDTB_MALLOC(sizeof(libdtb_llist_t));
                        if (!tree->phandle_hmap[idx]) {
                            LIBDTB_FREE(ph, sizeof(dt_phandle_t));
                            goto fail;
                        }
                        tree->phandle_hmap[idx]->head = NULL;
                        tree->phandle_hmap[idx]->tail = NULL;
                    }
                    libdtb_llist_push_back(tree->phandle_hmap[idx], &ph->llnode);
                }
            }
        }

        if (type == DTB_TOKEN_END_NODE) {
            if (!current) goto fail;
            current = current->parent;
            if (!current) root_state = LIBDTB_ROOT_DONE;
        }
    }

    if (root_state != LIBDTB_ROOT_DONE) goto fail;

    return tree;

fail:
    libdtb_destroy(tree);
    return NULL;
}

void libdtb_destroy(devicetree_t* tree) {
    if (!tree) return;

    dt_node_t* node = tree->root;
    while (node) {
        if (node->children.head) {
            node = LIBDTB_CONTAINER_OF(node->children.head, dt_node_t, llnode);
            continue;
        }

        libdtb_llist_node_t* pn;
        while ((pn = libdtb_llist_pop_front(&node->props)) != NULL) {
            dt_prop_t* p = LIBDTB_CONTAINER_OF(pn, dt_prop_t, llnode);
            LIBDTB_FREE(p, sizeof(dt_prop_t));
        }

        libdtb_llist_node_t* next_lln = node->llnode.next;
        dt_node_t* parent = node->parent;

        if (parent) { libdtb_llist_node_delete(&parent->children, &node->llnode); }

        LIBDTB_FREE(node, sizeof(dt_node_t));

        if (next_lln) {
            node = LIBDTB_CONTAINER_OF(next_lln, dt_node_t, llnode);
        } else if (parent) {
            node = parent;
        } else {
            node = NULL;
        }
    }

    if (tree->phandle_hmap) {
        uint32_t i;
        for (i = 0; i < LIBDTB_PHANDLE_BUCKETS; i++) {
            libdtb_llist_t* bucket = tree->phandle_hmap[i];
            if (!bucket) continue;
            libdtb_llist_node_t* ln;
            while ((ln = libdtb_llist_pop_front(bucket)) != NULL) {
                dt_phandle_t* ph = LIBDTB_CONTAINER_OF(ln, dt_phandle_t, llnode);
                LIBDTB_FREE(ph, sizeof(dt_phandle_t));
            }
            LIBDTB_FREE(bucket, sizeof(libdtb_llist_t));
        }
        LIBDTB_FREE_HMAP(tree->phandle_hmap, LIBDTB_PHANDLE_BUCKETS * sizeof(libdtb_llist_t*));
    }

    LIBDTB_FREE(tree, sizeof(devicetree_t));
}

dt_node_t* libdtb_find_child(dt_node_t* node, const char* name) {
    if (!node || !name) return NULL;
    libdtb_llist_node_t* n;
    for (n = node->children.head; n; n = n->next) {
        dt_node_t* c = LIBDTB_CONTAINER_OF(n, dt_node_t, llnode);
        if (LIBDTB_STRCMP(c->name, name) == 0) return c;
    }
    return NULL;
}

dt_node_t* libdtb_find_path(devicetree_t* tree, const char* path) {
    if (!tree || !path || path[0] != '/') return NULL;
    dt_node_t* cur = tree->root;
    const char* p = path + 1;
    if (*p == '\0') return cur;

    while (*p && cur) {
        const char* seg = p;
        while (*p && *p != '/') p++;
        size_t len = (size_t)(p - seg);
        if (len == 0) return NULL;

        dt_node_t* found = NULL;
        libdtb_llist_node_t* n;
        for (n = cur->children.head; n; n = n->next) {
            dt_node_t* c = LIBDTB_CONTAINER_OF(n, dt_node_t, llnode);
            if (LIBDTB_STRLEN(c->name) == len && LIBDTB_STRNCMP(c->name, seg, len) == 0) {
                found = c;
                break;
            }
        }

        if (!found) return NULL;
        cur = found;

        if (*p == '/')
            p++;
        else if (*p)
            return NULL;
        if (*p == '\0') break;
    }
    return cur;
}

const dtb_prop_t* libdtb_find_prop(dt_node_t* node, const char* name) {
    if (!node || !name) return NULL;
    libdtb_llist_node_t* n;
    for (n = node->props.head; n; n = n->next) {
        dt_prop_t* p = LIBDTB_CONTAINER_OF(n, dt_prop_t, llnode);
        if (LIBDTB_STRCMP(p->name, name) == 0) return p->raw;
    }
    return NULL;
}

bool libdtb_phandle_from_prop(const dtb_prop_t* prop, uint32_t* out) {
    if (!prop || !out) return false;
    return libdtb0_prop_read_cell(prop, 0, out);
}

dt_node_t* libdtb_resolve_phandle(devicetree_t* tree, uint32_t phandle) {
    if (!tree || !tree->phandle_hmap) return NULL;
    uint32_t idx = libdtb_hash32(phandle) & (LIBDTB_PHANDLE_BUCKETS - 1);
    libdtb_llist_t* bucket = tree->phandle_hmap[idx];
    if (!bucket) return NULL;
    libdtb_llist_node_t* n;
    for (n = bucket->head; n; n = n->next) {
        dt_phandle_t* p = LIBDTB_CONTAINER_OF(n, dt_phandle_t, llnode);
        if (p->phandle == phandle) return p->node;
    }
    return NULL;
}

dt_node_t* libdtb_resolve_phandle_prop(devicetree_t* tree, const dtb_prop_t* prop) {
    uint32_t phandle;
    if (!libdtb_phandle_from_prop(prop, &phandle)) return NULL;
    return libdtb_resolve_phandle(tree, phandle);
}
