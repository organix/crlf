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

#define SCOPE_PARENT(scope) ((scope)->parent)  // (scope_t *) -> (scope_t *)
#define SCOPE_STATE(scope) ((scope)->state)  // (scope_t *) -> (DATA_PTR)

BYTE scope_has_binding(scope_t * scope, DATA_PTR name);
BYTE scope_lookup_binding(scope_t * scope, DATA_PTR name, DATA_PTR * value);
BYTE scope_update_binding(scope_t * scope, DATA_PTR name, DATA_PTR value);

typedef struct actor_struct {
    BYTE        capability[8];
    scope_t     scope;
    DATA_PTR    behavior;
} actor_t;

#define ACTOR_SELF(actor) ((actor)->capability)  // (actor_t *) -> (DATA_PTR)
#define ACTOR_SCOPE(actor) (&(actor)->scope)  // (actor_t *) -> (scope_t *)
#define ACTOR_BEHAVIOR(actor) ((actor)->behavior)  // (actor_t *) -> (DATA_PTR)

typedef struct effect_struct {
    DATA_PTR    behavior;       // behavior for subsequent messages
    WORD        actors;         // snapshot of actor creation index
    WORD        events;         // snapshot of message-send event index
    scope_t     scope;          // scope for new bindings
    DATA_PTR    error;          // error value, or NULL if none
} effect_t;

#define EFFECT_BEHAVIOR(effect) ((effect)->behavior)  // (effect_t *) -> (DATA_PTR)
#define EFFECT_ACTORS(effect) ((effect)->actors)  // (effect_t *) -> (WORD)
#define EFFECT_EVENTS(effect) ((effect)->events)  // (effect_t *) -> (WORD)
#define EFFECT_SCOPE(effect) (&(effect)->scope)  // (effect_t *) -> (scope_t *)
#define EFFECT_ERROR(effect) ((effect)->error)  // (effect_t *) -> (DATA_PTR)

BYTE effect_init(effect_t * effect, actor_t * target, WORD actors, WORD events);
BYTE effect_create(effect_t * effect, DATA_PTR state, DATA_PTR behavior, DATA_PTR * address);
BYTE effect_send(effect_t * effect, DATA_PTR address, DATA_PTR message);
BYTE effect_become(effect_t * effect, DATA_PTR behavior);
BYTE effect_assign(effect_t * effect, DATA_PTR name, DATA_PTR value);
BYTE effect_fail(effect_t * effect, DATA_PTR error);

typedef struct event_struct {
    actor_t *   actor;
    DATA_PTR    message;
    effect_t    effect;         // actor-command effects
} event_t;

#define EVENT_ACTOR(event) ((event)->actor)  // (event_t *) -> (actor_t *)
#define EVENT_MESSAGE(event) ((event)->message)  // (event_t *) -> (DATA_PTR)
#define EVENT_EFFECT(event) (&(event)->effect)  // (event_t *) -> (effect_t *)

#endif // _EVENT_H_
