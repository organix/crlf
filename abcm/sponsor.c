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

/*
 * a configuration is an actor-machine state consisting of actors and pending events
 */

static actor_t * config_find_actor(config_t * config, DATA_PTR address) {
    LOG_LEVEL(LOG_LEVEL_TRACE+1, "config_find_actor: address =", (WORD)address);
    IF_NONE(value_print(address, 1));
    parse_t parse;
    if (!value_parse(address, &parse)
    ||  !(parse.type & T_Capability)) {
        LOG_WARN("config_find_actor: bad address!", (WORD)address);
        return NULL;  // bad address!
    }
    //DUMP_PARSE("config_find_actor: address", &parse);
    WORD ocap = 0;
    while (parse.value--) {
        ocap <<= 8;
        ocap |= parse.base[--parse.end];
    }
    LOG_LEVEL(LOG_LEVEL_TRACE+1, "config_find_actor: ocap =", ocap);
    actor_t * actor = &config->actor[ocap];
    LOG_LEVEL(LOG_LEVEL_TRACE, "config_find_actor: actor =", (WORD)actor);
    return actor;
}

BYTE config_create(config_t * config, scope_t * parent, DATA_PTR state, DATA_PTR behavior, DATA_PTR * address) {
    if (config->actors < 1) {
        LOG_WARN("config_create: no more actors!", (WORD)config);
        return false;  // no more actors!
    }
    LOG_TRACE("config_create: state =", (WORD)state);
    IF_TRACE(value_print(state, 0));
    LOG_TRACE("config_create: behavior =", (WORD)behavior);
    IF_TRACE(value_print(behavior, 0));
    WORD ocap = --config->actors;
    LOG_LEVEL(LOG_LEVEL_TRACE+1, "config_create: ocap =", ocap);
    actor_t * actor = &config->actor[ocap];
    LOG_LEVEL(LOG_LEVEL_TRACE+1, "config_create: actor =", (WORD)actor);
    actor->capability[0] = octets;      // binary octet data
    actor->capability[1] = n_3;         // size field (3 bytes)
    actor->capability[2] = 0x10;        // capability marker (DLE)
    actor->capability[3] = ocap & 0xFF; // capability (LSB)
    actor->capability[4] = ocap >> 8;   // capability (MSB)
    actor->capability[5] = null;        // --unused--
    actor->capability[6] = null;        // --unused--
    actor->capability[7] = null;        // --unused--
    scope_t * scope = ACTOR_SCOPE(actor);
    scope->parent = parent;  // chain scope to parent
    if (!COPY(&scope->state, state)) return false;  // allocation failure!
    if (!COPY(&actor->behavior, behavior)) return false;  // allocation failure!
    *address = ACTOR_SELF(actor);
    LOG_DEBUG("config_create: address =", (WORD)*address);
    IF_TRACE(value_print(*address, 0));
    return true;  // success!
}

BYTE config_send(config_t * config, DATA_PTR address, DATA_PTR message) {
    if (config->events < 1) {
        LOG_WARN("config_send: no more message-send events!", (WORD)config);
        return false;  // no more message-send events!
    }
    LOG_DEBUG("config_send: address =", (WORD)address);
    IF_TRACE(value_print(address, 1));
    actor_t * actor = config_find_actor(config, address);
    if (!actor) return false;  // bad actor!
    LOG_DEBUG("config_send: message =", (WORD)message);
    IF_TRACE(value_print(message, 1));
    if (value_equiv(address, v_null)) {
        LOG_WARN("config_send: ignoring message to null.", (WORD)message);
        // NOTE: this case is usually caught before evaluating the message expression, but just to be sure...
        return true;  // success!
    }
    WORD current = --config->events;
    LOG_LEVEL(LOG_LEVEL_TRACE+1, "config_send: current =", current);
    event_t * event = &config->event[current];
    LOG_LEVEL(LOG_LEVEL_TRACE+0, "config_send: event =", (WORD)event);
    event->actor = actor;
    if (!COPY(&event->message, message)) return false;  // out-of-memory!
    return true;  // success!
}

/*
 * Attempt to deliver a pending _Event_.
 * Return `true` on success, `false` on failure (including no events pending).
 */
