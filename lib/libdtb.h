#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "libdtb0.h"
#include "libdtb_kernel_api.h"
#include "llist.h"

#define LIBDTB_CONTAINER_OF(PTR, TYPE, MEMBER) ((TYPE*)((uintptr_t)(PTR) - __builtin_offsetof(TYPE, MEMBER)))

typedef struct dt_prop dt_prop_t;
typedef struct dt_phandle dt_phandle_t;
typedef struct dt_node dt_node_t;

typedef struct devicetree {
    const dtb_header_t* header;
    dt_node_t* root;
    libdtb_llist_t** phandle_hmap;
} devicetree_t;

struct dt_node {
    const char* name;
    dt_node_t* parent;
    libdtb_llist_t props;
    libdtb_llist_t children;
    libdtb_llist_node_t llnode;
};

struct dt_prop {
    const char* name;
    const dtb_prop_t* raw;
    libdtb_llist_node_t llnode;
};

struct dt_phandle {
    uint32_t phandle;
    dt_node_t* node;
    libdtb_llist_node_t llnode;
};

extern devicetree_t* libdtb_create(const void* dtb, size_t size);
extern void libdtb_destroy(devicetree_t* tree);

extern dt_node_t* libdtb_find_path(devicetree_t* tree, const char* path);
extern dt_node_t* libdtb_find_child(dt_node_t* node, const char* name);

extern const dtb_prop_t* libdtb_find_prop(dt_node_t* node, const char* name);

extern bool libdtb_phandle_from_prop(const dtb_prop_t* prop, uint32_t* out);
extern dt_node_t* libdtb_resolve_phandle(devicetree_t* tree, uint32_t phandle);
extern dt_node_t* libdtb_resolve_phandle_prop(devicetree_t* tree, const dtb_prop_t* prop);
