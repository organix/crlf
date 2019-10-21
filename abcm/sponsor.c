/*
 * sponsor.c -- actor resource provider/manager
 */
#include <stddef.h>  // for NULL, size_t, et. al.
#include <assert.h>

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

/*
 * allocation auditing support
 */

typedef struct {
    DATA_PTR    address;        // memory address
    WORD        size;           // allocation size
    struct {
        char *      _file_;         // source file name
        int         _line_;         // source line number
    }           reserve;        // reserve information
    struct {
        char *      _file_;         // source file name
        int         _line_;         // source line number
    }           release;        // release information
} alloc_audit_t;

#define MAX_AUDIT (1024)
static alloc_audit_t audit_history[MAX_AUDIT];
static int audit_index = 0;

static WORD value_size(DATA_PTR value) {
    parse_t parse = {
        .base = value,
        .size = MAX_WORD,  // don't know how big value will be
        .start = 0
    };
    assert(parse_value(&parse));
    WORD size = parse.end - parse.start;  // parse_value determines the span of the value
    return size;
}

BYTE audit_reserve(char * _file_, int _line_, sponsor_t * sponsor, DATA_PTR * data, WORD size) {
    assert(audit_index < MAX_AUDIT);
    BYTE ok = sponsor_reserve(sponsor, data, size);
    if (ok) {
        alloc_audit_t * history = &audit_history[audit_index++];
        history->address = *data;
        history->size = size;
        history->reserve._file_ = _file_;
        history->reserve._line_ = _line_;
    }
    return ok;
}

BYTE audit_copy(char * _file_, int _line_, sponsor_t * sponsor, DATA_PTR * data, DATA_PTR value) {
    assert(audit_index < MAX_AUDIT);
    BYTE ok = sponsor_copy(sponsor, data, value);
    if (ok) {
        alloc_audit_t * history = &audit_history[audit_index++];
        history->address = *data;
        history->size = value_size(*data);
        history->reserve._file_ = _file_;
        history->reserve._line_ = _line_;
    }
    return ok;
}

BYTE audit_release(char * _file_, int _line_, sponsor_t * sponsor, DATA_PTR * data) {
    DATA_PTR address = *data;
    BYTE ok = sponsor_release(sponsor, data);  // (*data == NULL) on return from sponsor_release!
    if (ok) {
        /* find most-recent allocation of address */
        int index = audit_index;
        while (index > 0) {
            alloc_audit_t * history = &audit_history[--index];
            if (history->address == address) {
                history->release._file_ = _file_;
                history->release._line_ = _line_;
                return true;  // found it!
            }
        }
        LOG_WARN("audit_release: no allocation @", (WORD)address);
        return false;
    }
    return ok;
}

#include <stdio.h>  // FIXME!
int audit_show_leaks() {
    int count = 0;
#if AUDIT_ALLOCATION
    LOG_INFO("audit_show_leaks: allocations", (WORD)audit_index);
    for (int index = 0; index < audit_index; ++index) {
        alloc_audit_t * history = &audit_history[index];
        if (history->release._file_ == NULL) {
            fprintf(stdout, "LEAK! %p[%d] %s:%d\n",
                history->address, (int)history->size, history->reserve._file_, history->reserve._line_);
            ++count;
        }
    }
    fprintf(stdout, "audit_show_leaks: %d leaks found.\n", count);
    fflush(stdout);
#else
    /* can't check for leaks if we're not auditing! */
#endif
    return count;
}
