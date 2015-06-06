#ifndef __TGC__
#define __TGC__

#include <stdlib.h>
#include <string.h>
#if 0
#define malloc(m) tgc_malloc(m, __FILE__, __LINE__, __func__)
#define realloc(m,n) tgc_realloc(m,n, __FILE__, __LINE__, __func__)
#define strdup(m) tgc_strdup(m, __FILE__, __LINE__, __func__)
#define calloc(m,n) tgc_malloc((m)*(n),__FILE__, __LINE__, __func__)
#define free(a) tgc_free(a)
#endif

void tgc_info();
void tgc_gcollect();
void tgc_rmRoot(void *ptr);
void tgc_addRoot(void *ptr);
void *tgc_realloc(void *ptr,size_t size,const  char *file, int line, const char *func);
void *tgc_malloc(size_t size, const char *file, int line, const  char *func);
char *tgc_strdup(const char *s,  const char *file, int line, const char *func);
void tgc_free(void *ptr);


#endif
