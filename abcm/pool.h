/*
 * pool.h -- memory management pools
 */
#ifndef _POOL_H_
#define _POOL_H_

#include "bose.h"

typedef struct pool_struct pool_t;
typedef struct pool_struct {
	BYTE 		(*reserve)(pool_t * pool, DATA_PTR * data, WORD size);
	BYTE 		(*share)(pool_t * pool, DATA_PTR * data);
	BYTE 		(*copy)(pool_t * pool, DATA_PTR * data, DATA_PTR value);
	BYTE 		(*release)(pool_t * pool, DATA_PTR * data);
} pool_t;

BYTE pool_reserve(pool_t * pool, DATA_PTR * data, WORD size);
BYTE pool_share(pool_t * pool, DATA_PTR * data);
BYTE pool_copy(pool_t * pool, DATA_PTR * data, DATA_PTR value);
BYTE pool_release(pool_t * pool, DATA_PTR * data);

extern pool_t * heap_pool;  // explicit reserve/release pool

#endif // _POOL_H_
