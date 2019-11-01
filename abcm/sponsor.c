/*
 * sponsor.c -- actor resource provider/manager
 */
#include <stddef.h>  // for NULL, size_t, et. al.
#include <string.h>  // for memcpy, et. al.
#include <assert.h>

#include "sponsor.h"
#include "bose.h"
#include "pool.h"
#include "event.h"
#include "program.h"
#include "object.h"
#include "equiv.h"
#include "print.h"
#include "abcm.h"

#define LOG_ALL // enable all logging
//#define LOG_INFO
//#define LOG_WARN
#include "log.h"

sponsor_t * sponsor;  // WE DECLARE A GLOBAL SPONSOR TO AVOID THREADING IT THROUGH ALL OTHER CALLS...

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
  * manage message-dispatch effects

For now we'll make these all responsibilities of the event, with help from the sponsor.

**/

/*
 * actor-message delivery event
 */

BYTE event_lookup_behavior(sponsor_t * sponsor, event_t * event, DATA_PTR * behavior) {
    actor_t * actor = event->actor;
    LOG_TRACE("event_lookup_behavior: state =", (WORD)actor->scope.state);
    IF_TRACE(value_print(actor->scope.state, 1));
    *behavior = actor->behavior;
    LOG_DEBUG("event_lookup_behavior: behavior =", (WORD)*behavior);
    return true;  // success
}

