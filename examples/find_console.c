#include <stdio.h>
#include <sys/mman.h>

#define LIBDTB_IMPL
#include "../libdtb_singleheader.h"

static void print_prop_string(const dtb_prop_t* p, const char* label) {
    const char* s = libdtb0_prop_get_string(p);
    if (s) printf("  %s = \"%s\"\n", label, s);
}

static void print_prop_cells(const dtb_prop_t* p, const char* label) {
    uint32_t len = libdtb0_be2host32(p->len);
    if (len == 0) return;
    uint32_t ncells = len / 4;
    printf("  %s = <", label);
    for (uint32_t i = 0; i < ncells; i++) {
        uint32_t v;
        libdtb0_prop_read_cell(p, i, &v);
        printf("%s0x%x", i ? " " : "", v);
    }
    printf(">\n");
}

static void print_prop_cell(const dtb_prop_t* p, uint32_t idx, const char* label) {
    uint32_t v;
    if (libdtb0_prop_read_cell(p, idx, &v)) printf("  %s = 0x%x\n", label, v);
}

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("Usage: find-console <dtb file>\n");
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

    printf("=== device tree loaded: %ld bytes ===\n", sz);

    dt_node_t* root = t->root;
    const dtb_prop_t* p = libdtb_find_prop(root, "model");
    if (p) {
        const char* model = libdtb0_prop_get_string(p);
        if (model) printf("model: %s\n", model);
    }

    p = libdtb_find_prop(root, "compatible");
    if (p) {
        printf("compatible: ");
        for (uint32_t i = 0;; i++) {
            const char* s = libdtb0_prop_get_string_at(p, i);
            if (!s) break;
            if (i > 0) printf(", ");
            printf("%s", s);
        }
        printf("\n");
    }

    printf("\n=== finding preferred console ===\n");
    dt_node_t* chosen = libdtb_find_path(t, "/chosen");
    if (!chosen) {
        printf("no /chosen node\n");
        goto done;
    }

    const dtb_prop_t* stdout_prop = libdtb_find_prop(chosen, "stdout-path");
    if (!stdout_prop) {
        printf("no stdout-path property\n");
        goto done;
    }

    const char* console_spec = libdtb0_prop_get_string(stdout_prop);
    if (!console_spec) {
        printf("stdout-path is empty\n");
        goto done;
    }
    printf("chosen/stdout-path = \"%s\"\n", console_spec);

    const char* colon = NULL;
    const char* baud = NULL;
    size_t device_len = 0;
    for (const char* s = console_spec; *s; s++) {
        if (*s == ':') {
            colon = s;
            baud = colon + 1;
            break;
        }
    }
    device_len = colon ? (size_t)(colon - console_spec) : LIBDTB_STRLEN(console_spec);
    if (colon) {
        printf("  device label: %.*s\n", (int)device_len, console_spec);
        printf("  baud rate:    %s\n", baud);
    } else {
        printf("  device label: %s\n", console_spec);
    }

    printf("\n=== checking aliases node ===\n");
    dt_node_t* aliases = libdtb_find_path(t, "/aliases");
    if (!aliases) {
        printf("no /aliases node\n");
        goto done;
    }

    int found = 0;
    libdtb_llist_node_t* an;
    for (an = aliases->props.head; an; an = an->next) {
        dt_prop_t* ap = LIBDTB_CONTAINER_OF(an, dt_prop_t, llnode);
        if (LIBDTB_STRNCMP(ap->name, console_spec, device_len) != 0 || ap->name[device_len] != '\0') continue;

        found = 1;
        const char* real_path = libdtb0_prop_get_string(ap->raw);
        printf("aliases/%.*s = \"%s\"\n", (int)device_len, console_spec, real_path ? real_path : "(null)");

        if (!real_path) break;

        dt_node_t* uart = libdtb_find_path(t, real_path);
        if (!uart) {
            printf("ERR: alias target '%s' not found in tree\n", real_path);
            break;
        }

        printf("\n=== console device: %s ===\n", real_path);

        p = libdtb_find_prop(uart, "compatible");
        if (p) print_prop_string(p, "compatible");

        p = libdtb_find_prop(uart, "reg");
        if (p) print_prop_cells(p, "reg");

        p = libdtb_find_prop(uart, "interrupts");
        if (p) print_prop_cells(p, "interrupts");

        p = libdtb_find_prop(uart, "clock-frequency");
        if (p) print_prop_cell(p, 0, "clock-frequency");

        p = libdtb_find_prop(uart, "current-speed");
        if (p) print_prop_cell(p, 0, "current-speed");

        p = libdtb_find_prop(uart, "status");
        if (p) print_prop_string(p, "status");
        break;
    }
    if (!found) printf("no alias '%.*s' found\n", (int)device_len, console_spec);

done:
    libdtb_destroy(t);
    munmap(f, sz);
    fclose(fp);
    return 0;
}
