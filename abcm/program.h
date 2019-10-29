/*
 * program.h -- bootstrap program
 */
#ifndef _PROGRAM_H_
#define _PROGRAM_H_

#include "bose.h"
#include "sponsor.h"

extern BYTE bootstrap[];

// evaluate actor expressions (expression -> value)
BYTE actor_eval(sponsor_t * sponsor, event_t * event, DATA_PTR expression, DATA_PTR * value);
// execute actor commands (action -> effects)
BYTE actor_exec(sponsor_t * sponsor, event_t * event, DATA_PTR command);
// execute actor script (array of commands)
BYTE script_exec(sponsor_t * sponsor, event_t * event, DATA_PTR script);

int run_actor_script(sponsor_t * sponsor, event_t * event);
// { "kind":"sponsor", "actors":<integer>, "events":<integer>, "script":[ ... ] }
int run_actor_config(DATA_PTR item);
// [ { "kind":"sponsor", ... }, ... ]
int run_program(DATA_PTR program);

#endif // _PROGRAM_H_
