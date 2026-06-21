// libdtb -- Single Header Build -- Generated With comp2header.pl
// Read the included README.md for how to use this library
// Find the source code at https://github.com/EvalynGoemer/libdtb.git
/*
==== Licenced under 0BSD and dedicated to the public domain where applicable ====
==== 0BSD Licence ====
BSD Zero Clause License
Copyright (c) 2026 Evalyn Goemer
Permission to use, copy, modify, and/or distribute this software for any
purpose with or without fee is hereby granted.
THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
PERFORMANCE OF THIS SOFTWARE.
==== Public Domain ====
This software is released to the public domain.
Anyone and anything may copy, edit, publish, use, compile, sell and
distribute this work and all its parts in any form for any purpose,
commercial and non-commercial, without any restrictions, without complying
with any conditions and by any means.
*/
#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#define LIBDTB_SINGLE_HEADER
#define DTB_SUPPORTED_VERSION 17
#define DTB_MAGIC 0xd00dfeed
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define libdtb0_be2host32(v) __builtin_bswap32((v))
#define libdtb0_be2host64(v) __builtin_bswap64((v))
#else
#define libdtb0_be2host32(v) (v)
#define libdtb0_be2host64(v) (v)
#endif
typedef struct {
    uint32_t magic;
    uint32_t totalsize;
    uint32_t off_dt_struct;
    uint32_t off_dt_strings;
    uint32_t off_mem_rsvmap;
    uint32_t version;
    uint32_t last_comp_version;
    uint32_t boot_cpuid_phys;
    uint32_t size_dt_strings;
    uint32_t size_dt_struct;
} dtb_header_t;
typedef struct {
    uint64_t address;
    uint64_t size;
} dtb_reserved_entry_t;
typedef struct {
    uint32_t len;
    uint32_t nameoff;
    uint8_t data[];
} dtb_prop_t;
typedef struct {
    uint64_t lo;
    uint64_t hi;
} dtb_u128_t;
typedef enum {
    DTB_TOKEN_BEGIN_NODE = 1, // start of a node
    DTB_TOKEN_END_NODE = 2, // end of a node
    DTB_TOKEN_PROP = 3, // property inside of a node
    DTB_TOKEN_NOP = 4, // no operation; skip
    DTB_TOKEN_END = 9, // end of device tree
    DTB_TOKEN_INVALID = -1, // failure to parse tree; do not continue parsing
} dtb_token_type_t;
/**
 * @brief validate a DTB header
 *
 * @param header pointer to the DTB header
 * @param file_size size of the DTB, or 0xFFFFFFFF if size is unknown
 * @retval true if the header is valid
 * @retval false if the header is invalid
 */
extern bool libdtb0_validate_header(const dtb_header_t* header, uint64_t file_size);
/**
 * @brief Get the next token inside the DTB
 *
 * @param header pointer to the DTB header
 * @param[out] node_string     Set to the node name   (DTB_TOKEN_BEGIN_NODE only).
 * @param[out] property_string Set to the prop name   (DTB_TOKEN_PROP only).
 * @param[out] property        Set to the prop itself (DTB_TOKEN_PROP only).
 * @param[in,out] cursor       Internal parse cursor; initialise to 0.
 * @return the encountered token
 * @retval DTB_TOKEN_INVALID if the DTB is malformed or any pointer parameter is NULL.
 */
extern dtb_token_type_t libdtb0_get_next_token(const dtb_header_t* header, const char** node_string, const char** property_string, const dtb_prop_t** property, uint64_t* cursor);
/**
 * @brief get the number of valid reserved memory entries
 *
 * @param header pointer to the DTB header
 * @return number of valid reserved memory entries
 */
extern uint32_t libdtb0_get_reserved_count(const dtb_header_t* header);
/**
 * @brief get a reserved memory entry
 *
 * @param header pointer to the DTB header
 * @param entry zero based entry index
 * @return the entry, {0, 0} if the index would be OOB for the reserved memory area
 */
extern dtb_reserved_entry_t libdtb0_get_reserved_entry(const dtb_header_t* header, unsigned int entry);
/**
 * @brief read a single cell as u32
 *
 * @param prop   property to read
 * @param index  cell index (0-based)
 * @param result receives the decoded value
 * @retval true on success
 * @retval false if the cell is out of bounds
 */
