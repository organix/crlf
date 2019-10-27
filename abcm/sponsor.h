/*
 * sponsor.h -- actor resource provider/manager
 */
#ifndef _SPONSOR_H_
#define _SPONSOR_H_

#include "bose.h"
#include "pool.h"

/*
 * The following convenience macros make it easy to inject auditing information to track/debug memory leaks
 */
#define AUDIT_ALLOCATION 1 /* track reserve/release calls to check for leaks */

#if AUDIT_ALLOCATION
#define RESERVE(dpp,size) audit_reserve(__FILE__, __LINE__, (sponsor), (dpp), (size))
#define COPY(to_dpp,from_dp) audit_copy(__FILE__, __LINE__, (sponsor), (to_dpp), (from_dp))
#define RELEASE(dpp) audit_release(__FILE__, __LINE__, (sponsor), (dpp))
#define TRACK(dp) audit_track(__FILE__, __LINE__, (sponsor), (dp))
#else
#define RESERVE(dpp,size) sponsor_reserve(sponsor, (dpp), (size))
#define COPY(to_dpp,from_dp) sponsor_copy(sponsor, (to_dpp), (from_dp))
#define RELEASE(dpp) sponsor_release(sponsor, (dpp))
#define TRACK(dp) (dp)
#endif

typedef struct sponsor_struct sponsor_t;

BYTE memo_reset(sponsor_t * sponsor);  // reset memo table between top-level values
DATA_PTR memo_get(BYTE index);  // get memo table entry at index
BYTE memo_add(parse_t * parse);  // add parsed String to memo table

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
    DATA_PTR    error;          // error value, or NULL if none
} effect_t;

typedef struct event_struct {
    actor_t *   actor;
    DATA_PTR    message;
    effect_t    effect;         // actor-command effects
} event_t;

BYTE event_init_scope(sponsor_t * sponsor, event_t * event, actor_t * actor, DATA_PTR state);
BYTE event_has_binding(sponsor_t * sponsor, event_t * event, DATA_PTR name);
BYTE event_lookup_binding(sponsor_t * sponsor, event_t * event, DATA_PTR name, DATA_PTR * value);
BYTE event_update_binding(sponsor_t * sponsor, event_t * event, DATA_PTR name, DATA_PTR value);
BYTE event_lookup_behavior(sponsor_t * sponsor, event_t * event, DATA_PTR * behavior);
BYTE event_update_behavior(sponsor_t * sponsor, event_t * event, DATA_PTR behavior);
BYTE event_lookup_actor(sponsor_t * sponsor, event_t * event, DATA_PTR * self);
BYTE event_lookup_message(sponsor_t * sponsor, event_t * event, DATA_PTR * message);
BYTE event_init_effects(sponsor_t * sponsor, event_t * event);
BYTE event_apply_effects(sponsor_t * sponsor, event_t * event);

typedef struct sponsor_struct {
    BYTE        (*dispatch)(sponsor_t * sponsor);
    // actor primitives
    BYTE        (*create)(sponsor_t * sponsor, event_t * event, DATA_PTR state, DATA_PTR behavior, DATA_PTR * address);
    BYTE        (*send)(sponsor_t * sponsor, DATA_PTR address, DATA_PTR message);
    BYTE        (*become)(sponsor_t * sponsor, DATA_PTR behavior);
    BYTE        (*fail)(sponsor_t * sponsor, event_t * event, DATA_PTR error);
    // memory management
    BYTE        (*reserve)(sponsor_t * sponsor, DATA_PTR * data, WORD size);
    BYTE        (*share)(sponsor_t * sponsor, DATA_PTR * data);
    BYTE        (*copy)(sponsor_t * sponsor, DATA_PTR * data, DATA_PTR value);
    BYTE        (*release)(sponsor_t * sponsor, DATA_PTR * data);
} sponsor_t;

BYTE sponsor_dispatch(sponsor_t * sponsor);

BYTE sponsor_create(sponsor_t * sponsor, event_t * event, DATA_PTR state, DATA_PTR behavior, DATA_PTR * address);
BYTE sponsor_send(sponsor_t * sponsor, DATA_PTR address, DATA_PTR message);
BYTE sponsor_become(sponsor_t * sponsor, DATA_PTR behavior);
BYTE sponsor_fail(sponsor_t * sponsor, event_t * event, DATA_PTR error);

BYTE sponsor_reserve(sponsor_t * sponsor, DATA_PTR * data, WORD size);
BYTE sponsor_share(sponsor_t * sponsor, DATA_PTR * data);
BYTE sponsor_copy(sponsor_t * sponsor, DATA_PTR * data, DATA_PTR value);
BYTE sponsor_release(sponsor_t * sponsor, DATA_PTR * data);

sponsor_t * new_bounded_sponsor(WORD actors, WORD events, pool_t * work_pool);

BYTE audit_reserve(char * _file_, int _line_, sponsor_t * sponsor, DATA_PTR * data, WORD size);
BYTE audit_copy(char * _file_, int _line_, sponsor_t * sponsor, DATA_PTR * data, DATA_PTR value);
BYTE audit_release(char * _file_, int _line_, sponsor_t * sponsor, DATA_PTR * data);
DATA_PTR audit_track(char * _file_, int _line_, sponsor_t * sponsor, DATA_PTR address);
int audit_show_leaks();

#endif // _SPONSOR_H_
