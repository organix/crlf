/*
 * sponsor.c -- actor resource provider/manager
 */
#include <stddef.h>  // for NULL, size_t, et. al.
#include <assert.h>

#include "sponsor.h"
#include "bose.h"
#include "pool.h"
#include "program.h"
#include "object.h"
#include "equiv.h"
#include "print.h"
#include "abcm.h"

#define LOG_ALL // enable all logging
//#define LOG_INFO
//#define LOG_WARN
#include "log.h"

/**  --FIXME--

Translating from a (capability) address to actual actor-reference is a sponsor-specific operation.
I am uncomfortable with the implied exposure of the actor's implementation structure.
How can we avoid that exposure? Does the sponsor become responsible for all the actor and event state?

These responsibilities include:
  * check for binding of actor-scope variables
  * lookup bindings for actor-scope variables
  * update bindings for actor-scope variables
  * lookup an actor's behavior script
  * update an actor's behavior script (become)
  * lookup an actor's address (self)
  * lookup bindings in message contents

For now we'll make these all responsibilities of the event, with help from the sponsor.

**/

BYTE event_has_binding(sponsor_t * sponsor, event_t * event, DATA_PTR name) {
    actor_t * actor = event->actor;
    WORD has = object_has(actor->state, name);
    LOG_TRACE("event_has_binding", has);
    return has;
}

BYTE event_lookup_binding(sponsor_t * sponsor, event_t * event, DATA_PTR name, DATA_PTR * value) {
    actor_t * actor = event->actor;
    *value = v_null;  // default value is `null`
    if (!object_get(actor->state, name, value)) {
        LOG_WARN("event_lookup_binding: undefined variable!", (WORD)name);
        // FIXME: we may want to "throw" an exception here...
    }
    // FIXME: binding are currently only found in the actor's immediate local state, we need to search parent scopes too...
    LOG_TRACE("event_lookup_binding: value =", (WORD)*value);
    return true;  // success
}

BYTE event_update_binding(sponsor_t * sponsor, event_t * event, DATA_PTR name, DATA_PTR value) {
    actor_t * actor = event->actor;
    DATA_PTR state;
    if (!object_add(sponsor, actor->state, name, value, &state)) return false;  // allocation failure!
    if (!RELEASE(&actor->state)) return false;  // reclamation failure!
    actor->state = TRACK(state);
    LOG_LEVEL(LOG_LEVEL_TRACE+1, "event_update_binding: state' =", (WORD)state);
    if (!value_print(state, 1)) return false;  // print failed
    LOG_TRACE("event_update_binding: value =", (WORD)value);
    return true;  // success
}

BYTE event_lookup_behavior(sponsor_t * sponsor, event_t * event, DATA_PTR * behavior) {
    actor_t * actor = event->actor;
    *behavior = actor->behavior;
    LOG_TRACE("event_lookup_behavior: behavior =", (WORD)*behavior);
    return true;  // success
}

BYTE event_update_behavior(sponsor_t * sponsor, event_t * event, DATA_PTR behavior) {
    LOG_WARN("event_update_behavior: BECOME NOT IMPLEMENTED!", (WORD)behavior);
    return false;  // failure!
}

BYTE event_lookup_actor(sponsor_t * sponsor, event_t * event, DATA_PTR * self) {
    actor_t * actor = event->actor;
    *self = actor->capability;
    LOG_TRACE("event_lookup_actor: self =", (WORD)*self);
    return true;  // success
}

BYTE event_lookup_message(sponsor_t * sponsor, event_t * event, DATA_PTR * message) {
    *message = event->message;
    LOG_TRACE("event_lookup_message: message =", (WORD)*message);
    return true;  // success
}

/*
 * resource-bounded sponsor
 */

