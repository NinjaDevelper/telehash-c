#ifndef __TGC__
#define __TGC__

#include <stdlib.h>

#define malloc tgc_malloc
#define realloc tgc_realloc
#define calloc(m,n) tgc_malloc((m)*(n))
#define free(a) {}


void tgc_info();
void tgc_gcollect();
void tgc_rmRoot(void *ptr);
void tgc_addRoot(void *ptr);
void *tgc_realloc(void *ptr,size_t size);
void *tgc_malloc(size_t size);

#endif
