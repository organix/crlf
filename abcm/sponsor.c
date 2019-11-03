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
    if (!COPY_INTO(CONFIG_POOL(config), &scope->state, state)) return false;  // allocation failure!
    if (!COPY_INTO(CONFIG_POOL(config), &actor->behavior, behavior)) return false;  // allocation failure!
    *address = ACTOR_SELF(actor);
    //if (!COPY_INTO(CONFIG_POOL(config), address, ACTOR_SELF(actor))) return false;  // allocation failure!
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
    if (!COPY_INTO(CONFIG_POOL(config), &event->message, message)) return false;  // out-of-memory!
    return true;  // success!
}

/*
 * Attempt to deliver a pending _Event_.
 * Return `true` on success, `false` on failure (including no events pending).
 */
static BYTE config_script_exec(config_t * config, event_t * event, DATA_PTR script) {
    effect_t * effect = EVENT_EFFECT(event);
    if (!effect_init(effect, EVENT_ACTOR(event), CONFIG_ACTORS(config), CONFIG_EVENTS(config))) return false;  // init failed!
    if (!script_exec(event, script)) {
        if (EFFECT_ERROR(effect)) {
            LOG_WARN("config_script_exec: caught actor FAIL!", (WORD)EFFECT_ERROR(effect));
            IF_WARN(value_print(EFFECT_ERROR(effect), 1));
        } else {
            LOG_WARN("config_script_exec: script failed!", (WORD)script);
        }
        if (!config_rollback(config, effect)) {
            LOG_WARN("config_script_exec: rollback failed!", (WORD)effect);
            return false;
        }
    } else if (!config_commit(config, effect)) {
        LOG_WARN("config_script_exec: commit failed!", (WORD)effect);
        return false;
    }
    return true;  // success!
}
BYTE config_dispatch(config_t * config) {
    if (CONFIG_EVENTS(config) == CONFIG_CURRENT(config)) {
        LOG_INFO("config_dispatch: work completed.", (WORD)config);
        return false;  // work completed.
    }
    WORD current = --config->current;
    LOG_LEVEL(LOG_LEVEL_TRACE+1, "config_dispatch: current =", current);
    event_t * event = CONFIG_EVENT(config);
    LOG_LEVEL(LOG_LEVEL_TRACE+1, "config_dispatch: event =", (WORD)event);
    LOG_DEBUG("config_dispatch: message =", (WORD)EVENT_MESSAGE(event));
    IF_DEBUG(value_print(EVENT_MESSAGE(event), 1));
    actor_t * actor = EVENT_ACTOR(event);
    LOG_DEBUG("config_dispatch: actor =", (WORD)actor);
    IF_DEBUG(value_print(ACTOR_SELF(actor), 1));
    DATA_PTR behavior = ACTOR_BEHAVIOR(actor);
    DATA_PTR script;
    if (!object_get(behavior, s_script, &script)) {
        LOG_WARN("config_dispatch: script required!", (WORD)behavior);
        return false;  // script required!
    }
    // execute script using temporary memory pool
    assert(SPONSOR_POOL(sponsor) == CONFIG_POOL(config));
#if EVENT_TEMP_POOL_SIZE
    sponsor->pool = new_temp_pool(CONFIG_POOL(config), EVENT_TEMP_POOL_SIZE);
#endif
    LOG_DEBUG("config_dispatch: temp_pool =", (WORD)SPONSOR_POOL(sponsor));
    if (!SPONSOR_POOL(sponsor)) {
        sponsor->pool = CONFIG_POOL(config);
        return false;  // out-of-memory
    }
    BYTE ok = config_script_exec(config, event, script);
#if EVENT_TEMP_POOL_SIZE
    RELEASE_ALL(sponsor->pool);
    RELEASE_FROM(CONFIG_POOL(config), (DATA_PTR *)&sponsor->pool);
#endif
    sponsor->pool = CONFIG_POOL(config);
    if (!RELEASE(&event->message)) return false;  // reclamation failure!
    LOG_DEBUG("config_dispatch: event completed.", (WORD)event);
    return ok;
}

/*
 * Apply message-delivery Effects to Configuration state.
 * Return `true` on success, `false` on failure.
 */