typedef struct {
    sponsor_t   sponsor;        // super-type member
    WORD        actors;         // actor creation limit
    WORD        events;         // message-send event limit
    pool_t *    work_pool;      // temporary working-memory pool
    actor_t *   actor;          // actor roster
    event_t *   event;          // event queue
    WORD        current;        // current-event index
    DATA_PTR    error;          // error value, or NULL if none
} bounded_sponsor_t;

static actor_t * bounded_sponsor_find_actor(sponsor_t * sponsor, DATA_PTR address) {
    bounded_sponsor_t * THIS = (bounded_sponsor_t *)sponsor;
    LOG_LEVEL(LOG_LEVEL_TRACE+1, "bounded_sponsor_find_actor: address =", (WORD)address);
    //if (!value_print(address, 1)) return NULL;  // print failed!
    parse_t parse;
    if (!value_parse(address, &parse)
    ||  !(parse.type & T_Capability)) {
        LOG_WARN("bounded_sponsor_find_actor: bad address!", (WORD)address);
        return NULL;  // bad address!
    }
    //DUMP_PARSE("bounded_sponsor_find_actor: address", &parse);
    WORD ocap = 0;
    while (parse.value--) {
        ocap <<= 8;
        ocap |= parse.base[--parse.end];
    }
    LOG_LEVEL(LOG_LEVEL_TRACE+1, "bounded_sponsor_find_actor: ocap =", ocap);
    actor_t * actor = &THIS->actor[ocap];
    LOG_LEVEL(LOG_LEVEL_TRACE, "bounded_sponsor_find_actor: actor =", (WORD)actor);
    return actor;
}

static BYTE bounded_sponsor_dispatch(sponsor_t * sponsor) {
    bounded_sponsor_t * THIS = (bounded_sponsor_t *)sponsor;
    if (THIS->events == THIS->current) {
        LOG_INFO("bounded_sponsor_dispatch: work completed.", (WORD)THIS);
        return false;  // work completed.
    }
    WORD current = --THIS->current;
    LOG_LEVEL(LOG_LEVEL_TRACE+1, "bounded_sponsor_dispatch: current =", current);
    event_t * event = &THIS->event[current];
    LOG_LEVEL(LOG_LEVEL_TRACE+1, "bounded_sponsor_dispatch: event =", (WORD)event);
    if (run_actor_script(sponsor, event) != 0) {
        LOG_WARN("bounded_sponsor_dispatch: actor-script execution failed!", (WORD)event);
        return false;  // actor-script execution failed!
    }
    if (!RELEASE(&event->message)) return false;  // reclamation failure!
    return true;  // success!
}

static BYTE bounded_sponsor_create(sponsor_t * sponsor, DATA_PTR state, DATA_PTR behavior, DATA_PTR * address) {
    bounded_sponsor_t * THIS = (bounded_sponsor_t *)sponsor;
    if (THIS->actors < 1) {
        LOG_WARN("bounded_sponsor_create: no more actors!", (WORD)THIS);
        return false;  // no more actors!
    }
    LOG_TRACE("bounded_sponsor_create: state =", (WORD)state);
    if (!value_print(state, 0)) return false;  // print failed!
    LOG_TRACE("bounded_sponsor_create: behavior =", (WORD)behavior);
    if (!value_print(behavior, 0)) return false;  // print failed!
    WORD ocap = --THIS->actors;
    LOG_LEVEL(LOG_LEVEL_TRACE+1, "bounded_sponsor_create: ocap =", ocap);
    actor_t * actor = &THIS->actor[ocap];
    LOG_LEVEL(LOG_LEVEL_TRACE+1, "bounded_sponsor_create: actor =", (WORD)actor);
    actor->capability[0] = octets;      // binary octet data
    actor->capability[1] = n_3;         // size field (3 bytes)
    actor->capability[2] = 0x10;        // capability marker (DLE)
    actor->capability[3] = ocap & 0xFF; // capability (LSB)
    actor->capability[4] = ocap >> 8;   // capability (MSB)
    if (!COPY(&actor->state, state)) return false;  // allocation failure!
    if (!COPY(&actor->behavior, behavior)) return false;  // allocation failure!
    *address = actor->capability;
    LOG_LEVEL(LOG_LEVEL_TRACE+1, "bounded_sponsor_create: address =", (WORD)*address);
    if (!value_print(*address, 0)) return false;  // print failed!
#if 0
    // start with nothing
    DATA_PTR object;
    DATA_PTR result;
    assert(COPY(&object, o_));
    // bind "behavior"
    assert(object_add(sponsor, object, s_behavior, actor->behavior, &result));
    assert(RELEASE(&object));
    object = result;
    // bind "state"
    assert(object_add(sponsor, object, s_state, actor->state, &result));
    assert(RELEASE(&object));
    object = result;
    // bind "actor"
    assert(object_add(sponsor, object, s_actor, actor->capability, &result));
    assert(RELEASE(&object));
    object = result;
    if (!value_print(object, 1)) return false;  // print failed!
    assert(RELEASE(&object));
#endif
    return true;  // success!
}

