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

static pool_vt heap_pool_vtable = {
    .reserve = heap_pool_reserve,
    .copy = generic_pool_copy,
    .release = heap_pool_release,
};
static heap_pool_t heap_pool_instance = {
    .pool.vtable = &heap_pool_vtable
};
pool_t * heap_pool = &heap_pool_instance.pool;

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

static pool_vt temp_pool_vtable = {
    .reserve = temp_pool_reserve,
    .copy = generic_pool_copy,
    .release = temp_pool_release,
};

pool_t * new_temp_pool(pool_t * parent, WORD size) {
    temp_pool_t * THIS;
    if (!RESERVE_FROM(parent, (DATA_PTR *)&THIS, sizeof(temp_pool_t) + size)) {
        LOG_WARN("new_temp_pool: could not reserve memory for pool", size);
    }
    THIS->pool.vtable = &temp_pool_vtable;
    THIS->base = (DATA_PTR)(THIS + 1);  // managed memory starts after pool structure
    THIS->size = size;
    THIS->offset = 0;
    LOG_DEBUG("new_temp_pool: created", size);
    return &THIS->pool;
}

/*
 * polymorphic method calls
 */

inline BYTE pool_reserve(pool_t * pool, DATA_PTR * data, WORD size) {
    return pool->vtable->reserve(pool, data, size);
}

inline BYTE pool_copy(pool_t * pool, DATA_PTR * data, DATA_PTR value) {
    return pool->vtable->copy(pool, data, value);
}

inline BYTE pool_release(pool_t * pool, DATA_PTR * data) {
    return pool->vtable->release(pool, data);
}

/*
 * allocation auditing support
 */

#include <stdio.h>  // FIXME!

