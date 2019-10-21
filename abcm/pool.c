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

#include <stddef.h>  // for NULL, size_t, et. al.
#include <stdlib.h>  // for malloc, et. al.
#include <string.h>  // for memcpy, et. al.
//#include <assert.h>

typedef struct {
    pool_t      pool;           // super-type member
} heap_pool_t;

static BYTE heap_pool_reserve(pool_t * pool, DATA_PTR * data, WORD size) {
    //heap_pool_t * THIS = (heap_pool_t *)pool;
    LOG_TRACE("heap_pool_reserve: size =", size);
    VOID_PTR p = malloc(size);
    *data = p;  // stomp on data either way!
    LOG_TRACE("heap_pool_reserve: data @", (WORD)(*data));
    return (p != NULL);
}

static BYTE heap_pool_share(pool_t * pool, DATA_PTR * data) {
    LOG_WARN("heap_pool does not reference-count", (WORD)(*data));
    return false;
}

static BYTE heap_pool_copy(pool_t * pool, DATA_PTR * data, DATA_PTR value) {
    //heap_pool_t * THIS = (heap_pool_t *)pool;
    LOG_TRACE("heap_pool_copy: value =", (WORD)value);
    parse_t parse = {
        .base = value,
        .size = MAX_WORD,  // don't know how big value will be
        .start = 0
    };
    if (!parse_value(&parse)) return false;  // bad parse!
    WORD size = parse.end - parse.start;  // parse_value determines the span of the value
    if (!heap_pool_reserve(pool, data, size)) return false;  // bad allocation!
    memcpy(*data, parse.base, size);
    LOG_TRACE("heap_pool_copy: data @", (WORD)(*data));
    return true;
}

static BYTE heap_pool_release(pool_t * pool, DATA_PTR * data) {
    //heap_pool_t * THIS = (heap_pool_t *)pool;
    LOG_TRACE("heap_pool_release: data @", (WORD)(*data));
    VOID_PTR p = *data;
    if (p == NULL) return false;
    free(p);
    *data = NULL;  // destroy pointer to free'd memory
    LOG_TRACE("heap_pool_release: data =", (WORD)(*data));
    return true;
}

static heap_pool_t heap_pool_instance = {
    .pool.reserve = heap_pool_reserve,
    .pool.share = heap_pool_share,
    .pool.copy = heap_pool_copy,
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

inline BYTE pool_copy(pool_t * pool, DATA_PTR * data, DATA_PTR value) {
    return pool->copy(pool, data, value);
}

inline BYTE pool_release(pool_t * pool, DATA_PTR * data) {
    return pool->release(pool, data);
}
