/*
 * event.h -- actor message-delivery event
 */
#include <stddef.h>  // for NULL, size_t, et. al.
#include <string.h>  // for memcpy, et. al.
#include <assert.h>

#include "event.h"
#include "bose.h"
#include "pool.h"
#include "sponsor.h"
#include "object.h"
//#include "equiv.h"
#include "print.h"

#define LOG_ALL // enable all logging
//#define LOG_INFO
//#define LOG_WARN
#include "log.h"

/*
 * Search `scope` for a binding to `name`.
 * Return `true` on success, `false` on failure.
 */
BYTE scope_has_binding(scope_t * scope, DATA_PTR name) {
    LOG_TRACE("scope_has_binding @", (WORD)scope);
    LOG_TRACE("scope_has_binding: name =", (WORD)name);
    WORD has = object_has(SCOPE_STATE(scope), name);
    while (!has) {
        scope = SCOPE_PARENT(scope);
        if (!scope) break;
        has = object_has(SCOPE_STATE(scope), name);
    }
    LOG_DEBUG("scope_has_binding", has);
    return has;
}

/*
 * Search `scope` for a value bound to `name`.
 * If found, `*value` is set to the value found.
 * If not found, `*value` is set to `v_null`.
 * Return `true` on success (`*value` assigned), `false` on failure.
 */
BYTE scope_lookup_binding(scope_t * scope, DATA_PTR name, DATA_PTR * value) {
    LOG_TRACE("scope_lookup_binding @", (WORD)scope);
    LOG_TRACE("scope_lookup_binding: name =", (WORD)name);
    *value = v_null;  // default value is `null`
    while (!object_get(SCOPE_STATE(scope), name, value)) {
        scope = SCOPE_PARENT(scope);
        if (!scope) {
            LOG_WARN("scope_lookup_binding: undefined variable!", (WORD)name);
            // FIXME: we may want to "throw" an exception here...
            break;
        }
        LOG_TRACE("scope_lookup_binding: searching scope", (WORD)SCOPE_STATE(scope));
        IF_TRACE(value_print(SCOPE_STATE(scope), 1));
    }
    LOG_DEBUG("scope_lookup_binding: value =", (WORD)*value);
    return true;  // success
}

/*
 * Update `scope` with a binding from `name` to `value`.
 * Note this can only affect the immediate (local) SCOPE_STATE.
 * Return `true` on success, `false` on failure.
 */
BYTE scope_update_binding(scope_t * scope, DATA_PTR name, DATA_PTR value) {
    LOG_TRACE("scope_update_binding @", (WORD)scope);
    LOG_TRACE("scope_update_binding: name =", (WORD)name);
    LOG_LEVEL(LOG_LEVEL_TRACE+1, "scope_update_binding: state =", (WORD)SCOPE_STATE(scope));
    IF_LEVEL(LOG_LEVEL_TRACE+1, value_print(SCOPE_STATE(scope), 1));
    DATA_PTR state;
    if (!object_add(SCOPE_STATE(scope), name, value, &state)) return false;  // allocation failure!
    if (!RELEASE(&scope->state)) return false;  // reclamation failure!
    scope->state = TRACK(state);
    LOG_LEVEL(LOG_LEVEL_TRACE+1, "scope_update_binding: state' =", (WORD)state);
    IF_LEVEL(LOG_LEVEL_TRACE+1, value_print(state, 1));
    LOG_DEBUG("scope_update_binding: value =", (WORD)value);
    return true;  // success
}

/*
 * Create a new Actor with initial `state` and `behavior`.
 * If successful, `*address` is set to the object-capability designating the new actor.
 * Return `true` on success, `false` on failure.
 */
