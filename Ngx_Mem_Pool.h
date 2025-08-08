#pragma once
#include <memory.h>
#include <stdlib.h>


#define ngx_memzero(buf, n)   (void) memset(buf, 0, n)
#define ngx_align_ptr(p, a)                                                   \
    (u_char *) (((uintptr_t) (p) + ((uintptr_t) a - 1)) & ~((uintptr_t) a - 1))

#ifndef NGX_ALIGNMENT
#define NGX_ALIGNMENT   sizeof(unsigned long)    /* platform word */
#endif

typedef void (*ngx_pool_cleanup_pt)(void* data);
using u_char = unsigned char;

struct ngx_pool_cleanup_t {
    ngx_pool_cleanup_pt   handler;
    void* data;   
    ngx_pool_cleanup_t* next;  
};


struct ngx_pool_large_t {
    ngx_pool_large_t* next;    
    void* alloc;   
};

struct ngx_pool_t; 

struct ngx_pool_data_t {
    u_char* last;   
    u_char* end;   
    ngx_pool_t* next;   
    unsigned int                 failed; 
};


struct ngx_pool_t {
    ngx_pool_data_t       d;         
    size_t                max;       
    ngx_pool_t* current;   
    ngx_pool_large_t* large;    
    ngx_pool_cleanup_t* cleanup;  
};

#define ngx_align(d, a)     (((d) + (a - 1)) & ~(a - 1))            
const int ngx_pagesize = 4096;                                       
const int NGX_MAX_ALLOC_FROM_POOL = (ngx_pagesize - 1);          
const int NGX_DEFAULT_POOL_SIZE = (16 * 1024);                     
const int NGX_POOL_ALIGNMENT = 16;                                 
const int NGX_MIN_POOL_SIZE = ngx_align((sizeof(ngx_pool_t) + 2 * sizeof(ngx_pool_large_t)), NGX_POOL_ALIGNMENT); 


class Ngx_Mem_Pool {

private:
    ngx_pool_t* pool;

    void* ngx_palloc_small(size_t size, unsigned int align); 
    void* ngx_palloc_block(size_t size); 
    void* ngx_palloc_large(size_t size); 
public:
    bool ngx_create_pool(size_t size);                 
    void* ngx_palloc(size_t size);   
    void* ngx_pnalloc(size_t size); 
    void* ngx_pcalloc(size_t size);  
    void ngx_pfree(void* p);        
    ngx_pool_cleanup_t* ngx_pool_cleanup_add(size_t size); 
    void ngx_reset_pool();  
    void ngx_destroy_pool();  

};