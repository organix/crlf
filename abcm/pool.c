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
 * generic allocator methods
 */

#include <string.h>  // for memcpy, et. al.

static BYTE generic_pool_copy(pool_t * pool, DATA_PTR * data, DATA_PTR value) {
    LOG_LEVEL(LOG_LEVEL_TRACE+1, "generic_pool_copy: value =", (WORD)value);
    parse_t parse = {
        .base = value,
        .size = MAX_WORD,  // don't know how big value will be
        .start = 0
    };
    if (!parse_value(&parse)) return false;  // bad parse!
    WORD size = parse.end - parse.start;  // parse_value determines the span of the value
    if (!pool_reserve(pool, data, size)) return false;  // bad allocation!
    memcpy(*data, parse.base + parse.start, size);
    LOG_LEVEL(LOG_LEVEL_TRACE+2, "generic_pool_copy: data @", (WORD)(*data));
    return true;
}

/*
 * standard heap-memory allocator
 */

#include <stddef.h>  // for NULL, size_t, et. al.
#include <stdlib.h>  // for malloc, et. al.
//#include <assert.h>

typedef struct {
    pool_t      pool;           // super-type member
} heap_pool_t;

static BYTE heap_pool_reserve(pool_t * pool, DATA_PTR * data, WORD size) {
    //heap_pool_t * THIS = (heap_pool_t *)pool;
    LOG_LEVEL(LOG_LEVEL_TRACE+1, "heap_pool_reserve: size =", size);
    VOID_PTR p = malloc(size);
    *data = p;  // stomp on data either way!
    LOG_LEVEL(LOG_LEVEL_TRACE+1, "heap_pool_reserve: data @", (WORD)(*data));
    BYTE ok = (p != NULL);
    if (!ok) {
        LOG_WARN("heap_pool_reserve: OUT-OF-MEMORY!", size);
    }
    return ok;
}

static BYTE heap_pool_release(pool_t * pool, DATA_PTR * data) {
    //heap_pool_t * THIS = (heap_pool_t *)pool;
    LOG_LEVEL(LOG_LEVEL_TRACE+1, "heap_pool_release: data @", (WORD)(*data));
    VOID_PTR p = *data;
    if (p == NULL) return false;
    free(p);
    *data = NULL;  // destroy pointer to free'd memory
    LOG_LEVEL(LOG_LEVEL_TRACE+2, "heap_pool_release: data =", (WORD)(*data));
    return true;
}

static BYTE heap_pool_close(pool_t * pool) {
    LOG_WARN("heap_pool does not close", (WORD)(pool));
    return false;
}

static heap_pool_t heap_pool_instance = {
    .pool.reserve = heap_pool_reserve,
    .pool.copy = generic_pool_copy,
    .pool.release = heap_pool_release,
    .pool.close = heap_pool_close,
};
pool_t * heap_pool = (pool_t *)&heap_pool_instance;

/*
 * simple linear allocator
 */

#include <stddef.h>  // for NULL, size_t, et. al.
//#include <assert.h>

typedef struct {
    pool_t      pool;           // super-type member
    DATA_PTR    base;           // base address of managed memory
    WORD        size;           // size of managed memory (in bytes)
    WORD        offset;         // offset to next available allocation
} temp_pool_t;

static BYTE temp_pool_reserve(pool_t * pool, DATA_PTR * data, WORD size) {
    temp_pool_t * THIS = (temp_pool_t *)pool;
    LOG_LEVEL(LOG_LEVEL_TRACE+1, "temp_pool_reserve: size =", size);
    VOID_PTR p = NULL;
    if (THIS->offset + size <= THIS->size) {
        p = THIS->base + THIS->offset;
        THIS->offset += size;
    }
    *data = p;  // stomp on data either way!
    LOG_LEVEL(LOG_LEVEL_TRACE+1, "temp_pool_reserve: data @", (WORD)(*data));
    BYTE ok = (p != NULL);
    if (!ok) {
        LOG_WARN("temp_pool_reserve: OUT-OF-MEMORY!", THIS->offset);
    }
    return ok;
}

static BYTE temp_pool_release(pool_t * pool, DATA_PTR * data) {
    //temp_pool_t * THIS = (temp_pool_t *)pool;
    LOG_LEVEL(LOG_LEVEL_TRACE+1, "temp_pool_release: data @", (WORD)(*data));
    LOG_DEBUG("temp_pool needs no release", (WORD)(*data));
    *data = NULL;  // destroy pointer to "free'd" memory
    LOG_LEVEL(LOG_LEVEL_TRACE+2, "temp_pool_release: data =", (WORD)(*data));
    return true;
}

static BYTE temp_pool_close(pool_t * pool) {
    // FIXME: WE CAN'T RELEASE THE POOL ITESELF BECAUSE WE DON'T KNOW WHO ALLOCATED IT!!
    temp_pool_t * THIS = (temp_pool_t *)pool;
    LOG_DEBUG("temp_pool_close: offset =", THIS->offset);
    LOG_DEBUG("temp_pool_close: size =", THIS->size);
    return true;
}

pool_t * new_temp_pool(pool_t * parent, WORD size) {
    temp_pool_t * THIS;
    if (!pool_reserve(parent, (DATA_PTR *)&THIS, sizeof(temp_pool_t) + size)) {
        LOG_WARN("new_temp_pool: could not reserve memory for pool", size);
    }
    THIS->base = (DATA_PTR)(THIS + 1);  // managed memory starts after pool structure
    THIS->size = size;
    THIS->offset = 0;
    THIS->pool.reserve = temp_pool_reserve;
    THIS->pool.copy = generic_pool_copy;
    THIS->pool.release = temp_pool_release;
    THIS->pool.close = temp_pool_close;
    LOG_DEBUG("new_temp_pool: created", size);
    return (pool_t *)THIS;
}

/*
 * polymorphic dispatch functions
 */

inline BYTE pool_reserve(pool_t * pool, DATA_PTR * data, WORD size) {
    return pool->reserve(pool, data, size);
}

inline BYTE pool_copy(pool_t * pool, DATA_PTR * data, DATA_PTR value) {
    return pool->copy(pool, data, value);
}

inline BYTE pool_release(pool_t * pool, DATA_PTR * data) {
    return pool->release(pool, data);
}

inline BYTE pool_close(pool_t * pool) {
    return pool->close(pool);
}
