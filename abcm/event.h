/*
 * event.h -- actor message-delivery event
 */
#ifndef _EVENT_H_
#define _EVENT_H_

#include "bose.h"
#include "pool.h"

typedef struct scope_struct scope_t;
typedef struct scope_struct {
    scope_t *   parent;
    DATA_PTR    state;
} scope_t;

typedef struct actor_struct {
    BYTE        capability[8];
    scope_t     scope;
    DATA_PTR    behavior;
} actor_t;

typedef struct effect_struct {
    DATA_PTR    behavior;       // behavior for subsequent messages
    WORD        actors;         // updated actor creation limit
    WORD        events;         // updated message-send event limit
    scope_t     scope;          // scope for new bindings
    DATA_PTR    error;          // error value, or NULL if none
} effect_t;

typedef struct event_struct {
    actor_t *   actor;
    DATA_PTR    message;
    effect_t    effect;         // actor-command effects
} event_t;

/*
typedef struct scope_struct scope_t;
typedef struct actor_struct actor_t;
typedef struct event_struct event_t;

typedef struct scope_struct {
    scope_t *   parent;
    DATA_PTR    state;
} scope_t;

typedef struct actor_struct {
    BYTE        capability[8];
    scope_t     scope;
    DATA_PTR    behavior;
} actor_t;

typedef struct effect_struct {
    DATA_PTR    behavior;       // behavior for subsequent messages
    WORD        actors;         // actor creation limit
    WORD        events;         // message-send event limit
    scope_t     scope;          // scope for new bindings
    DATA_PTR    error;          // error value, or NULL if none
} effect_t;

typedef struct event_struct {
    actor_t *   actor;
    DATA_PTR    message;
    effect_t    effect;         // actor-command effects
} event_t;

#define PER_MESSAGE_LOCAL_SCOPE 1 // create a new empty scope per message, with the actor state as parent.

BYTE scope_has_binding(sponsor_t * sponsor, scope_t * scope, DATA_PTR name);
BYTE scope_lookup_binding(sponsor_t * sponsor, scope_t * scope, DATA_PTR name, DATA_PTR * value);
BYTE scope_update_binding(sponsor_t * sponsor, scope_t * scope, DATA_PTR name, DATA_PTR value);

BYTE event_lookup_behavior(sponsor_t * sponsor, event_t * event, DATA_PTR * behavior);
BYTE event_update_behavior(sponsor_t * sponsor, event_t * event, DATA_PTR behavior);
BYTE event_lookup_actor(sponsor_t * sponsor, event_t * event, DATA_PTR * self);
BYTE event_lookup_message(sponsor_t * sponsor, event_t * event, DATA_PTR * message);
BYTE event_lookup_scope(sponsor_t * sponsor, event_t * event, scope_t ** scope);
BYTE event_init_effects(sponsor_t * sponsor, event_t * event, WORD actors, WORD events);
BYTE event_apply_effects(sponsor_t * sponsor, event_t * event);
BYTE event_revert_effects(sponsor_t * sponsor, event_t * event);
*/

#endif // _EVENT_H_
