/*
 * actor.c -- Actor operations
 */
//#include <string.h>  // for memcpy, et. al.
//#include <assert.h>

#include "actor.h"
#include "bose.h"
//#include "sponsor.h"
#include "equiv.h"
#include "print.h"

#define LOG_ALL // enable all logging
//#define LOG_INFO
//#define LOG_WARN
#include "log.h"

BYTE actor_create(DATA_PTR state, DATA_PTR behavior, DATA_PTR * actor) {
    LOG_TRACE("actor_create: state =", (WORD)state);
    if (!value_print(state, 0)) return false;  // print failed!
    LOG_TRACE("actor_create: behavior =", (WORD)behavior);
    if (!value_print(behavior, 0)) return false;  // print failed!
    *actor = v_null;  // FIXME: FINISH IMPLEMENTATION!
    LOG_WARN("actor_create: not implemented!", true);
    return true;  // success!
};

BYTE actor_send(DATA_PTR actor, DATA_PTR message) {
    LOG_TRACE("actor_send: actor =", (WORD)actor);
    LOG_TRACE("actor_send: message =", (WORD)message);
    if (!value_print(message, 0)) return false;  // print failed!
    if (value_equiv(actor, v_null)) {
        LOG_INFO("actor_send: ignoring message to null.", (WORD)message);
        // FIXME: we may want to catch this case earlier, and avoid evaluating the message expression...
        return true;  // success!
    }
    // FIXME: FINISH IMPLEMENTATION!
    LOG_WARN("actor_send: not implemented!", false);
    return false;  // failed!
};
