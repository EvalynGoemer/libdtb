#include <stdio.h>
#include <sys/mman.h>

#include "../lib/libdtb.h"

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("Usage: standalone <dtb file>\n");
        return -1;
    }

    FILE* fp = fopen(argv[1], "rb");
    if (!fp) {
        printf("ERR: failed to open file\n");
        return -1;
    }

    fseek(fp, 0, SEEK_END);
    long sz = ftell(fp);
    if (sz == -1) {
        printf("ERR: failed to get file size\n");
        fclose(fp);
        return -1;
    }

    int fd = fileno(fp);
    void* f = mmap(NULL, sz, PROT_READ, MAP_SHARED, fd, 0);
    if (f == MAP_FAILED) {
        printf("ERR: failed to MMAP file\n");
        fclose(fp);
        return -1;
    }

    devicetree_t* t = libdtb_create(f, sz);
    if (!t) {
        printf("ERR: failed to parse DTB\n");
        munmap(f, sz);
        fclose(fp);
        return -1;
    }

    dt_node_t* soc = libdtb_find_path(t, "/soc");
    if (soc) {
        printf("found /soc\n");
        const dtb_prop_t* p = libdtb_find_prop(soc, "compatible");
        if (p) {
            const char* compat = libdtb0_prop_get_string(p);
            printf("  compatible = %s\n", compat ? compat : "(null)");
        }
    }

    dt_node_t* chosen = libdtb_find_child(t->root, "chosen");
    if (chosen) {
        printf("found child 'chosen'\n");
        const dtb_prop_t* p = libdtb_find_prop(chosen, "stdout-path");
        if (p) {
            const char* console = libdtb0_prop_get_string(p);
            printf("  stdout-path = %s\n", console ? console : "(null)");
        }
    }

    libdtb_destroy(t);
    munmap(f, sz);
    fclose(fp);
    return 0;
}
