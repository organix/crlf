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
#include "runtime.h"
#include "object.h"
#include "equiv.h"
#include "print.h"

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
    IF_TRACE(value_print_limit(state, 0, 2));
    LOG_TRACE("config_create: behavior =", (WORD)behavior);
    IF_TRACE(value_print_limit(behavior, 0, 2));
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
    actor_t * actor = EVENT_ACTOR(event);
#if ANNOTATE_DELIVERY
    hex_word((WORD)sponsor);
    print(':');
/*
    parse_t parse;
    if (!value_parse(ACTOR_SELF(actor), &parse)) {
        LOG_WARN("value_print: bad value", (WORD)ACTOR_SELF(actor));
        return false;  // bad value
    }
    parse_print(&parse, 0);
    prints(" <== ");
    if (!value_parse(EVENT_MESSAGE(event), &parse)) {
        LOG_WARN("value_print: bad value", (WORD)EVENT_MESSAGE(event));
        return false;  // bad value
    }
    parse_print(&parse, 0);
    newline();
*/
    value_print(ACTOR_SELF(actor), 0);
    prints("  <== ");
    value_print(EVENT_MESSAGE(event), 0);
#else
    LOG_DEBUG("config_dispatch: message =", (WORD)EVENT_MESSAGE(event));
    IF_DEBUG(value_print(EVENT_MESSAGE(event), 1));
    LOG_DEBUG("config_dispatch: actor =", (WORD)actor);
    IF_DEBUG(value_print(ACTOR_SELF(actor), 1));
#endif
    DATA_PTR behavior = ACTOR_BEHAVIOR(actor);
    LOG_TRACE("config_dispatch: behavior =", (WORD)behavior);
    IF_TRACE(value_print_limit(behavior, 0, 2));
    DATA_PTR script;
    if (!object_get(behavior, s_script, &script)) {
        LOG_WARN("config_dispatch: script required!", (WORD)behavior);
        return false;  // script required!
    }
    // execute script using temporary memory pool
    pool_t * sponsor_pool = SPONSOR_POOL(sponsor);  // remember original sponsor pool
    //assert(SPONSOR_POOL(sponsor) == CONFIG_POOL(config));
#if STATIC_TEMP_POOL_SIZE
    sponsor->pool = clear_temp_pool();
#else
#if EVENT_TEMP_POOL_SIZE
    sponsor->pool = new_temp_pool(CONFIG_POOL(config), EVENT_TEMP_POOL_SIZE);
#endif
#endif
    LOG_DEBUG("config_dispatch: temp_pool =", (WORD)SPONSOR_POOL(sponsor));
    if (!SPONSOR_POOL(sponsor)) {
        sponsor->pool = sponsor_pool;
        return false;  // out-of-memory
    }
#if ANNOTATE_BEHAVIOR
    prints("BEH: ");
    value_print_limit(script, 1, 2);
#endif
    BYTE ok = config_script_exec(config, event, script);
#if STATIC_TEMP_POOL_SIZE
    RELEASE_ALL(sponsor->pool);
    sponsor->pool = NULL;
#else
#if EVENT_TEMP_POOL_SIZE
    RELEASE_ALL(sponsor->pool);
    RELEASE_FROM(CONFIG_POOL(config), (DATA_PTR *)&sponsor->pool);
#endif
#endif
    sponsor->pool = sponsor_pool;  // restore original sponsor pool
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
    WORD count;
    if (!object_count(SCOPE_STATE(scope), &count)) return false;  // failed to get object property count!
    if (count > 0) {
        LOG_DEBUG("config_commit: state' =", (WORD)SCOPE_STATE(scope));
        IF_TRACE(value_print(SCOPE_STATE(scope), 1));
        scope_t * parent = ACTOR_SCOPE(actor);
        DATA_PTR state;
        if (!object_concat(SCOPE_STATE(parent), SCOPE_STATE(scope), &state)) {
            LOG_WARN("event_apply_effects: state merge failed!", (WORD)actor);
            return false;  // state merge failed!
        }
        state = TRACK(state);
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
    }
    // commit completed.
    return true;  // success!
}