static BYTE bounded_sponsor_send(sponsor_t * sponsor, DATA_PTR address, DATA_PTR message) {
    bounded_sponsor_t * THIS = (bounded_sponsor_t *)sponsor;
    if (THIS->events < 1) {
        LOG_WARN("bounded_sponsor_send: no more message-send events!", (WORD)THIS);
        return false;  // no more message-send events!
    }
    LOG_TRACE("bounded_sponsor_send: address =", (WORD)address);
    if (!value_print(address, 1)) return false;  // print failed!
    actor_t * actor = bounded_sponsor_find_actor(sponsor, address);
    if (!actor) return false;  // bad actor!
    LOG_TRACE("bounded_sponsor_send: message =", (WORD)message);
    if (!value_print(message, 1)) return false;  // print failed!
    if (value_equiv(address, v_null)) {
        LOG_INFO("bounded_sponsor_send: ignoring message to null.", (WORD)message);
        // FIXME: we may want to catch this case earlier, and avoid evaluating the message expression...
        return true;  // success!
    }
    WORD current = --THIS->events;
    LOG_LEVEL(LOG_LEVEL_TRACE+1, "bounded_sponsor_send: current =", current);
    event_t * event = &THIS->event[current];
    LOG_LEVEL(LOG_LEVEL_TRACE+1, "bounded_sponsor_send: event =", (WORD)event);
    event->actor = actor;
    COPY(&event->message, message);
    return true;  // success!
}

static BYTE bounded_sponsor_become(sponsor_t * sponsor, DATA_PTR behavior) {
    bounded_sponsor_t * THIS = (bounded_sponsor_t *)sponsor;
    if (THIS) return false;  // unimplemented failure
    return true;  // success!
}

