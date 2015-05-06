#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>


#ifdef __MAIN__
void *tgc_util_sys_log(const char *file, int line, const char *function, const char * format, ...)
{
  char buffer[256];
  va_list args;
  va_start (args, format);
  vsnprintf (buffer, 256, format, args);
  fprintf(stderr,"%s:%d %s() %s\n", file, line, function, buffer);
  fflush(stderr);
  va_end (args);
  return NULL;

}
#define LOG(fmt, ...) tgc_util_sys_log(__FILE__, __LINE__, __func__, fmt, ## __VA_ARGS__)
#else
#include "util.h"

#endif

typedef struct tgc_list {
    void *ptr;
    struct tgc_list *next;
    struct tgc_list *prev;
    int used;
    size_t size;
} *tgc_list_t;

tgc_list_t blocks=NULL;
tgc_list_t gcRoot=NULL;

tgc_list_t _tgc_addList(tgc_list_t* top,void *ptr){
    tgc_list_t list=(tgc_list_t)calloc(1,sizeof(struct tgc_list));
    list->ptr=ptr;
    list->next=(*top);
    list->prev=NULL;

    if((*top)){
        (*top)->prev=list;
    }
    
    (*top)=list;
    return list;
}

void _tgc_rmList(tgc_list_t *top,tgc_list_t list){
    if(list->next){
        list->next->prev=list->prev;
    }
    if(list->prev){
        list->prev->next=list->next;
    }
    if((*top)==list) (*top)=list->next;
    free(list);
}

void *tgc_malloc(size_t size){
    void *ptr=calloc(1,size);
    tgc_list_t list=_tgc_addList(&blocks,ptr);
    list->size=size;
    return ptr;
}

void *tgc_realloc(void *ptr,size_t size){
    tgc_list_t list=NULL;
    if(ptr==NULL) return tgc_malloc(size);
    
    for(list=blocks;list;list=list->next){
        if(list->ptr==ptr) break;
    }
    if(list==NULL){
        LOG("not allocated memory %h",ptr);
        exit(1);
     }else{
        ptr=list->ptr=realloc(ptr,size);
        list->size=size;
    }
    return ptr;
}

void tgc_addRoot(void *ptr){
    _tgc_addList(&gcRoot,ptr);
}    

void tgc_rmRoot(void *ptr){
    tgc_list_t list=NULL;
    for(list=gcRoot;!list;list=list->next){
        if(list->ptr==ptr){
             _tgc_rmList(&gcRoot,list);
             return;
         }
     }
     return;
}    

void _tgc_mark(void *ptr){
    tgc_list_t list=NULL;
    size_t i=0;
    
    for(list=blocks;list;list=list->next){
        if(list->ptr<=ptr && ptr<=list->ptr+list->size && !list->used){
            list->used=1;
            if(list->size>=sizeof(void*)){
                for(i=0;i<list->size-sizeof(void*)+1;i++){
                   void *p=*(void **)(list->ptr+i);
                    _tgc_mark(p);
                }
           }
        } 
    }
}
   
void tgc_gcollect(){
    tgc_list_t list=NULL;
    for(list=blocks;list;list=list->next){
        list->used=0;
    }
    for(list=gcRoot;list;list=list->next){
        _tgc_mark(list->ptr);
    }
    for(list=blocks;list;){
        tgc_list_t next=list->next;
        if(!list->used){
            LOG("freeing %p",list->ptr);
            free(list->ptr);
            _tgc_rmList(&blocks,list);
        }
        list=next;
    }
}   

void tgc_info(){
    int count=0;
    tgc_list_t list=NULL;

    for(list=blocks;list;list=list->next){
        LOG("not freed blocks=%p",list->ptr);
        count++;
    }
    LOG("# of not freed blocks=%d",count);
}

#ifdef __MAIN__

struct test {
    long t0;
    int *t1;
    struct test* t2;
    int t3;
    long *t4;
};
 
 int main(int argc,char *argv[]){
    int i=0;
    struct test*t=(struct test*)tgc_malloc(sizeof(struct test));
    LOG("should not be freed %p",t);

    tgc_addRoot(t);    
    for(i=0;i<10;i++){
        tgc_malloc(10);
    }
    struct test*t1=(struct test*)tgc_malloc(sizeof(struct test));
    struct test*t2=(struct test*)tgc_malloc(sizeof(struct test));
    LOG("should not be freed %p",t1);
    LOG("should not be freed %p",t2);
    t->t0=1341234;
    int ttt=123;
    t->t1=&ttt;
    t->t2=t1;
    t->t3=45775;
    long *l=(long *)tgc_malloc(sizeof(long));
    LOG("should not be freed %p",l);
    (*l)=123;
    t->t4=l;
    t1->t2=t2;    
    tgc_info();     
    tgc_gcollect();
    tgc_info();     
    return 0;
 }
#endif
