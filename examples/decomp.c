#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>

#define LIBDTB_IMPL
#include "../libdtb0_singleheader.h"

#define PRINT_INDENT(d)                                  \
    for (int _i = 0; _i < (d); _i++) { printf("    "); }

static bool is_string_prop(const dtb_prop_t* prop, uint32_t len) {
    if (!isprint(prop->data[0])) return false;
    for (uint32_t i = 0; i < len; i++) {
        uint8_t c = prop->data[i];
        if (c == 0) continue;
        if (!isprint(c)) return false;
    }
    if (prop->data[len - 1] != 0 && memchr(prop->data, 0, len)) return false;
    if (len >= 2 && (len % 4) == 0 && prop->data[1] == 0) return false;
    return true;
}

static void print_dts_cells_u32(const dtb_prop_t* prop, uint32_t ncells) {
    for (uint32_t i = 0; i < ncells; i++) {
        uint32_t v;
        libdtb0_prop_read_cell(prop, i, &v);
        printf("%s0x%x", i ? " " : "", v);
    }
}

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("Usage: dtb-decomp <dtb file>\n");
        return -1;
    }

    FILE* fp = fopen(argv[1], "rb");
    if (!fp) {
        printf("ERR: failed to open file\n");
        return -1;
    }

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    if (size == -1) {
        printf("ERR: failed to get file size\n");
        fclose(fp);
        return -1;
    }

    int fd = fileno(fp);
    void* file = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
    if (file == MAP_FAILED) {
        printf("ERR: failed to MMAP file\n");
        fclose(fp);
        return -1;
    }

    dtb_header_t* header = (dtb_header_t*)file;
    int ret = -1;

    uint32_t n_rsv;
    const char* node_string;
    const char* prop_string;
    const dtb_prop_t* prop;
    uint64_t cursor = 0;
    int depth = 0;

    if (!libdtb0_validate_header(header, size)) {
        printf("ERR: failed to validate DTB header\n");
        goto done;
    }

    printf("/dts-v1/;\n");
    n_rsv = libdtb0_get_reserved_count(header);
    for (uint32_t i = 0; i < n_rsv; i++) {
        dtb_reserved_entry_t entry = libdtb0_get_reserved_entry(header, i);
        printf("/memreserve/ 0x%lx 0x%lx;\n", entry.address, entry.size);
    }

    while (1) {
        dtb_token_type_t type = libdtb0_get_next_token(header, &node_string, &prop_string, &prop, &cursor);

        switch (type) {
            case DTB_TOKEN_BEGIN_NODE:
                if (depth == 0) {
                    printf("/ {\n");
                } else {
                    PRINT_INDENT(depth);
                    printf("%s {\n", node_string);
                }
                depth++;
                break;

            case DTB_TOKEN_PROP: {
                PRINT_INDENT(depth);
                uint32_t len = libdtb0_be2host32(prop->len);

                if (len == 0) {
                    printf("%s;\n", prop_string);
                    break;
                }

                if (is_string_prop(prop, len)) {
                    bool is_list = memchr(prop->data, 0, len - 1) != NULL;
                    if (is_list) {
                        printf("%s = ", prop_string);
                        for (uint32_t i = 0;; i++) {
                            const char* s = libdtb0_prop_get_string_at(prop, i);
                            if (!s) break;
                            if (i > 0) printf(", ");
                            printf("\"%s\"", s);
                        }
                        printf(";\n");
                    } else {
                        printf("%s = \"%.*s\";\n", prop_string, (int)len, (const char*)prop->data);
                    }
                } else if ((len % 4) == 0) {
                    printf("%s = <", prop_string);
                    print_dts_cells_u32(prop, len / 4);
                    printf(">;\n");
                } else {
                    printf("%s = [", prop_string);
                    for (uint32_t i = 0; i < len; i++) printf("%s%02x", i ? " " : "", prop->data[i]);
                    printf("];\n");
                }
                break;
            }

            case DTB_TOKEN_END_NODE:
                depth--;
                PRINT_INDENT(depth);
                printf("};\n");
                break;

            case DTB_TOKEN_END: ret = 0; goto done;

            case DTB_TOKEN_INVALID: printf("ERR: DTB is malformed\n"); goto done;

            default: break;
        }
    }

done:
    munmap(file, size);
    fclose(fp);
    return ret;
}
