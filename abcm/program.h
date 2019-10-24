/*
 * program.h -- bootstrap program
 */
#ifndef _PROGRAM_H_
#define _PROGRAM_H_

#include "bose.h"
#include "sponsor.h"

extern BYTE bootstrap[];

BYTE actor_eval(sponsor_t * sponsor, DATA_PTR expression, DATA_PTR * value);  // evaluate actor expressions (expression -> value)
BYTE actor_exec(sponsor_t * sponsor, DATA_PTR command);  // execute actor commands (action -> effects)

int run_actor_script(sponsor_t * sponsor, DATA_PTR script);
int run_actor_config(DATA_PTR item);
int run_program(DATA_PTR program);  // [ { "kind":"sponsor", ... }, ... ]

#endif // _PROGRAM_H_
