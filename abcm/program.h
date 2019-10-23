/*
 * program.h -- bootstrap program
 */
#ifndef _PROGRAM_H_
#define _PROGRAM_H_

#define USE_PARSE_CURSOR 0 /* maintain `parse_t *` navigation through program structure */

#include "bose.h"

extern BYTE bootstrap[];

#if USE_PARSE_CURSOR
BYTE actor_eval(parse_t * parse, DATA_PTR * value_out);  // evaluate actor expressions (expression -> value)
BYTE actor_exec(parse_t * parse);  // execute actor commands (action -> effects)
#else
BYTE actor_eval(DATA_PTR expression, DATA_PTR * value);  // evaluate actor expressions (expression -> value)
BYTE actor_exec(DATA_PTR command);  // execute actor commands (action -> effects)
#endif

int run_program(DATA_PTR program);

#endif // _PROGRAM_H_
