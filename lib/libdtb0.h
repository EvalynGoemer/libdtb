#pragma once

#include <stdbool.h>
#include <stdint.h>

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