extern bool libdtb0_prop_read_cell(const dtb_prop_t* prop, uint32_t index, uint32_t* result);
/**
 * @brief read up to 2 cells from a property as u64
 *
 * @param prop   property to read
 * @param index  starting cell index (0-based)
 * @param cells  number of cells (0=zero, 1=u32, 2=u64)
 * @param result receives the decoded value
 * @retval true on success
 * @retval false if cells are out of bounds
 */
extern bool libdtb0_prop_read_cells_u64(const dtb_prop_t* prop, uint32_t index, uint8_t cells, uint64_t* result);
/**
 * @brief read up to 4 cells from a property as dtb_u128_t
 *
 * @param prop   property to read
 * @param index  starting cell index (0-based)
 * @param cells  number of cells (0=zero, 1=u32, 2=u64, 3=u96, 4=u128)
 * @param result receives the decoded value
 * @retval true on success
 * @retval false if cells are out of bounds
 */
extern bool libdtb0_prop_read_cells_u128(const dtb_prop_t* prop, uint32_t index, uint8_t cells, dtb_u128_t* result);
/**
 * @brief read a null-terminated string from a property
 *
 * @param prop property to read
 * @retval non-NULL pointer into the DTB
 * @retval NULL if not null-terminated within the property length
 */
extern const char* libdtb0_prop_get_string(const dtb_prop_t* prop);
/**
 * @brief read the Nth null-terminated string from a property
 *
 * @param prop property to read
 * @param index string index (0-based)
 * @retval non-NULL pointer into the DTB
 * @retval NULL if the string is not null-terminated within the property length
 */
extern const char* libdtb0_prop_get_string_at(const dtb_prop_t* prop, uint32_t index);
#ifndef LIBDTB_FREESTANDING
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
#if !defined(LIBDTB_OWN_MALLOC) && (defined(LIBDTB_IMPL) || !defined(LIBDTB_SINGLE_HEADER))
#include <stdlib.h>
#define LIBDTB_MALLOC(sz) malloc((sz))
#define LIBDTB_FREE(p, sz) free((p))
#define LIBDTB_MALLOC_HMAP(sz) malloc((sz))
#define LIBDTB_FREE_HMAP(p, sz) free((p))
#endif
#if !defined(LIBDTB_OWN_STRING) && (defined(LIBDTB_IMPL) || !defined(LIBDTB_SINGLE_HEADER))
#include <string.h>
#define LIBDTB_STRLEN(s) strlen((s))
#define LIBDTB_STRCMP(a, b) strcmp((a), (b))
#define LIBDTB_STRNCMP(a, b, n) strncmp((a), (b), (n))
#endif
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
#endif
#ifdef LIBDTB_IMPL
#define LIBDTB0_ALIGN_UP(x, align) ((((uint64_t)(x)) + ((align) - 1)) & ~((uint64_t)((align) - 1)))
#define LIBDTB0_IS_ALIGNED(x, align) (((uint64_t)(x) & ((align) - 1)) == 0)
bool libdtb0_validate_header(const dtb_header_t* header, uint64_t file_size) {
    if (file_size < sizeof(dtb_header_t)) return false;
    if (!LIBDTB0_IS_ALIGNED(header, 4)) return false;
    uint32_t magic = libdtb0_be2host32(header->magic);
    uint32_t version = libdtb0_be2host32(header->version);
    uint32_t lcversion = libdtb0_be2host32(header->last_comp_version);
    uint32_t total_size = libdtb0_be2host32(header->totalsize);
    uint32_t struct_offset = libdtb0_be2host32(header->off_dt_struct);
    uint32_t string_offset = libdtb0_be2host32(header->off_dt_strings);
    uint32_t struct_size = libdtb0_be2host32(header->size_dt_struct);
    uint32_t string_size = libdtb0_be2host32(header->size_dt_strings);
    uint32_t reserved_offset = libdtb0_be2host32(header->off_mem_rsvmap);
    if (magic != DTB_MAGIC) return false;
    if (version > DTB_SUPPORTED_VERSION || version < lcversion) return false;
    if (total_size == 0 || total_size > file_size) return false;
    if (struct_offset >= total_size || string_offset >= total_size || reserved_offset >= total_size) return false;
    if (struct_size == 0 || string_size == 0) return false;
    if (!LIBDTB0_IS_ALIGNED(struct_size, 4)) return false;
    if ((uint64_t)struct_offset + struct_size > total_size) return false;
    if ((uint64_t)string_offset + string_size > total_size) return false;
    if (!LIBDTB0_IS_ALIGNED(struct_offset, 4)) return false;
    if (!LIBDTB0_IS_ALIGNED(string_offset, 4)) return false;
    if (!LIBDTB0_IS_ALIGNED(reserved_offset, 8)) return false;
    if (reserved_offset >= struct_offset) return false;
    if (string_offset <= struct_offset) return false;
    if ((uint64_t)struct_offset + struct_size > (uint64_t)string_offset) return false;
    return true;
}
dtb_reserved_entry_t libdtb0_get_reserved_entry(const dtb_header_t* header, unsigned int entry_num) {
    if (!header) return (dtb_reserved_entry_t){ 0, 0 };
    uint64_t offset = libdtb0_be2host32(header->off_mem_rsvmap) + (uint64_t)entry_num * sizeof(dtb_reserved_entry_t);
    if (offset + sizeof(dtb_reserved_entry_t) > libdtb0_be2host32(header->off_dt_struct)) return (dtb_reserved_entry_t){ 0, 0 };
    dtb_reserved_entry_t* entry = (dtb_reserved_entry_t*)((uintptr_t)header + offset);
    uint64_t addr = libdtb0_be2host64(entry->address);
    uint64_t size = libdtb0_be2host64(entry->size);
    return (dtb_reserved_entry_t){ .address = addr, .size = size };
}
uint32_t libdtb0_get_reserved_count(const dtb_header_t* header) {
    if (!header) return 0;
    uint32_t rsv_off = libdtb0_be2host32(header->off_mem_rsvmap);
    uint32_t struct_off = libdtb0_be2host32(header->off_dt_struct);
    if (struct_off <= rsv_off) return 0;
    uint32_t cap = (struct_off - rsv_off) / sizeof(dtb_reserved_entry_t);
    if (cap == 0) return 0;
    for (uint32_t i = 0; i < cap; i++) {
        dtb_reserved_entry_t entry = libdtb0_get_reserved_entry(header, i);
        if (!entry.address && !entry.size) return i;
    }
    return cap;
}
#define LIBDTB0_CHK_BOUNDS(sz)                                                                                             \
    if ((uint64_t)*cursor + (uint64_t)(sz) > (uint64_t)libdtb0_be2host32(header->size_dt_struct)) return DTB_TOKEN_INVALID