BYTE event_update_behavior(sponsor_t * sponsor, event_t * event, DATA_PTR behavior) {
    LOG_TRACE("event_update_behavior: behavior =", (WORD)behavior);
    event->effect.behavior = behavior;
    IF_TRACE(value_print(behavior, 0));
    return true;  // success
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

BYTE event_lookup_scope(sponsor_t * sponsor, event_t * event, scope_t ** scope) {
    *scope = &event->effect.scope;
    LOG_TRACE("event_lookup_scope: scope =", (WORD)*scope);
    return true;  // success
}

BYTE event_init_effects(sponsor_t * sponsor, event_t * event, WORD actors, WORD events) {
    LOG_TRACE("event_init_effects: event =", (WORD)event);
    //actor_t * actor = event->actor;
    event->effect.behavior = NULL;
    event->effect.actors = actors;
    event->effect.events = events;
    event->effect.scope.parent = &event->actor->scope;
    if (!COPY(&event->effect.scope.state, o_)) return false;  // allocation failure!
    LOG_DEBUG("event_init_effects: state =", (WORD)event->effect.scope.state);
    IF_TRACE(value_print(event->effect.scope.state, 1));
    event->effect.error = NULL;
    return true;  // success
}

BYTE event_apply_effects(sponsor_t * sponsor, event_t * event) {
    LOG_TRACE("event_apply_effects: event =", (WORD)event);
    if (event->effect.error) {
        LOG_WARN("event_apply_effects: can't apply error effect!", (WORD)event->effect.error);
        return false;  // apply failed!
    }
    actor_t * actor = event->actor;
    DATA_PTR state;
    if (!object_concat(actor->scope.state, event->effect.scope.state, &state)) {
        LOG_WARN("event_apply_effects: state merge failed!", (WORD)&event->effect.scope);
        return false;  // state merge failed!
    }
    if (!RELEASE(&actor->scope.state)) return false;  // reclamation failure!
    actor->scope.state = TRACK(state);
    LOG_DEBUG("event_apply_effects: state =", (WORD)event->effect.scope.state);
    IF_TRACE(value_print(event->effect.scope.state, 1));
    if (event->effect.behavior) {
        LOG_DEBUG("event_apply_effects: becoming", (WORD)event->effect.behavior);
        if (!RELEASE(&actor->behavior)) return false;  // reclamation failure!
        if (!COPY(&actor->behavior, event->effect.behavior)) return false;  // allocation failure!
        // event->effect.behavior is either static or an alias, so don't RELEASE it....
    }
    return true;  // success
}

BYTE event_revert_effects(sponsor_t * sponsor, event_t * event) {
    LOG_TRACE("event_revert_effects: event =", (WORD)event);
    if (event->effect.error) {
        if (!RELEASE(&event->effect.error)) return false;  // reclamation failure!
    }
    // FIXME: reclaim new actors and events...
    // event->effect.behavior is either static or an alias, so don't RELEASE it....
    return true;  // success
}

/*
 * generic sponsor methods
 */

typedef struct {
    pool_t      pool;           // super-type member
    sponsor_t * sponsor;        // allocation sponsor
} sponsor_pool_t;

static BYTE sponsor_pool_reserve(pool_t * pool, DATA_PTR * data, WORD size) {
    BYTE ok = RESERVE(data, size);
    if (ok) {
        LOG_DEBUG("sponsor_pool_reserve: data @", (WORD)*data);
    }
    return ok;
}

static BYTE generic_sponsor_temp_pool(sponsor_t * sponsor, WORD size, sponsor_t ** child) {
    sponsor_pool_t sponsor_pool = {
        .sponsor = sponsor,
        .pool.reserve = sponsor_pool_reserve,
    };
    pool_t * pool = new_temp_pool(&sponsor_pool.pool, size);
    LOG_TRACE("generic_sponsor_temp_pool: pool =", (WORD)pool);
    if (!pool) return false;  // failed to create temp pool!
    *child = new_pool_sponsor(sponsor, pool);
    LOG_DEBUG("generic_sponsor_temp_pool: child =", (WORD)*child);
    return (*child != NULL);
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
} bounded_sponsor_t;

static actor_t * bounded_sponsor_find_actor(sponsor_t * sponsor, DATA_PTR address) {
    bounded_sponsor_t * THIS = (bounded_sponsor_t *)sponsor;
    LOG_LEVEL(LOG_LEVEL_TRACE+1, "bounded_sponsor_find_actor: address =", (WORD)address);
    IF_NONE(value_print(address, 1));
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
    if (!event_init_effects(sponsor, event, THIS->actors, THIS->events)) return false;  // init failed!
    LOG_DEBUG("bounded_sponsor_dispatch: message =", (WORD)event->message);
    IF_DEBUG(value_print(event->message, 1));
    actor_t * actor = event->actor;
    LOG_DEBUG("bounded_sponsor_dispatch: actor =", (WORD)actor);
    IF_DEBUG(value_print(actor->capability, 1));
    if (run_actor_script(sponsor, event) == 0) {
        LOG_DEBUG("bounded_sponsor_dispatch: new actors", (event->effect.actors - THIS->actors));
        LOG_DEBUG("bounded_sponsor_dispatch: new events", (event->effect.events - THIS->events));
        if (!event_apply_effects(sponsor, event)) {
            LOG_WARN("bounded_sponsor_dispatch: failed to apply effects!", (WORD)event);
            return false;  // effects failed!
        }
        if (!RELEASE(&event->message)) return false;  // reclamation failure!
    } else {
        if (event->effect.error) {
            LOG_WARN("bounded_sponsor_dispatch: caught actor FAIL!", (WORD)event->effect.error);
            IF_WARN(value_print(event->effect.error, 1));
        }
        LOG_WARN("bounded_sponsor_dispatch: actor-script execution failed!", (WORD)event);
        if (!event_revert_effects(sponsor, event)) {
            LOG_WARN("bounded_sponsor_dispatch: failed to revert effects!", (WORD)event);
            return false;  // effects failed!
        }
        if (!RELEASE(&event->message)) return false;  // reclamation failure!
        return false;  // execution failed!
    }
    LOG_DEBUG("bounded_sponsor_dispatch: event completed.", (WORD)event);
    return true;  // success!
}

static BYTE bounded_sponsor_create(sponsor_t * sponsor, scope_t * scope, DATA_PTR state, DATA_PTR behavior, DATA_PTR * address) {
    bounded_sponsor_t * THIS = (bounded_sponsor_t *)sponsor;
    if (THIS->actors < 1) {
        LOG_WARN("bounded_sponsor_create: no more actors!", (WORD)THIS);
        return false;  // no more actors!
    }
    LOG_TRACE("bounded_sponsor_create: state =", (WORD)state);
    IF_TRACE(value_print(state, 0));
    LOG_TRACE("bounded_sponsor_create: behavior =", (WORD)behavior);
    IF_TRACE(value_print(behavior, 0));
    WORD ocap = --THIS->actors;
    LOG_LEVEL(LOG_LEVEL_TRACE+1, "bounded_sponsor_create: ocap =", ocap);
    actor_t * actor = &THIS->actor[ocap];
    LOG_LEVEL(LOG_LEVEL_TRACE+1, "bounded_sponsor_create: actor =", (WORD)actor);
    actor->capability[0] = octets;      // binary octet data
    actor->capability[1] = n_3;         // size field (3 bytes)
    actor->capability[2] = 0x10;        // capability marker (DLE)
    actor->capability[3] = ocap & 0xFF; // capability (LSB)
    actor->capability[4] = ocap >> 8;   // capability (MSB)
    actor->capability[5] = null;        // --unused--
    actor->capability[6] = null;        // --unused--
    actor->capability[7] = null;        // --unused--
    // chain scope to parent
    actor->scope.parent = scope;
    if (!COPY(&actor->scope.state, state)) return false;  // allocation failure!
    if (!COPY(&actor->behavior, behavior)) return false;  // allocation failure!
    *address = actor->capability;
    LOG_DEBUG("bounded_sponsor_create: address =", (WORD)*address);
    IF_TRACE(value_print(*address, 0));
    return true;  // success!
}

static BYTE bounded_sponsor_send(sponsor_t * sponsor, DATA_PTR address, DATA_PTR message) {
    bounded_sponsor_t * THIS = (bounded_sponsor_t *)sponsor;
    if (THIS->events < 1) {
        LOG_WARN("bounded_sponsor_send: no more message-send events!", (WORD)THIS);
        return false;  // no more message-send events!
    }
    LOG_DEBUG("bounded_sponsor_send: address =", (WORD)address);
    IF_TRACE(value_print(address, 1));
    actor_t * actor = bounded_sponsor_find_actor(sponsor, address);
    if (!actor) return false;  // bad actor!
    LOG_DEBUG("bounded_sponsor_send: message =", (WORD)message);
    IF_TRACE(value_print(message, 1));
    if (value_equiv(address, v_null)) {
        LOG_WARN("bounded_sponsor_send: ignoring message to null.", (WORD)message);
        // FIXME: we may want to catch this case earlier, and avoid evaluating the message expression...
        return true;  // success!
    }
    WORD current = --THIS->events;
    LOG_LEVEL(LOG_LEVEL_TRACE+1, "bounded_sponsor_send: current =", current);
    event_t * event = &THIS->event[current];
    LOG_LEVEL(LOG_LEVEL_TRACE+0, "bounded_sponsor_send: event =", (WORD)event);
    event->actor = actor;
    COPY(&event->message, message);
    return true;  // success!
}

static BYTE bounded_sponsor_become(sponsor_t * sponsor, DATA_PTR behavior) {
    bounded_sponsor_t * THIS = (bounded_sponsor_t *)sponsor;
    event_t * event = &THIS->event[THIS->current];
    LOG_LEVEL(LOG_LEVEL_TRACE+0, "bounded_sponsor_become: event =", (WORD)event);
    if (!event_update_behavior(sponsor, event, behavior)) return false;  // update failed!
    return true;  // success!
}

static BYTE bounded_sponsor_fail(sponsor_t * sponsor, event_t * event, DATA_PTR error) {
    //bounded_sponsor_t * THIS = (bounded_sponsor_t *)sponsor;
    LOG_DEBUG("bounded_sponsor_fail: error =", (WORD)error);
    event->effect.error = error;
    IF_TRACE(value_print(error, 0));
    return true;  // success!
}

static BYTE bounded_sponsor_reserve(sponsor_t * sponsor, DATA_PTR * data, WORD size) {
    bounded_sponsor_t * THIS = (bounded_sponsor_t *)sponsor;
    return pool_reserve(THIS->work_pool, data, size);
}

static BYTE bounded_sponsor_copy(sponsor_t * sponsor, DATA_PTR * data, DATA_PTR value) {
    bounded_sponsor_t * THIS = (bounded_sponsor_t *)sponsor;
    return pool_copy(THIS->work_pool, data, value);
}

static BYTE bounded_sponsor_release(sponsor_t * sponsor, DATA_PTR * data) {
    bounded_sponsor_t * THIS = (bounded_sponsor_t *)sponsor;
    return pool_release(THIS->work_pool, data);
}

static BYTE bounded_sponsor_destroy(sponsor_t * sponsor) {
    LOG_WARN("bounded_sponsor_destroy: NOT IMPLEMENTED!", (WORD)sponsor);
    return false;  // failure!
}

sponsor_t * new_bounded_sponsor(WORD actors, WORD events, pool_t * pool) {
    LOG_DEBUG("new_bounded_sponsor: actors =", actors);
    if (actors > 0xFFFF) {  // FIXME: this should be a configurable limit somewhere, but this code depends on it.
        LOG_WARN("new_bounded_sponsor: too many actors!", 0xFFFF);
        return NULL;  // too many actors!
    }
    LOG_DEBUG("new_bounded_sponsor: events =", events);
    if (events > 0xFFFF) {  // FIXME: this should be a configurable limit somewhere, but this code depends on it.
        LOG_WARN("new_bounded_sponsor: too many events!", 0xFFFF);
        return NULL;  // too many events!
    }
    DATA_PTR data = NULL;
    if (!pool_reserve(heap_pool, &data, sizeof(bounded_sponsor_t))) return NULL;  // allocation failure!
    bounded_sponsor_t * THIS = (bounded_sponsor_t *)data;
    THIS->actors = actors;
    THIS->events = events;
    THIS->work_pool = pool;
    if (actors && !pool_reserve(heap_pool, (DATA_PTR *)&THIS->actor, sizeof(actor_t) * actors)) return NULL;  // allocation failure!
    if (events && !pool_reserve(heap_pool, (DATA_PTR *)&THIS->event, sizeof(event_t) * events)) return NULL;  // allocation failure!
    THIS->current = events;
    THIS->sponsor.dispatch = bounded_sponsor_dispatch;
    THIS->sponsor.create = bounded_sponsor_create;
    THIS->sponsor.send = bounded_sponsor_send;
    THIS->sponsor.become = bounded_sponsor_become;
    THIS->sponsor.fail = bounded_sponsor_fail;
    THIS->sponsor.reserve = bounded_sponsor_reserve;
    THIS->sponsor.copy = bounded_sponsor_copy;
    THIS->sponsor.release = bounded_sponsor_release;
    THIS->sponsor.temp_pool = generic_sponsor_temp_pool;
    THIS->sponsor.destroy = bounded_sponsor_destroy;
    return (sponsor_t *)THIS;
}

/*
 * delegated pool-sponsor
 */

typedef struct {
    sponsor_t   sponsor;        // super-type member
    sponsor_t * parent;         // parent sponsor (for delegation)
    pool_t *    work_pool;      // temporary working-memory pool
} pool_sponsor_t;

static BYTE pool_sponsor_dispatch(sponsor_t * sponsor) {
    pool_sponsor_t * THIS = (pool_sponsor_t *)sponsor;
    return sponsor_dispatch(THIS->parent);
}

static BYTE pool_sponsor_create(sponsor_t * sponsor, scope_t * scope, DATA_PTR state, DATA_PTR behavior, DATA_PTR * address) {
    pool_sponsor_t * THIS = (pool_sponsor_t *)sponsor;
    return sponsor_create(THIS->parent, scope, state, behavior, address);
}

static BYTE pool_sponsor_send(sponsor_t * sponsor, DATA_PTR address, DATA_PTR message) {
    pool_sponsor_t * THIS = (pool_sponsor_t *)sponsor;
    return sponsor_send(THIS->parent, address, message);
}

static BYTE pool_sponsor_become(sponsor_t * sponsor, DATA_PTR behavior) {
    pool_sponsor_t * THIS = (pool_sponsor_t *)sponsor;
    return sponsor_become(THIS->parent, behavior);
}

static BYTE pool_sponsor_fail(sponsor_t * sponsor, event_t * event, DATA_PTR error) {
    pool_sponsor_t * THIS = (pool_sponsor_t *)sponsor;
    return sponsor_fail(THIS->parent, event, error);
}

static BYTE pool_sponsor_reserve(sponsor_t * sponsor, DATA_PTR * data, WORD size) {
    pool_sponsor_t * THIS = (pool_sponsor_t *)sponsor;
    return pool_reserve(THIS->work_pool, data, size);
}

static BYTE pool_sponsor_copy(sponsor_t * sponsor, DATA_PTR * data, DATA_PTR value) {
    pool_sponsor_t * THIS = (pool_sponsor_t *)sponsor;
    return pool_copy(THIS->work_pool, data, value);
}

static BYTE pool_sponsor_release(sponsor_t * sponsor, DATA_PTR * data) {
    pool_sponsor_t * THIS = (pool_sponsor_t *)sponsor;
    return pool_release(THIS->work_pool, data);
}

static BYTE pool_sponsor_destroy(sponsor_t * sponsor) {
    pool_sponsor_t * THIS = (pool_sponsor_t *)sponsor;
    //if (!pool_close(THIS->work_pool)) return false;  // close failed!
    sponsor = THIS->parent;  // we were allocated by our parent...
    if (!RELEASE((DATA_PTR *)&THIS))  {
        LOG_WARN("pool_sponsor_destroy: release failed!", (WORD)THIS);
        return false;  // reclamation failure!
    }
    return true;  // success!
}

sponsor_t * new_pool_sponsor(sponsor_t * sponsor, pool_t * pool) {
    DATA_PTR data = NULL;
    if (!RESERVE(&data, sizeof(pool_sponsor_t))) return NULL;  // allocation failure!
    pool_sponsor_t * THIS = (pool_sponsor_t *)data;
    THIS->parent = sponsor;
    THIS->work_pool = pool;
    THIS->sponsor.dispatch = pool_sponsor_dispatch;
    THIS->sponsor.create = pool_sponsor_create;
    THIS->sponsor.send = pool_sponsor_send;
    THIS->sponsor.become = pool_sponsor_become;
    THIS->sponsor.fail = pool_sponsor_fail;
    THIS->sponsor.reserve = pool_sponsor_reserve;
    THIS->sponsor.copy = pool_sponsor_copy;
    THIS->sponsor.release = pool_sponsor_release;
    THIS->sponsor.temp_pool = generic_sponsor_temp_pool;
    THIS->sponsor.destroy = pool_sponsor_destroy;
    return (sponsor_t *)THIS;
}

/*
 * polymorphic dispatch functions
 */

inline BYTE sponsor_dispatch(sponsor_t * sponsor) {
    return sponsor->dispatch(sponsor);
}

inline BYTE sponsor_create(sponsor_t * sponsor, scope_t * scope, DATA_PTR state, DATA_PTR behavior, DATA_PTR * address) {
    return sponsor->create(sponsor, scope, state, behavior, address);
}

inline BYTE sponsor_send(sponsor_t * sponsor, DATA_PTR address, DATA_PTR message) {
    return sponsor->send(sponsor, address, message);
}

inline BYTE sponsor_become(sponsor_t * sponsor, DATA_PTR behavior) {
    return sponsor->become(sponsor, behavior);
}

inline BYTE sponsor_fail(sponsor_t * sponsor, event_t * event, DATA_PTR error) {
    return sponsor->fail(sponsor, event, error);
}

inline BYTE sponsor_reserve(sponsor_t * sponsor, DATA_PTR * data, WORD size) {
    return sponsor->reserve(sponsor, data, size);
}

inline BYTE sponsor_copy(sponsor_t * sponsor, DATA_PTR * data, DATA_PTR value) {
    return sponsor->copy(sponsor, data, value);
}

inline BYTE sponsor_release(sponsor_t * sponsor, DATA_PTR * data) {
    return sponsor->release(sponsor, data);
}

inline BYTE sponsor_temp_pool(sponsor_t * sponsor, WORD size, sponsor_t ** child) {
    return sponsor->temp_pool(sponsor, size, child);
}

inline BYTE sponsor_destroy(sponsor_t * sponsor) {
    return sponsor->destroy(sponsor);
}
