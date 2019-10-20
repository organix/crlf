/*
 * pool.c -- memory management pools
 */
#include <assert.h>

#include "pool.h"
#include "bose.h"

#define LOG_ALL // enable all logging
//#define LOG_INFO
//#define LOG_WARN
#include "log.h"

/*
 * standard heap-memory allocator
 */
#include <stdlib.h>  // for malloc, et. al.

typedef struct {
    pool_t      pool;           // super-type member
} heap_pool_t;

static BYTE heap_pool_reserve(pool_t * pool, DATA_PTR * data, WORD size) {
    //heap_pool_t * THIS = (heap_pool_t *)pool;
    VOID_PTR p = malloc(size);
    *data = p;  // stomp on data either way!
    return (p != NULL);
}

static BYTE heap_pool_share(pool_t * pool, DATA_PTR * data) {
    LOG_WARN("heap_pool does not reference-count", (WORD)(*data));
    return false;
}

static BYTE heap_pool_release(pool_t * pool, DATA_PTR * data) {
    //heap_pool_t * THIS = (heap_pool_t *)pool;
    VOID_PTR p = *data;
    if (p == NULL) return false;
    free(p);
    *data = NULL;  // destroy pointer to free'd memory
    return true;
}

static heap_pool_t heap_pool_instance = {
    .pool.reserve = heap_pool_reserve,
    .pool.share = heap_pool_share,
    .pool.release = heap_pool_release,
};
pool_t * heap_pool = (pool_t *)&heap_pool_instance;

/*
 * polymorphic dispatch functions
 */
inline BYTE pool_reserve(pool_t * pool, DATA_PTR * data, WORD size) {
    return pool->reserve(pool, data, size);
}

inline BYTE pool_share(pool_t * pool, DATA_PTR * data) {
    return pool->share(pool, data);
}

inline BYTE pool_release(pool_t * pool, DATA_PTR * data) {
    return pool->release(pool, data);
}