dtb_token_type_t libdtb0_get_next_token(const dtb_header_t* header, const char** node_string, const char** property_string, const dtb_prop_t** property, uint64_t* cursor) {
    if (!header || !node_string || !property_string || !property || !cursor) return DTB_TOKEN_INVALID;
    LIBDTB0_CHK_BOUNDS(sizeof(uint32_t));
    uint32_t token = libdtb0_be2host32(*(uint32_t*)((uintptr_t)header + libdtb0_be2host32(header->off_dt_struct) + *cursor));
    *cursor += 4;
    switch (token) {
        case DTB_TOKEN_BEGIN_NODE: {
            const char* name = (const char*)((uintptr_t)header + libdtb0_be2host32(header->off_dt_struct) + *cursor);
            LIBDTB0_CHK_BOUNDS(sizeof(char));
            uint32_t len = 1;
            for (const char* str = name; *str; str++) {
                LIBDTB0_CHK_BOUNDS(len + sizeof(char));
                len++;
            }
            *cursor += len;
            *cursor = LIBDTB0_ALIGN_UP(*cursor, 4);
            *node_string = name;
            *property_string = (const char*)"";
            *property = NULL;
            return DTB_TOKEN_BEGIN_NODE;
        }
        case DTB_TOKEN_PROP: {
            LIBDTB0_CHK_BOUNDS(sizeof(dtb_prop_t));
            const dtb_prop_t* p = (const dtb_prop_t*)((uintptr_t)header + libdtb0_be2host32(header->off_dt_struct) + *cursor);
            uint32_t nameoff = libdtb0_be2host32(p->nameoff);
            if (nameoff >= libdtb0_be2host32(header->size_dt_strings)) return DTB_TOKEN_INVALID;
            const char* str = (const char*)((uintptr_t)header + libdtb0_be2host32(header->off_dt_strings) + nameoff);
            uint32_t max_namelen = libdtb0_be2host32(header->size_dt_strings) - nameoff;
            uint32_t namelen = 0;
            while (namelen < max_namelen && str[namelen] != '\0') namelen++;
            if (namelen == max_namelen) return DTB_TOKEN_INVALID;
            uint32_t plen = libdtb0_be2host32(p->len);
            LIBDTB0_CHK_BOUNDS(sizeof(dtb_prop_t) + LIBDTB0_ALIGN_UP(plen, 4));
            *cursor += sizeof(dtb_prop_t);
            *cursor += LIBDTB0_ALIGN_UP(plen, 4);
            *property = p;
            *property_string = str;
            return DTB_TOKEN_PROP;
        }
        case DTB_TOKEN_NOP:
        case DTB_TOKEN_END:
        case DTB_TOKEN_END_NODE: return (dtb_token_type_t)token;
        default:                 return DTB_TOKEN_INVALID;
    }
}
#undef LIBDTB0_CHK_BOUNDS
bool libdtb0_prop_read_cell(const dtb_prop_t* prop, uint32_t index, uint32_t* result) {
    if (!prop || !result) return false;
    uint32_t len = libdtb0_be2host32(prop->len);
    if ((uint64_t)index * 4 + 4 > len) return false;
    *result = libdtb0_be2host32(*(const uint32_t*)(prop->data + index * 4));
    return true;
}
bool libdtb0_prop_read_cells_u64(const dtb_prop_t* prop, uint32_t index, uint8_t cells, uint64_t* result) {
    if (!prop || !result) return false;
    if (cells > 2) return false;
    if (cells == 0) {
        *result = 0;
        return true;
    }
    uint32_t len = libdtb0_be2host32(prop->len);
    if ((uint64_t)index * 4 + (uint64_t)cells * 4 > len) return false;
    const uint32_t* data = (const uint32_t*)(prop->data + index * 4);
    if (cells == 1) {
        *result = libdtb0_be2host32(data[0]);
    } else {
        *result = ((uint64_t)libdtb0_be2host32(data[0]) << 32) | libdtb0_be2host32(data[1]);
    }
    return true;
}
bool libdtb0_prop_read_cells_u128(const dtb_prop_t* prop, uint32_t index, uint8_t cells, dtb_u128_t* result) {
    if (!prop || !result) return false;
    if (cells > 4) return false;
    if (cells == 0) {
        result->lo = 0;
        result->hi = 0;
        return true;
    }
    uint32_t len = libdtb0_be2host32(prop->len);
    if ((uint64_t)index * 4 + (uint64_t)cells * 4 > len) return false;
    const uint32_t* data = (const uint32_t*)(prop->data + index * 4);
    switch (cells) {
        case 1:
            result->lo = libdtb0_be2host32(data[0]);
            result->hi = 0;
            return true;
        case 2:
            result->lo = ((uint64_t)libdtb0_be2host32(data[0]) << 32) | libdtb0_be2host32(data[1]);
            result->hi = 0;
            return true;
        case 3:
            result->hi = libdtb0_be2host32(data[0]);
            result->lo = ((uint64_t)libdtb0_be2host32(data[1]) << 32) | libdtb0_be2host32(data[2]);
            return true;
        case 4:
            result->hi = ((uint64_t)libdtb0_be2host32(data[0]) << 32) | libdtb0_be2host32(data[1]);
            result->lo = ((uint64_t)libdtb0_be2host32(data[2]) << 32) | libdtb0_be2host32(data[3]);
            return true;
        default: return false;
    }
}
const char* libdtb0_prop_get_string(const dtb_prop_t* prop) {
    return libdtb0_prop_get_string_at(prop, 0);
}
const char* libdtb0_prop_get_string_at(const dtb_prop_t* prop, uint32_t index) {
    if (!prop) return NULL;
    uint32_t len = libdtb0_be2host32(prop->len);
    if (len == 0) return NULL;
    uint32_t offset = 0;
    for (uint32_t i = 0; i <= index; i++) {
        if (offset >= len) return NULL;
        uint32_t str_start = offset;
        while (offset < len && prop->data[offset] != '\0') offset++;
        if (offset >= len) return NULL;
        if (i == index) return (const char*)(prop->data + str_start);
        offset++;
    }
    return NULL;
}
#endif
#if defined(LIBDTB_IMPL) && !defined(LIBDTB_FREESTANDING)
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
#endif