BYTE effect_create(effect_t * effect, DATA_PTR state, DATA_PTR behavior, DATA_PTR * address) {
    LOG_TRACE("effect_create: state =", (WORD)state);
    LOG_TRACE("effect_create: behavior =", (WORD)behavior);
    scope_t * scope = SCOPE_PARENT(EFFECT_SCOPE(effect));  // inherit from parent actor's scope
    BYTE ok = sponsor_create(sponsor, scope, state, behavior, address);
    if (ok) {
        LOG_DEBUG("effect_create: address =", (WORD)*address);
        IF_TRACE(value_print(*address, 1));
        *address = TRACK(*address);
    }
    return ok;
}

/*
 * Send an asynchronous `message` to the actor designated by object-capability `address`.
 * Return `true` on success, `false` on failure.
 */
BYTE effect_send(effect_t * effect, DATA_PTR address, DATA_PTR message) {
    BYTE ok = sponsor_send(sponsor, address, message);
    if (ok) {
        LOG_DEBUG("effect_send: address =", (WORD)address);
        IF_TRACE(value_print(address, 1));
        LOG_DEBUG("effect_send: message =", (WORD)message);
        IF_TRACE(value_print(message, 1));
    }
    return ok;
}

/*
 * Define subsequent Actor `behavior`.
 * Return `true` on success, `false` on failure.
 */
BYTE effect_become(effect_t * effect, DATA_PTR behavior) {
    LOG_DEBUG("effect_become: behavior =", (WORD)behavior);
    effect->behavior = behavior;
    IF_TRACE(value_print(behavior, 0));
    return true;  // success
}

/*
 * Define subsequent Actor State, where `name` is bound to `value`.
 * Return `true` on success, `false` on failure.
 */
BYTE effect_assign(effect_t * effect, DATA_PTR name, DATA_PTR value) {
    BYTE ok = scope_update_binding(EFFECT_SCOPE(effect), name, value);
    if (ok) {
        LOG_DEBUG("effect_assign: name =", (WORD)name);
        IF_TRACE(value_print(name, 1));
        LOG_DEBUG("effect_assign: value =", (WORD)value);
        IF_TRACE(value_print(value, 0));
    }
    return ok;
}

/*
 * Signal an `error` condition.
 * Return `true` on success, `false` on failure.
 */
BYTE effect_fail(effect_t * effect, DATA_PTR error) {
    LOG_DEBUG("effect_fail: error =", (WORD)error);
    effect->error = error;
    IF_TRACE(value_print(error, 1));
    return true;  // success!
}

#if 0
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
#if PER_MESSAGE_LOCAL_SCOPE
    *scope = &event->effect.scope;
#else
    *scope = &event->actor->scope;
#endif
    LOG_TRACE("event_lookup_scope: scope =", (WORD)*scope);
    return true;  // success
}

BYTE event_init_effects(sponsor_t * sponsor, event_t * event, WORD actors, WORD events) {
    LOG_TRACE("event_init_effects: event =", (WORD)event);
    //actor_t * actor = event->actor;
    event->effect.behavior = NULL;
    event->effect.actors = actors;
    event->effect.events = events;
#if PER_MESSAGE_LOCAL_SCOPE
    event->effect.scope.parent = &event->actor->scope;
    if (!COPY(&event->effect.scope.state, o_)) return false;  // allocation failure!
    LOG_DEBUG("event_init_effects: state =", (WORD)event->effect.scope.state);
    IF_TRACE(value_print(event->effect.scope.state, 1));
#endif
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
#if PER_MESSAGE_LOCAL_SCOPE
    DATA_PTR state;
    if (!object_concat(sponsor, actor->scope.state, event->effect.scope.state, &state)) {
        LOG_WARN("event_apply_effects: state merge failed!", (WORD)&event->effect.scope);
        return false;  // state merge failed!
    }
    if (!RELEASE(&actor->scope.state)) return false;  // reclamation failure!
    actor->scope.state = TRACK(state);
    LOG_DEBUG("event_apply_effects: state =", (WORD)event->effect.scope.state);
    IF_TRACE(value_print(event->effect.scope.state, 1));
#endif
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
#endif