static BYTE config_release_actors(config_t * config, WORD final_actors) {
    LOG_TRACE("config_release_actors: config =", (WORD)config);
    pool_t * pool = CONFIG_POOL(config);
    LOG_LEVEL(LOG_LEVEL_TRACE+1, "config_release_actors: pool =", (WORD)pool);
    // release actors
    LOG_LEVEL(LOG_LEVEL_TRACE+1, "config_release_actors: config->actor =", (WORD)config->actor);
    while (CONFIG_ACTORS(config) < final_actors) {
        LOG_TRACE("config_release_actors: releasing actor #", CONFIG_ACTORS(config));
        actor_t * actor = &config->actor[config->actors++];
        LOG_TRACE("config_release_actors: actor =", (WORD)actor);
        IF_LEVEL(LOG_LEVEL_TRACE+1, value_print(ACTOR_SELF(actor), 1));
        if (!RELEASE_FROM(pool, &actor->behavior)) return false;  // reclamation failure!
        if (!RELEASE_FROM(pool, &ACTOR_SCOPE(actor)->state)) return false;  // reclamation failure!
    }
    return true;  // success!
}
static BYTE config_release_events(config_t * config, WORD final_events) {
    assert(final_events <= CONFIG_CURRENT(config));  // can only release un-delivered events...
    LOG_TRACE("config_release_events: config =", (WORD)config);
    pool_t * pool = CONFIG_POOL(config);
    LOG_LEVEL(LOG_LEVEL_TRACE+1, "config_release_events: pool =", (WORD)pool);
    // release events (if any)
    LOG_LEVEL(LOG_LEVEL_TRACE+1, "config_release_events: config->event =", (WORD)config->event);
    while (CONFIG_EVENTS(config) < final_events) {
        LOG_TRACE("config_release_events: releasing event #", CONFIG_EVENTS(config));
        event_t * event = &config->event[config->events++];
        LOG_TRACE("config_release_events: event =", (WORD)event);
        if (!RELEASE_FROM(pool, &event->message)) return false;  // reclamation failure!
    }
    LOG_DEBUG("config_release_events: final_events =", CONFIG_EVENTS(config));
    return true;  // success!
}
/*
 * Abort message-delivery and undo any partial Effects on Configuration state.
 * Return `true` on success, `false` on failure.
 */
BYTE config_rollback(config_t * config, effect_t * effect) {
    LOG_TRACE("config_rollback: config =", (WORD)config);
    LOG_DEBUG("config_rollback: effect =", (WORD)effect);
    // release actors
    LOG_TRACE("config_rollback: actors =", CONFIG_ACTORS(config));
    if (!config_release_actors(config, EFFECT_ACTORS(effect))) return false;  // release failure!
    LOG_DEBUG("config_rollback: actors reset to", CONFIG_ACTORS(config));
    // release events
    LOG_TRACE("config_rollback: events =", CONFIG_EVENTS(config));
    if (!config_release_events(config, EFFECT_EVENTS(effect))) return false;  // release failure!
    LOG_DEBUG("config_rollback: events reset to", CONFIG_EVENTS(config));
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
    if (!RESERVE_FROM(pool, (DATA_PTR *)&config, sizeof(config_t))) return NULL;  // allocation failure!
    config->pool = pool;
    config->actors = actors;
    if (actors) {
        if (!RESERVE_FROM(pool, (DATA_PTR *)&config->actor, sizeof(actor_t) * actors)) return NULL;  // allocation failure!
    } else {
        config->actor = NULL;  // there are no actors in this config
    }
    config->events = events;
    if (events) {
        if (!RESERVE_FROM(pool, (DATA_PTR *)&config->event, sizeof(event_t) * events)) return NULL;  // allocation failure!
    } else {
        config->event = NULL;  // there are no events in this config
    }
    config->current = events;  // event queue is initially empty...
    LOG_DEBUG("new_config: config =", (WORD)config);
    return config;
}

BYTE config_release(config_t ** config_ref, WORD actors, WORD events) {
    config_t * config = *config_ref;
    LOG_DEBUG("config_release: config =", (WORD)config);
    pool_t * pool = CONFIG_POOL(config);
    LOG_TRACE("config_release: pool =", (WORD)pool);
    // release actors
    LOG_TRACE("config_release: config->actor =", (WORD)config->actor);
    if (config->actor) {
        if (!config_release_actors(config, actors)) return false;  // release failure!
        if (!RELEASE_FROM(pool, (DATA_PTR *)&config->actor)) return false;  // reclamation failure!
    }
    // release events (if any)
    LOG_TRACE("config_release: config->event =", (WORD)config->event);
    if (config->event) {
        if (!config_release_events(config, CONFIG_CURRENT(config))) return false;  // release failure!
        if (!RELEASE_FROM(pool, (DATA_PTR *)&config->event)) return false;  // reclamation failure!
    }
    // release config
    LOG_TRACE("config_release: releasing config", (WORD)*config_ref);
    if (!RELEASE_FROM(pool, (DATA_PTR *)config_ref)) return false;  // reclamation failure!
    LOG_WARN("config_release: release completed.", (WORD)*config_ref);
    return true;  // success!
}

/*
 * a sponsor is a root object providing access to resource-management mechanisms for computations.
 */