static BYTE bounded_sponsor_fail(sponsor_t * sponsor, DATA_PTR error) {
    bounded_sponsor_t * THIS = (bounded_sponsor_t *)sponsor;
    LOG_TRACE("bounded_sponsor_fail: error =", (WORD)error);
    THIS->error = error;
    if (!value_print(error, 0)) return false;  // print failed
    return true;  // success!
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

sponsor_t * new_bounded_sponsor(WORD actors, WORD events, pool_t * work_pool) {
    LOG_TRACE("new_bounded_sponsor: actors =", actors);
    if (actors > 0xFFFF) {  // FIXME: this should be a configurable limit somewhere, but this code depends on it.
        LOG_WARN("new_bounded_sponsor: too many actors!", 0xFFFF);
        return NULL;  // too many actors!
    }
    LOG_TRACE("new_bounded_sponsor: events =", events);
    if (events > 0xFFFF) {  // FIXME: this should be a configurable limit somewhere, but this code depends on it.
        LOG_WARN("new_bounded_sponsor: too many events!", 0xFFFF);
        return NULL;  // too many events!
    }
    DATA_PTR data = NULL;
    if (!pool_reserve(heap_pool, &data, sizeof(bounded_sponsor_t))) return NULL;  // allocation failure!
    bounded_sponsor_t * THIS = (bounded_sponsor_t *)data;
    THIS->actors = actors;
    THIS->events = events;
    THIS->work_pool = work_pool;
    if (actors && !pool_reserve(heap_pool, (DATA_PTR *)&THIS->actor, sizeof(actor_t) * actors)) return NULL;  // allocation failure!
    if (events && !pool_reserve(heap_pool, (DATA_PTR *)&THIS->event, sizeof(event_t) * events)) return NULL;  // allocation failure!
    THIS->current = events;
    THIS->error = NULL;
    THIS->sponsor.dispatch = bounded_sponsor_dispatch;
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

inline BYTE sponsor_dispatch(sponsor_t * sponsor) {
    return sponsor->dispatch(sponsor);
}

inline BYTE sponsor_create(sponsor_t * sponsor, DATA_PTR state, DATA_PTR behavior, DATA_PTR * address) {
    return sponsor->create(sponsor, state, behavior, address);
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
    sponsor_t * sponsor;        // sponsor for resources
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

static void record_allocation(char * _file_, int _line_, sponsor_t * sponsor, DATA_PTR address, WORD size) {
    assert(audit_index < MAX_AUDIT);
    alloc_audit_t * history = &audit_history[audit_index++];
    history->sponsor = sponsor;
    history->address = address;
    history->size = size;
    history->reserve._file_ = _file_;
    history->reserve._line_ = _line_;
    history->release._file_ = NULL;
    history->release._line_ = 0;
}

BYTE audit_reserve(char * _file_, int _line_, sponsor_t * sponsor, DATA_PTR * data, WORD size) {
    BYTE ok = sponsor_reserve(sponsor, data, size);
    if (ok) {
        record_allocation(_file_, _line_, sponsor, *data, size);
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

BYTE audit_copy(char * _file_, int _line_, sponsor_t * sponsor, DATA_PTR * data, DATA_PTR value) {
    BYTE ok = sponsor_copy(sponsor, data, value);
    if (ok) {
        WORD size = value_size(*data);
        record_allocation(_file_, _line_, sponsor, *data, size);
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

DATA_PTR audit_track(char * _file_, int _line_, sponsor_t * sponsor, DATA_PTR address) {
    /* find most-recent allocation of address */
    int index = audit_index;
    while (index > 0) {
        alloc_audit_t * history = &audit_history[--index];
        if (history->address == address) {
            assert(history->sponsor == sponsor);  // make sure we have the same sponsor!
            history->reserve._file_ = _file_;  // update source file name
            history->reserve._line_ = _line_;  // update source line number
            return address;  // found it!
        }
    }
    //LOG_WARN("audit_track: no allocation @", (WORD)address);  // FIXME: we want to show where the TRACK was...
    log_event(_file_, _line_, LOG_LEVEL_WARN, "audit_track: no allocation @", (WORD)address);
    return address;
}


#include <stdio.h>  // FIXME!
int audit_show_leaks() {
    int count = 0;
    WORD total = 0;
#if AUDIT_ALLOCATION
    LOG_INFO("audit_show_leaks: allocations", (WORD)audit_index);
    for (int index = 0; index < audit_index; ++index) {
        alloc_audit_t * history = &audit_history[index];
        if (history->release._file_ == NULL) {
            fprintf(stdout, "LEAK! %p[%d] from %p %s:%d\n",
                history->address, (int)history->size, history->sponsor, history->reserve._file_, history->reserve._line_);
            ++count;
        }
        total += history->size;
    }
    LOG_INFO("audit_show_leaks: total size", total);
    if (count == 0) {  // if there were no leaks...
        audit_index = 0;  // ...clear the audit history and start again.
    }
    fprintf(stdout, "audit_show_leaks: leaks found = %d\n", count);
    fflush(stdout);
#else
    /* can't check for leaks if we're not auditing! */
#endif
    return count;
}
