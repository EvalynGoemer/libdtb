# libdtb

A public domain freestanding DTB parsing library for kernels and bootloaders written in C99.

Has a fully freestanding stream based parser (libdtb0)
and a portable parser that unflattens the tree and allows path based search and resolving of phandles (libdtb)

Licenced under 0BSD and dedicated to the public domain where applicable.
See `LICENCE` and `PUBLIC_DOMAIN` for details.

## Using/Porting

### Single header

There are two single header builds you can get

- `libdtb0_singleheader.h` - freestanding streaming parser only
- `libdtb_singleheader.h` - full parser

Before including the header define `LIBDTB_IMPL` in one translation unit only
other files do not need to define it.

Define `LIBDTB_FREESTANDING` as well if you wish to drop the non freestanding parts
when using the full `libdtb_singleheader.h`

Example using custom malloc/free/string macros
helpers

```c
#include <mm/heap.h>
#include <mm/ptm.h>
#include <klibc/string.h>

#define LIBDTB_OWN_MALLOC
#define LIBDTB_OWN_STRING

#define LIBDTB_MALLOC(sz) kmalloc((sz))
#define LIBDTB_FREE(p, sz) kfree((p), (sz))
#define LIBDTB_MALLOC_HMAP(sz) ptm_alloc_page_hhdm()
#define LIBDTB_FREE_HMAP(p, sz) ptm_free_page_hhdm()

#define LIBDTB_STRLEN(s) strlen((s))
#define LIBDTB_STRCMP(a, b) strcmp((a), (b))
#define LIBDTB_STRNCMP(a, b, n) strncmp((a), (b), (n))

#define LIBDTB_IMPL
#include "libdtb_singleheader.h"
```

### Non Single Header

Simply compile in all the `.c` files inside of `/lib` and modify `libdtb_kernel_api.h`
to match your environment and define the macros properly.
