#ifndef __TGC__
#define __TGC__

#include <stdlib.h>
#include <string.h>

#define malloc tgc_malloc
#define realloc tgc_realloc
#define strdup tgc_strdup
#define calloc(m,n) tgc_malloc((m)*(n))
#define free(a) tgc_free(a)


void tgc_info();
void tgc_gcollect();
void tgc_rmRoot(void *ptr);
void tgc_addRoot(void *ptr);
void *tgc_realloc(void *ptr,size_t size);
void *tgc_malloc(size_t size);
char *tgc_strdup(const char *s);
void tgc_free(void *ptr);
void tgc_gcollect_force();
void tgc_setGC(int _useGC);

#endif
