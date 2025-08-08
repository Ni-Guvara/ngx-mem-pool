#include "Ngx_Mem_Pool.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Data stData;
struct Data
{
    char* ptr;
    FILE* pfile;
};

void func1(void* p)
{
    char* p1 = (char*)p;
    printf("free ptr mem!");
    free(p1);
}
void func2(void* pf)
{
    FILE* pf1 = (FILE*)pf;
    printf("close file!");
    fclose(pf1);
}
void main()
{
    Ngx_Mem_Pool pool;
    
    // 512 - sizeof(ngx_pool_t) - 4095   =>   max
    if (false == pool.ngx_create_pool(512))
    {
        printf("ngx_create_pool fail...");
        return;
    }
  
    void* p1 = pool.ngx_palloc(128); 
   
    stData* p2 = (stData*)pool.ngx_palloc(512); 
    if (p2 == NULL)
    {
        printf("ngx_palloc 512 bytes fail...");
        return;
    }
    p2->ptr = (char*)malloc(12);
    strcpy(p2->ptr, "hello world");
    p2->pfile = fopen("data.txt", "w");

    ngx_pool_cleanup_t* c1 = pool.ngx_pool_cleanup_add(sizeof(char*));
    c1->handler = func1;
    c1->data = p2->ptr;

    ngx_pool_cleanup_t* c2 = pool.ngx_pool_cleanup_add(sizeof(FILE*));
    c2->handler = func2;
    c2->data = p2->pfile;

    pool.ngx_destroy_pool(); 

    return;
}
