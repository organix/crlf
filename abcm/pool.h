/*
 * pool.h -- memory management pools
 */
#ifndef _POOL_H_
#define _POOL_H_

#include "bose.h"

typedef struct pool_struct pool_t;
typedef struct pool_struct {
    DATA_PTR    base;           // base address of the memory pool
    WORD        size;           // size (in bytes) of valid pool data [0, size-1]
	BYTE 		(*reserve)(pool_t * pool, DATA_PTR * data, WORD size);
	BYTE 		(*share)(pool_t * pool, DATA_PTR * data);
	BYTE 		(*release)(pool_t * pool, DATA_PTR * data);
} pool_t;

//BYTE pool_create(pool_t * pool, WORD size);
BYTE pool_reserve(pool_t * pool, DATA_PTR * data, WORD size);
BYTE pool_share(pool_t * pool, DATA_PTR * data);
BYTE pool_release(pool_t * pool, DATA_PTR * data);
//BYTE pool_destroy(pool_t * pool);

extern pool_t * heap_pool;

#endif // _POOL_H_