BYTE config_commit(config_t * config, effect_t * effect) {
    LOG_TRACE("config_commit: config =", (WORD)config);
    event_t * event = CONFIG_EVENT(config);
    LOG_DEBUG("config_commit: event =", (WORD)event);
    actor_t * actor = EVENT_ACTOR(event);
    LOG_TRACE("config_commit: actor =", (WORD)actor);
    LOG_DEBUG("config_commit: effect =", (WORD)effect);
    DATA_PTR error = EFFECT_ERROR(effect);
    if (error) {
        LOG_DEBUG("config_commit: error =", (WORD)error);
        IF_TRACE(value_print(error, 0));
        LOG_WARN("CAN'T COMMMIT ERROR EFFECT!", (WORD)effect);
        return false;  // commit failure!
    }
    LOG_DEBUG("config_commit: new actors =", (EFFECT_ACTORS(effect) - CONFIG_ACTORS(config)));
    LOG_DEBUG("config_commit: new events =", (EFFECT_EVENTS(effect) - CONFIG_EVENTS(config)));
    // merge state update effects, if any, into actor state
    scope_t * scope = EFFECT_SCOPE(effect);
    WORD length;
    if (!object_length(SCOPE_STATE(scope), &length)) return false;  // failed to get object length!
    if (length > 0) {
        LOG_DEBUG("config_commit: state' =", (WORD)SCOPE_STATE(scope));
        IF_TRACE(value_print(SCOPE_STATE(scope), 1));
        scope_t * parent = ACTOR_SCOPE(actor);
        DATA_PTR state;
        if (!object_concat(SCOPE_STATE(parent), SCOPE_STATE(scope), &state)) {
            LOG_WARN("event_apply_effects: state merge failed!", (WORD)actor);
            return false;  // state merge failed!
        }
        if (!RELEASE(&scope->state)) return false;  // reclamation failure!
        // FIXME: if EFFECT_SCOPE is (still) empty, no need to merge/copy...
        if (!RELEASE_FROM(CONFIG_POOL(config), &parent->state)) return false;  // reclamation failure!
        if (!COPY_INTO(CONFIG_POOL(config), &parent->state, state)) return false;  // allocation failure!
    }
    // update actor behavior, if requested
    DATA_PTR behavior = EFFECT_BEHAVIOR(effect);
    if (behavior) {
        LOG_DEBUG("config_commit: behavior' =", (WORD)behavior);
        IF_TRACE(value_print(behavior, 0));
        if (!RELEASE_FROM(CONFIG_POOL(config), &actor->behavior)) return false;  // reclamation failure!
        if (!COPY_INTO(CONFIG_POOL(config), &actor->behavior, behavior)) return false;  // allocation failure!
        if (!RELEASE(&effect->behavior)) return false;  // reclamation failure!
        // FIXME: behaviors value should never be pool-allocated, so no need to copy/release...
    }
    // commit completed.
    return true;  // success!
}

/*
 * Abort message-delivery and undo any partial Effects on Configuration state.
 * Return `true` on success, `false` on failure.
 */
BYTE config_rollback(config_t * config, effect_t * effect) {
    LOG_TRACE("config_rollback: config =", (WORD)config);
    event_t * event = CONFIG_EVENT(config);
    LOG_DEBUG("config_rollback: event =", (WORD)event);
    actor_t * actor = EVENT_ACTOR(event);
    LOG_TRACE("config_rollback: actor =", (WORD)actor);
    LOG_DEBUG("config_rollback: effect =", (WORD)effect);
    // FIXME: RELEASE doomed actors and events...
    LOG_TRACE("config_rollback: actors =", CONFIG_ACTORS(config));
    LOG_TRACE("config_rollback: events =", CONFIG_EVENTS(config));
    config->actors = EFFECT_ACTORS(effect);
    config->events = EFFECT_EVENTS(effect);
    LOG_DEBUG("config_rollback: actors reset to", CONFIG_ACTORS(config));
    LOG_DEBUG("config_rollback: events reset to", CONFIG_EVENTS(config));
    scope_t * scope = EFFECT_SCOPE(effect);
    if (!RELEASE(&scope->state)) return false;  // reclamation failure!
    if (EFFECT_BEHAVIOR(effect)) {
        if (!RELEASE(&effect->behavior)) return false;  // reclamation failure!
    }
    if (EFFECT_ERROR(effect)) {
        if (!RELEASE(&effect->error)) return false;  // reclamation failure!
    }
    // rollback completed.
    LOG_WARN("config_rollback: rollback completed.", (WORD)effect);
    return true;  // success!
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
    config->pool = pool;
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
