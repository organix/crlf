/*
 * program.h -- bootstrap program
 */
#ifndef _PROGRAM_H_
#define _PROGRAM_H_

#include "bose.h"

extern BYTE bootstrap[];

BYTE actor_eval(DATA_PTR expression, DATA_PTR * value);  // evaluate actor expressions (expression -> value)
BYTE actor_exec(DATA_PTR command);  // execute actor commands (action -> effects)

int run_program(DATA_PTR program);

#endif // _PROGRAM_H_
