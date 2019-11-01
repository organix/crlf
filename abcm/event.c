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
