/*
 * pool.h -- memory management pools
 */
#ifndef _POOL_H_
#define _POOL_H_

#include "bose.h"

typedef struct pool_struct pool_t;
typedef struct pool_struct {
    BYTE        (*reserve)(pool_t * pool, DATA_PTR * data, WORD size);
    BYTE        (*copy)(pool_t * pool, DATA_PTR * data, DATA_PTR value);
    BYTE        (*release)(pool_t * pool, DATA_PTR * data);
    BYTE        (*close)(pool_t * pool);
} pool_t;

BYTE pool_reserve(pool_t * pool, DATA_PTR * data, WORD size);
BYTE pool_copy(pool_t * pool, DATA_PTR * data, DATA_PTR value);
BYTE pool_release(pool_t * pool, DATA_PTR * data);
BYTE pool_close(pool_t * pool);

extern pool_t * heap_pool;  // explicit reserve/release pool

pool_t * new_temp_pool(pool_t * parent, WORD size);  // create temporary working-memory pool

#endif // _POOL_H_
