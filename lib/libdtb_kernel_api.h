#pragma once

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