sponsor_t * new_sponsor(pool_t * pool, memo_t * memo, WORD actors, WORD events) {
    sponsor_t * sponsor;
    if (!RESERVE_FROM(pool, (DATA_PTR *)&sponsor, sizeof(sponsor_t))) return NULL;  // allocation failure!
    LOG_TRACE("new_sponsor: pool =", (WORD)pool);
    sponsor->pool = pool;
    LOG_TRACE("new_sponsor: memo =", (WORD)memo);
    sponsor->memo = memo;
#if REF_COUNTED_BOOT_SPONSOR
    if (memo) {
        if (!COPY_INTO(pool, (DATA_PTR *)&sponsor->memo, (DATA_PTR)memo)) {  // increase memo ref-count
            LOG_WARN("load_program: failed to increase memo ref-count!", (WORD)memo);
            return NULL;  // failed to increase memo ref-count!
        }
    }
#endif
    sponsor->actors = actors;
    sponsor->events = events;
#if USE_HEAP_POOL_FOR_CONFIG
    sponsor->config = new_config(heap_pool, actors, events);  // switch to heap-pool for config allocations
#else
    sponsor->config = new_config(pool, actors, events);  // allocate config from same pool as sponsor
#endif
    if (!SPONSOR_CONFIG(sponsor)) return NULL;  // allocation failure!
    sponsor->next = NULL;
    LOG_DEBUG("new_sponsor: sponsor =", (WORD)sponsor);
    return sponsor;
}

static BYTE sponsor_release_ring(sponsor_t * sponsor) {
    // release each sponsor in the dispatch ring, if any.
    sponsor_t * current = SPONSOR_NEXT(sponsor);
    LOG_DEBUG("sponsor_release_ring: first sponsor =", (WORD)current);
    if (current) {
#if REF_COUNTED_BOOT_SPONSOR
        do {
            sponsor_t * next = SPONSOR_NEXT(current);
            memo_t * memo = SPONSOR_MEMO(current);
            LOG_DEBUG("sponsor_release_ring: releasing memo =", (WORD)memo);
            if (!RELEASE((DATA_PTR *)&current->memo)) {
                LOG_WARN("sponsor_release_ring: failed to reduce memo ref-count!", (WORD)memo);
                return false;  // failed to release memo!
            }
            sponsor_t * victim = current;
            LOG_DEBUG("sponsor_release_ring: releasing sponsor =", (WORD)victim);
            if (!RELEASE((DATA_PTR *)&victim)) {
                LOG_WARN("sponsor_release_ring: failed to release sponsor!", (WORD)current);
                return false;  // reclamation failure!
            }
            current = next;
        } while (current != SPONSOR_NEXT(sponsor));
        sponsor->next = NULL;  // clear link to first sponsor
        LOG_WARN("sponsor_release_ring: released ring for", (WORD)sponsor);
#else
        memo_t * memo = NULL;
        sponsor_t * next = SPONSOR_NEXT(current);
        do {
            // FIXME: this weird iteration is needed to release the memo structures -- should use ref-counted allocator!
            current = next;
            next = SPONSOR_NEXT(current);
            if (memo != SPONSOR_MEMO(current)) {
                memo = SPONSOR_MEMO(current);
                LOG_DEBUG("sponsor_release_ring: releasing memo =", (WORD)memo);
                if (!RELEASE((DATA_PTR *)&current->memo)) {
                    LOG_WARN("sponsor_release_ring: failed to release memo!", (WORD)memo);
                    return false;  // failed to release memo!
                }
            }
            sponsor_t * victim = current;
            LOG_DEBUG("sponsor_release_ring: releasing sponsor =", (WORD)victim);
            if (!RELEASE((DATA_PTR *)&victim)) {
                LOG_WARN("sponsor_release_ring: failed to release sponsor!", (WORD)current);
                return false;  // reclamation failure!
            }
        } while (current != SPONSOR_NEXT(sponsor));
        sponsor->next = NULL;  // clear link to first sponsor
        LOG_WARN("sponsor_release_ring: released ring for", (WORD)sponsor);
#endif
    }
    return true;  // success!
}

BYTE sponsor_shutdown(sponsor_t * sponsor) {
    LOG_DEBUG("sponsor_shutdown: sponsor =", (WORD)sponsor);
    if (!config_release(&sponsor->config, sponsor->actors, sponsor->events)) return false;  // reclamation failure!
    LOG_WARN("sponsor_shutdown: shutdown completed.", (WORD)sponsor);
    return true;  // success!
}

BYTE sponsor_release(sponsor_t ** sponsor_ref) {
    LOG_DEBUG("sponsor_release: sponsor =", (WORD)*sponsor_ref);
    assert(SPONSOR_POOL(*sponsor_ref) == SPONSOR_POOL(sponsor));
    if (!sponsor_release_ring(*sponsor_ref)) return false;  // reclamation failure!
    if (!sponsor_shutdown(*sponsor_ref)) return false;  // shutdown failure!
    if (!RELEASE((DATA_PTR *)sponsor_ref)) return false;  // reclamation failure!
    LOG_WARN("sponsor_release: release completed.", (WORD)*sponsor_ref);
    return true;  // success!
}
