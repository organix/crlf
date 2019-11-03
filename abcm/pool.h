/*
 * pool.h -- memory management pools
 */
#ifndef _POOL_H_
#define _POOL_H_

#include "bose.h"

#define AUDIT_ALLOCATION 1 /* track reserve/release calls to check for leaks */

#if AUDIT_ALLOCATION
#define RESERVE(dpp,size) audit_reserve(__FILE__, __LINE__, SPONSOR_POOL(sponsor), (dpp), (size))
#define RESERVE_FROM(pool,dpp,size) audit_reserve(__FILE__, __LINE__, (pool), (dpp), (size))
#define COPY(to_dpp,from_dp) audit_copy(__FILE__, __LINE__, SPONSOR_POOL(sponsor), (to_dpp), (from_dp))
#define COPY_INTO(pool,to_dpp,from_dp) audit_copy(__FILE__, __LINE__, (pool), (to_dpp), (from_dp))
#define RELEASE(dpp) audit_release(__FILE__, __LINE__, SPONSOR_POOL(sponsor), (dpp))
#define RELEASE_FROM(pool,dpp) audit_release(__FILE__, __LINE__, (pool), (dpp))
#define RELEASE_ALL(pool) audit_release_all(__FILE__, __LINE__, (pool))
#define TRACK(vp) audit_track(__FILE__, __LINE__, SPONSOR_POOL(sponsor), (vp))
#else
#define RESERVE(dpp,size) pool_reserve(SPONSOR_POOL(sponsor), (dpp), (size))
#define RESERVE_FROM(pool,dpp,size) pool_reserve((pool), (dpp), (size))
#define COPY(to_dpp,from_dp) pool_copy(SPONSOR_POOL(sponsor), (to_dpp), (from_dp))
#define COPY_INTO(pool,to_dpp,from_dp) pool_copy((pool), (to_dpp), (from_dp))
#define RELEASE(dpp) pool_release(SPONSOR_POOL(sponsor), (dpp))
#define RELEASE_FROM(pool,dpp) pool_release((pool), (dpp))
#define RELEASE_ALL(pool) (0)
#define TRACK(vp) (vp)
#endif

typedef struct pool_struct pool_t;

typedef struct {
    BYTE        (*reserve)(pool_t * pool, DATA_PTR * data, WORD size);
    BYTE        (*copy)(pool_t * pool, DATA_PTR * data, DATA_PTR value);
    BYTE        (*release)(pool_t * pool, DATA_PTR * data);
} pool_vt;

BYTE pool_reserve(pool_t * pool, DATA_PTR * data, WORD size);
BYTE pool_copy(pool_t * pool, DATA_PTR * data, DATA_PTR value);
BYTE pool_release(pool_t * pool, DATA_PTR * data);

typedef struct pool_struct {
    pool_vt *   vtable;
} pool_t;

extern pool_t * heap_pool;  // explicit reserve/release pool

pool_t * new_temp_pool(pool_t * parent, WORD size);  // create temporary working-memory pool

BYTE audit_reserve(char * _file_, int _line_, pool_t * pool, DATA_PTR * data, WORD size);
BYTE audit_copy(char * _file_, int _line_, pool_t * pool, DATA_PTR * data, DATA_PTR value);
BYTE audit_release(char * _file_, int _line_, pool_t * pool, DATA_PTR * data);
VOID_PTR audit_track(char * _file_, int _line_, pool_t * pool, VOID_PTR address);

int audit_release_all(char * _file_, int _line_, pool_t * pool);
int audit_show_leaks();

#endif // _POOL_H_
