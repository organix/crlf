/*
 * actor.h -- Actor operations
 */
#ifndef _ACTOR_H_
#define _ACTOR_H_

#include "bose.h"
//#include "sponsor.h"

/*
    { "kind":"actor_send", "message":<dictionary>, "actor":<address> }
    { "kind":"actor_become", "behavior":<behavior> }
    { "kind":"actor_ignore" }
    { "kind":"actor_assign", "name":<string>, "value":<expression> }
    { "kind":"actor_fail", "error":<expression> }
    { "kind":"actor_create", "state":<dictionary>, "behavior":<behavior> }
    { "kind":"actor_self" }
    { "kind":"actor_behavior", "name":<string>, "script":[<action>, ...] }
    { "kind":"actor_state", "name":<string> }
    { "kind":"actor_has_state", "name":<string> }
    { "kind":"actor_message" }
*/

BYTE actor_send(DATA_PTR actor, DATA_PTR message);

#endif // _ACTOR_H_