BYTE config_dispatch(config_t * config) {
    if (CONFIG_EVENTS(config) == CONFIG_CURRENT(config)) {
        LOG_INFO("config_dispatch: work completed.", (WORD)config);
        return false;  // work completed.
    }
    WORD current = --config->current;
    LOG_LEVEL(LOG_LEVEL_TRACE+1, "config_dispatch: current =", current);
    event_t * event = &config->event[current];
    LOG_LEVEL(LOG_LEVEL_TRACE+1, "config_dispatch: event =", (WORD)event);
    //sponsor->event = event;  // FIXME: maybe we should keep this in `config_t` rather than `sponsor_t`...
    LOG_DEBUG("config_dispatch: message =", (WORD)EVENT_MESSAGE(event));
    IF_DEBUG(value_print(EVENT_MESSAGE(event), 1));
    actor_t * actor = EVENT_ACTOR(event);
    LOG_DEBUG("config_dispatch: actor =", (WORD)actor);
    IF_DEBUG(value_print(ACTOR_SELF(actor), 1));
    effect_t * effect = EVENT_EFFECT(event);
    if (!effect_init(effect, actor, CONFIG_ACTORS(config), CONFIG_EVENTS(config))) return false;  // init failed!
    DATA_PTR behavior = ACTOR_BEHAVIOR(actor);
    DATA_PTR script;
    if (!object_get(behavior, s_script, &script)) {
        LOG_WARN("config_dispatch: script required!", (WORD)behavior);
        return 1;  // script required!
    }
    //WORD size = (1 << 12);  // 4k working-memory pool
    if (!script_exec(event, script)) {
        LOG_WARN("config_dispatch: script failed!", (WORD)script);
        return 1;  // script failed!
    }

    if (run_actor_script(sponsor, event) == 0) {
        LOG_DEBUG("config_dispatch: new actors", (EFFECT_ACTORS(effect) - CONFIG_ACTORS(config)));
        LOG_DEBUG("config_dispatch: new events", (EFFECT_EVENTS(effect) - CONFIG_EVENTS(config)));
        if (!event_apply_effects(sponsor, event)) {
            LOG_WARN("config_dispatch: failed to apply effects!", (WORD)event);
            return false;  // effects failed!
        }
        if (!RELEASE(&event->message)) return false;  // reclamation failure!
    } else {
        if (EFFECT_ERROR(effect)) {
            LOG_WARN("config_dispatch: caught actor FAIL!", (WORD)EFFECT_ERROR(effect));
            IF_WARN(value_print(EFFECT_ERROR(effect), 1));
        }
        LOG_WARN("config_dispatch: actor-script execution failed!", (WORD)event);
        if (!event_revert_effects(sponsor, event)) {
            LOG_WARN("config_dispatch: failed to revert effects!", (WORD)event);
            return false;  // effects failed!
        }
        if (!RELEASE(&event->message)) return false;  // reclamation failure!
        return false;  // execution failed!
    }
    LOG_DEBUG("config_dispatch: event completed.", (WORD)event);
    return true;  // success!
}

/*
 * Apply message-delivery Effects.
 * Return `true` on success, `false` on failure.
 */
BYTE config_apply(config_t * config, effect_t * effect) {
    return false;  // NOT IMPLEMENTED!
}

config_t * new_config(pool_t * pool, WORD actors, WORD events) {
    LOG_DEBUG("new_config: actors =", actors);
    if (actors > 0xFFFF) {  // FIXME: this should be a configurable limit somewhere, but this code depends on it.
        LOG_WARN("new_config: too many actors!", 0xFFFF);
        return NULL;  // too many actors!
    }
    LOG_DEBUG("new_config: events =", events);
    if (events > 0xFFFF) {  // FIXME: this should be a configurable limit somewhere, but this code depends on it.
        LOG_WARN("new_config: too many events!", 0xFFFF);
        return NULL;  // too many events!
    }
    config_t * config;
    if (!pool_reserve(pool, (DATA_PTR *)&config, sizeof(config_t))) return NULL;  // allocation failure!
    config->actors = actors;
    if (actors) {
        if (!pool_reserve(pool, (DATA_PTR *)&config->actor, sizeof(actor_t) * actors)) return NULL;  // allocation failure!
    } else {
        config->actor = NULL;  // there are no actors in this config
    }
    config->events = events;
    if (events) {
        if (!pool_reserve(pool, (DATA_PTR *)&config->event, sizeof(event_t) * events)) return NULL;  // allocation failure!
    } else {
        config->event = NULL;  // there are no events in this config
    }
    config->current = events;  // event queue is initially empty...
    LOG_DEBUG("new_config: config =", (WORD)config);
    return config;
}

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
 * resource-bounded sponsor
 */

#if 0
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
#endif

/*
 * a sponsor is a root object providing access to resource-management mechanisms for computations.
 */

sponsor_t * new_sponsor(pool_t * pool, WORD actors, WORD events) {
    sponsor_t * sponsor;
    if (!pool_reserve(pool, (DATA_PTR *)&sponsor, sizeof(sponsor_t))) return NULL;  // allocation failure!
    sponsor->pool = pool;
    if (!SPONSOR_POOL(sponsor)) return NULL;  // allocation failure!
    sponsor->config = new_config(pool, actors, events);
    if (!SPONSOR_CONFIG(sponsor)) return NULL;  // allocation failure!
    LOG_DEBUG("new_sponsor: sponsor =", (WORD)sponsor);
    return sponsor;
}

inline BYTE sponsor_dispatch(sponsor_t * sponsor) {
    return config_dispatch(SPONSOR_CONFIG(sponsor));
}

inline BYTE sponsor_create(sponsor_t * sponsor, scope_t * parent, DATA_PTR state, DATA_PTR behavior, DATA_PTR * address) {
    return config_create(SPONSOR_CONFIG(sponsor), parent, state, behavior, address);
}

inline BYTE sponsor_send(sponsor_t * sponsor, DATA_PTR address, DATA_PTR message) {
    return config_send(SPONSOR_CONFIG(sponsor), address, message);
}

inline BYTE sponsor_become(sponsor_t * sponsor, DATA_PTR behavior) {
    return effect_become(EVENT_EFFECT(SPONSOR_EVENT(sponsor)), behavior);
}

inline BYTE sponsor_fail(sponsor_t * sponsor, event_t * event, DATA_PTR error) {
    assert(SPONSOR_EVENT(sponsor) == event);
    return effect_fail(EVENT_EFFECT(event), error);
}
