#include "Ngx_Mem_Pool.h"

bool Ngx_Mem_Pool::ngx_create_pool(size_t size)
{
  
    pool = (ngx_pool_t*)malloc(size);
    if (pool == nullptr) {
        return false;
    }

    pool->d.last = (u_char*)pool + sizeof(ngx_pool_t);
    pool->d.end = (u_char*)pool + size;
    pool->d.next = nullptr;
    pool->d.failed = 0;

    size = size - sizeof(ngx_pool_t);
    pool->max = (size < NGX_MAX_ALLOC_FROM_POOL) ? size : NGX_MAX_ALLOC_FROM_POOL;

    pool->current = pool;
    pool->large = nullptr;
    pool->cleanup = nullptr;

    return true;
}

void* Ngx_Mem_Pool::ngx_palloc(size_t size)
{
    if (size <= pool->max) {
        return ngx_palloc_small(size, 1);
    }
    return ngx_palloc_large(size);
}

void* Ngx_Mem_Pool::ngx_pnalloc(size_t size)
{
    if (size <= pool->max) {
        return ngx_palloc_small(size, 0);
    }
    return ngx_palloc_large(size);
}


void* Ngx_Mem_Pool::ngx_pcalloc(size_t size)
{
    void* p;

    p = ngx_palloc(size);
    if (p) {
        ngx_memzero(p, size);
    }
    return p;
}

void Ngx_Mem_Pool::ngx_pfree(void* p)
{
    ngx_pool_large_t* l;

    for (l = pool->large; l; l = l->next) {
        if (p == l->alloc) {
            free(l->alloc);
            l->alloc = nullptr;
            return;
        }
    }
}



ngx_pool_cleanup_t* Ngx_Mem_Pool::ngx_pool_cleanup_add(size_t size)
{
    ngx_pool_cleanup_t* c;

    c = (ngx_pool_cleanup_t*)ngx_palloc(sizeof(ngx_pool_cleanup_t));
    if (c == nullptr) {
        return nullptr;
    }

    if (size) {
        c->data = ngx_palloc(size);
        if (c->data == nullptr) {
            return nullptr;
        }
    }
    else {
        c->data = nullptr;
    }

    c->handler = nullptr;
    c->next = pool->cleanup;

    pool->cleanup = c;
    return c;
}

void Ngx_Mem_Pool::ngx_reset_pool()
{
    ngx_pool_t* p;
    ngx_pool_large_t* l;

    for (l = pool->large; l; l = l->next) {
        if (l->alloc) {
            free(l->alloc);
        }
    }

    pool->d.last = (u_char*)pool + sizeof(ngx_pool_t);
    pool->d.failed = 0;

    for (p = pool->d.next; p; p = p->d.next) {
        p->d.last = (u_char*)p + sizeof(ngx_pool_t);
        p->d.failed = 0;
    }

    pool->current = pool;
    pool->large = nullptr;
}

void Ngx_Mem_Pool::ngx_destroy_pool()
{
    ngx_pool_t* p, * n;
    ngx_pool_large_t* l;
    ngx_pool_cleanup_t* c;

    for (c = pool->cleanup; c; c = c->next) {
        if (c->handler) {
            c->handler(c->data);
        }
    }

    for (l = pool->large; l; l = l->next) {
        if (l->alloc) {
            free(l->alloc);
        }
    }

    for (p = pool, n = pool->d.next; /* void */; p = n, n = n->d.next) {
        free(p);

        if (n == nullptr) {
            break;
        }
    }
}


void* Ngx_Mem_Pool::ngx_palloc_small(size_t size, unsigned int align) {
    u_char* m;
    ngx_pool_t* p;

    p = pool->current;

    do {
        m = p->d.last;

        if (align) {
            m = ngx_align_ptr(m, NGX_ALIGNMENT);
        }

        if ((size_t)(p->d.end - m) >= size) {
            p->d.last = m + size;

            return m;
        }

        p = p->d.next;

    } while (p);

    return ngx_palloc_block(size);
}

void* Ngx_Mem_Pool::ngx_palloc_block(size_t size) {
    u_char* m;
    size_t       psize;
    ngx_pool_t* p, * _new;

    psize = (size_t)(pool->d.end - (u_char*)pool);

    m = (u_char*)malloc(psize);
    if (m == nullptr) {
        return nullptr;
    }

    _new = (ngx_pool_t*)m;

    _new->d.end = m + psize;
    _new->d.next = nullptr;
    _new->d.failed = 0;

    m += sizeof(ngx_pool_data_t);
    m = ngx_align_ptr(m, NGX_ALIGNMENT);
    _new->d.last = m + size;

    for (p = pool->current; p->d.next; p = p->d.next) {
        if (p->d.failed++ > 4) {
            pool->current = p->d.next;
        }
    }

    p->d.next = _new;

    return m;
}

void* Ngx_Mem_Pool::ngx_palloc_large(size_t size) {
    void* p;
    unsigned int    n;
    ngx_pool_large_t* large;

    p = malloc(size);
    if (p == nullptr) {
        return nullptr;
    }

    n = 0;

    for (large = pool->large; large; large = large->next) {
        if (large->alloc == nullptr) {
            large->alloc = p;
            return p;
        }

        if (n++ > 3) {
            break;
        }
    }

    large = (ngx_pool_large_t*)ngx_palloc_small(sizeof(ngx_pool_large_t), 1);
    if (large == nullptr) {
        free(p);
        return nullptr;
    }

    large->alloc = p;
    large->next = pool->large;
    pool->large = large;

    return p;
}