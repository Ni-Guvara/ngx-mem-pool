### nginx内存池：主要特点就是快，由于内存分配的时候是单纯的指针移动

1. `bool ngx_create_pool(size_t size); `  // 创建内存池
2. ` void* ngx_palloc_small(size_t size, unsigned int align);` // 分配小块内存
3. ` void* ngx_palloc_block(size_t size);` // 对内存进行扩展
4. `void* ngx_palloc_large(size_t size); `// 分配大块内存
5. `void* ngx_palloc(size_t size); `  // 内存字节对齐
6. ` void* ngx_pnalloc(size_t size);`  // 内存字节未对齐
7. ` void* ngx_pcalloc(size_t size);`  // 调用ngx_palloc，但会对内存设置0
8. void ngx_pfree(void* p);`         // 释放大块内存；小块内存不带回收是由于当内存1、3使用中间的2使用 end、last没法移动
9. ` ngx_pool_cleanup_t* ngx_pool_cleanup_add(size_t size);` // 添加回调handler，用于释放外部分配的内存
10. `void ngx_reset_pool();`    //内存重置
11. `void ngx_destroy_pool();`  //内存池销毁
