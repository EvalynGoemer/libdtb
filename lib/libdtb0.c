#include "libdtb0.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

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
