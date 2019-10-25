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

int run_actor_script(sponsor_t * sponsor, event_t * event);
int run_actor_config(DATA_PTR item);
int run_program(DATA_PTR program);  // [ { "kind":"sponsor", ... }, ... ]

#endif // _PROGRAM_H_
