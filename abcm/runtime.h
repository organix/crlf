/*
 * runtime.h -- runtime execution/evaluation engine
 */
#ifndef _RUNTIME_H_
#define _RUNTIME_H_

#include "bose.h"
#include "event.h"

// evaluate actor expressions (expression -> value)
BYTE actor_eval(event_t * event, DATA_PTR expression, DATA_PTR * value);

// execute actor commands (action -> effects)
BYTE actor_exec(event_t * event, DATA_PTR command);

// execute actor script (array of commands)
BYTE script_exec(event_t * event, DATA_PTR script);

#endif // _RUNTIME_H_
