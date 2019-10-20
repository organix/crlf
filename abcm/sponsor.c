/*
 * sponsor.c -- actor resource provider/manager
 */
#include <stddef.h>  // for NULL, size_t, et. al.
//#include <assert.h>

#include "sponsor.h"
#include "bose.h"
#include "pool.h"

#define LOG_ALL // enable all logging
//#define LOG_INFO
//#define LOG_WARN
#include "log.h"

/*
 * resource-bounded sponsor
 */

typedef struct {
    sponsor_t   sponsor;        // super-type member
    DATA_PTR    actors;         // actor creation limit
    DATA_PTR    events;         // message-send event limit
    pool_t *    work_pool;      // temporary working-memory pool
} bounded_sponsor_t;

static BYTE bounded_sponsor_create(sponsor_t * sponsor, DATA_PTR state, DATA_PTR behavior) {
    bounded_sponsor_t * THIS = (bounded_sponsor_t *)sponsor;
    if (THIS) return false;  // unimplemented failure
    return true;
}

static BYTE bounded_sponsor_send(sponsor_t * sponsor, DATA_PTR address, DATA_PTR message) {
    bounded_sponsor_t * THIS = (bounded_sponsor_t *)sponsor;
    if (THIS) return false;  // unimplemented failure
    return true;
}

static BYTE bounded_sponsor_become(sponsor_t * sponsor, DATA_PTR behavior) {
    bounded_sponsor_t * THIS = (bounded_sponsor_t *)sponsor;
    if (THIS) return false;  // unimplemented failure
    return true;
}

static BYTE bounded_sponsor_fail(sponsor_t * sponsor, DATA_PTR error) {
    bounded_sponsor_t * THIS = (bounded_sponsor_t *)sponsor;
    if (THIS) return false;  // unimplemented failure
    return true;
}

static BYTE bounded_sponsor_reserve(sponsor_t * sponsor, DATA_PTR * data, WORD size) {
    bounded_sponsor_t * THIS = (bounded_sponsor_t *)sponsor;
    return pool_reserve(THIS->work_pool, data, size);
}

static BYTE bounded_sponsor_share(sponsor_t * sponsor, DATA_PTR * data) {
    bounded_sponsor_t * THIS = (bounded_sponsor_t *)sponsor;
    return pool_share(THIS->work_pool, data);
}

static BYTE bounded_sponsor_copy(sponsor_t * sponsor, DATA_PTR * data, DATA_PTR value) {
    bounded_sponsor_t * THIS = (bounded_sponsor_t *)sponsor;
    return pool_copy(THIS->work_pool, data, value);
}

static BYTE bounded_sponsor_release(sponsor_t * sponsor, DATA_PTR * data) {
    bounded_sponsor_t * THIS = (bounded_sponsor_t *)sponsor;
    return pool_release(THIS->work_pool, data);
}

sponsor_t * new_bounded_sponsor(DATA_PTR actors, DATA_PTR events, pool_t * work_pool) {
    DATA_PTR data = NULL;
    if (!pool_reserve(heap_pool, &data, sizeof(bounded_sponsor_t))) return NULL;  // allocation failure!
    bounded_sponsor_t * THIS = (bounded_sponsor_t *)data;
    if(!pool_copy(heap_pool, &THIS->actors, actors)) return NULL;  // copy failure!
    if(!pool_copy(heap_pool, &THIS->events, events)) return NULL;  // copy failure!
    THIS->work_pool = work_pool;
    THIS->sponsor.create = bounded_sponsor_create;
    THIS->sponsor.send = bounded_sponsor_send;
    THIS->sponsor.become = bounded_sponsor_become;
    THIS->sponsor.fail = bounded_sponsor_fail;
    THIS->sponsor.reserve = bounded_sponsor_reserve;
    THIS->sponsor.share = bounded_sponsor_share;
    THIS->sponsor.copy = bounded_sponsor_copy;
    THIS->sponsor.release = bounded_sponsor_release;
    return (sponsor_t *)THIS;
}

/*
 * polymorphic dispatch functions
 */
inline BYTE sponsor_create(sponsor_t * sponsor, DATA_PTR state, DATA_PTR behavior) {
    return sponsor->create(sponsor, state, behavior);
}

inline BYTE sponsor_send(sponsor_t * sponsor, DATA_PTR address, DATA_PTR message) {
    return sponsor->send(sponsor, address, message);
}

inline BYTE sponsor_become(sponsor_t * sponsor, DATA_PTR behavior) {
    return sponsor->become(sponsor, behavior);
}

inline BYTE sponsor_fail(sponsor_t * sponsor, DATA_PTR error) {
    return sponsor->fail(sponsor, error);
}

inline BYTE sponsor_reserve(sponsor_t * sponsor, DATA_PTR * data, WORD size) {
    return sponsor->reserve(sponsor, data, size);
}

inline BYTE sponsor_share(sponsor_t * sponsor, DATA_PTR * data) {
    return sponsor->share(sponsor, data);
}

inline BYTE sponsor_copy(sponsor_t * sponsor, DATA_PTR * data, DATA_PTR value) {
    return sponsor->copy(sponsor, data, value);
}

inline BYTE sponsor_release(sponsor_t * sponsor, DATA_PTR * data) {
    return sponsor->release(sponsor, data);
}