typedef struct {
    pool_t *    pool;           // allocation pool
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

static void record_allocation(char * _file_, int _line_, pool_t * pool, DATA_PTR address, WORD size) {
    assert(audit_index < MAX_AUDIT);
    alloc_audit_t * history = &audit_history[audit_index++];
    history->pool = pool;
    history->address = address;
    history->size = size;
    history->reserve._file_ = _file_;
    history->reserve._line_ = _line_;
    history->release._file_ = NULL;
    history->release._line_ = 0;
}

BYTE audit_reserve(char * _file_, int _line_, pool_t * pool, DATA_PTR * data, WORD size) {
    BYTE ok = pool_reserve(pool, data, size);
    if (ok) {
        record_allocation(_file_, _line_, pool, *data, size);
    }
    return ok;
}

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

BYTE audit_copy(char * _file_, int _line_, pool_t * pool, DATA_PTR * data, DATA_PTR value) {
    BYTE ok = pool_copy(pool, data, value);
    if (ok) {
        WORD size = value_size(*data);
        record_allocation(_file_, _line_, pool, *data, size);
    }
    return ok;
}

BYTE audit_release(char * _file_, int _line_, pool_t * pool, DATA_PTR * data) {
    DATA_PTR address = *data;
    /* find most-recent allocation of address */
    int index = audit_index;
    while (index > 0) {
        alloc_audit_t * history = &audit_history[--index];
        if (history->address == address) {
            if (history->pool != pool) {
                LOG_WARN("audit_release: WRONG POOL!", (WORD)pool);
                IF_WARN(fprintf(stdout, "%p[%d] from %p %s:%d -> %s:%d\n",
                    history->address, (int)history->size, history->pool,
                    history->reserve._file_, history->reserve._line_,
                    _file_, _line_));
                return false;
            }
            history->release._file_ = _file_;
            history->release._line_ = _line_;
            memset(address, null, history->size);  // FIXME: should memory scribbling be optional?
            BYTE ok = pool_release(pool, data);  // (*data == NULL) on return from pool_release!
            return ok;  // found it!
        }
    }
    log_event(_file_, _line_, LOG_LEVEL_WARN, "audit_release: NO ALLOCATION @", (WORD)address);
    //IF_WARN(fprintf(stdout, "%p[?] from %p ?:? -> %s:%d\n", address, pool, _file_, _line_));
    return false;
}

VOID_PTR audit_track(char * _file_, int _line_, pool_t * pool, VOID_PTR address) {
    /* find most-recent allocation of address */
    int index = audit_index;
    while (index > 0) {
        alloc_audit_t * history = &audit_history[--index];
        if (history->address == address) {
            if (history->pool != pool) {
                LOG_WARN("audit_track: WRONG POOL!", (WORD)pool);
                IF_WARN(fprintf(stdout, "%p[%d] from %p %s:%d -> %s:%d\n",
                    history->address, (int)history->size, history->pool,
                    history->reserve._file_, history->reserve._line_,
                    _file_, _line_));
                return address;  // leave history unchanged!
            }
            history->reserve._file_ = _file_;  // update source file name
            history->reserve._line_ = _line_;  // update source line number
            return address;  // found it!
        }
    }
    log_event(_file_, _line_, LOG_LEVEL_WARN, "audit_track: NO ALLOCATION @", (WORD)address);
    //IF_WARN(fprintf(stdout, "%p[?] from %p ?:? -> %s:%d\n", address, pool, _file_, _line_));
    return address;
}

int audit_release_all(char * _file_, int _line_, pool_t * pool) {  // bulk-remove all allocation in `pool`
    LOG_DEBUG("audit_release_all: pool", (WORD)pool);
    WORD count = 0;
#if AUDIT_ALLOCATION
    WORD total = 0;
    WORD gone = 0;
    LOG_TRACE("audit_release_all: allocations", (WORD)audit_index);
    for (int index = 0; index < audit_index; ++index) {
        alloc_audit_t * history = &audit_history[index];
        if (history->pool == pool) {
            if (history->release._file_ == NULL) {
                IF_DEBUG(fprintf(stdout, "FREE: %p[%d] from %p %s:%d\n",
                    history->address, (int)history->size, history->pool,
                    history->reserve._file_, history->reserve._line_));
                history->release._file_ = _file_;
                history->release._line_ = _line_;
            } else {
                IF_WARN(fprintf(stdout, "GONE! %p[%d] from %p %s:%d -> %s:%d\n",
                    history->address, (int)history->size, history->pool,
                    history->reserve._file_, history->reserve._line_,
                    history->release._file_, history->release._line_));
                gone += history->size;
            }
            history->pool = NULL;  // malloc reuse can cause multiple counting of deadpools...
            ++count;
            total += history->size;
        }
    }
    if (gone) {
        LOG_WARN("audit_release_all: size already gone", gone);
    }
    if (total) {
        LOG_WARN("audit_release_all: total size released", total);
    }
    LOG_INFO("audit_release_all: allocations released", count);
#else
    /* can't bulk-remove if we're not auditing! */
#endif
    return count;
}

int audit_check_leaks() {
    WORD count = 0;
#if AUDIT_ALLOCATION
    WORD total = 0;
    WORD leaked = 0;
    LOG_INFO("audit_show_leaks: allocations", (WORD)audit_index);
    for (int index = 0; index < audit_index; ++index) {
        alloc_audit_t * history = &audit_history[index];
        if (history->release._file_ == NULL) {
            IF_WARN(fprintf(stdout, "LEAK! %p[%d] from %p %s:%d\n",
                history->address, (int)history->size, history->pool,
                history->reserve._file_, history->reserve._line_));
            ++count;
            leaked += history->size;
        }
        total += history->size;
    }
    LOG_INFO("audit_show_leaks: total size", total);
    if (count == 0) {  // if there were no leaks...
        audit_index = 0;  // ...clear the audit history and start again.
    } else {
        LOG_WARN("audit_show_leaks: total leaked", leaked);
    }
    LOG_INFO("audit_show_leaks: leaks found", count);
#else
    /* can't check for leaks if we're not auditing! */
#endif
    return count;
}
