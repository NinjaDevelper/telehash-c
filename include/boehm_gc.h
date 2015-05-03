#ifndef NO_GC
#ifndef __BOEHM_GC__
#define __BOEHM_GC__
#include <gc.h>
#define malloc GC_malloc
#define realloc GC_realloc
#define calloc(m,n) GC_malloc((m)*(n))
#define free(a) {}
#endif
#endif
