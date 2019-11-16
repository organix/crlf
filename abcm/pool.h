/*
 * pool.h -- memory management pools
 */
#ifndef _POOL_H_
#define _POOL_H_

#include "bose.h"

#define AUDIT_ALLOCATION 1 /* track reserve/release calls to check for leaks */
#define FULL_AUDIT_DUMP 0 /* report ALL allocations, not just the leaks... */
#define SCRIBBLE_ON_FREE 1 /* write null (0xFF) on memory before releasing it */
#define KEEP_POOL_METRICS 1 /* track memory usage metrics across pools */

#define STATIC_TEMP_POOL_SIZE (1 << 12)  /* if this is 0, static temp_pool is not used. */

#if AUDIT_ALLOCATION
#define RESERVE_FROM(pool,dpp,size) audit_reserve(__FILE__, __LINE__, (pool), (dpp), (size))
#define COPY_INTO(pool,to_dpp,from_dp) audit_copy(__FILE__, __LINE__, (pool), (to_dpp), (from_dp))
#define RELEASE_FROM(pool,dpp) audit_release(__FILE__, __LINE__, (pool), (dpp))
#define RELEASE_ALL(pool) audit_release_all(__FILE__, __LINE__, (pool))
#define TRACK_IN(pool,vp) audit_track(__FILE__, __LINE__, (pool), (vp))
#else
#define RESERVE_FROM(pool,dpp,size) pool_reserve((pool), (dpp), (size))
#define COPY_INTO(pool,to_dpp,from_dp) pool_copy((pool), (to_dpp), (from_dp))
#define RELEASE_FROM(pool,dpp) pool_release((pool), (dpp))
#define RELEASE_ALL(pool) (0)
#define TRACK_IN(pool,vp) (vp)
#endif
#define RESERVE(dpp,size) RESERVE_FROM(SPONSOR_POOL(sponsor),dpp,size)
#define COPY(to_dpp,from_dp) COPY_INTO(SPONSOR_POOL(sponsor),to_dpp,from_dp)
#define RELEASE(dpp) RELEASE_FROM(SPONSOR_POOL(sponsor),dpp)
#define TRACK(vp) TRACK_IN(SPONSOR_POOL(sponsor),vp)

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

extern pool_t * heap_pool;  // globally-available explicit reserve/release pool

pool_t * new_ref_pool(pool_t * parent/*, WORD size*/);  // create reference-counted memory pool
#if STATIC_TEMP_POOL_SIZE
//extern pool_t * temp_pool;  // globally-available temporary working-memory pool
pool_t * clear_temp_pool();  // reset static temp_pool to empty-state
#else
pool_t * new_temp_pool(pool_t * parent, WORD size);  // create temporary working-memory pool
#endif

BYTE audit_reserve(char * _file_, int _line_, pool_t * pool, DATA_PTR * data, WORD size);
BYTE audit_copy(char * _file_, int _line_, pool_t * pool, DATA_PTR * data, DATA_PTR value);
BYTE audit_release(char * _file_, int _line_, pool_t * pool, DATA_PTR * data);
VOID_PTR audit_track(char * _file_, int _line_, pool_t * pool, VOID_PTR address);

int audit_release_all(char * _file_, int _line_, pool_t * pool);
int audit_check_leaks();

void show_pool_metrics();

#endif // _POOL_H_
