/*
 * program.h -- bootstrap program
 */
#ifndef _PROGRAM_H_
#define _PROGRAM_H_

#include "bose.h"

extern BYTE bootstrap[];

/*
{
    "kind": "sponsor",
    "actors": <integer>,
    "events": <integer>,
    "script": [ ... ]
}
*/
int run_actor_config(DATA_PTR spec);

/*
[
    { "kind":"sponsor", ... },
    ...
]
*/
int run_program(DATA_PTR program);

#endif // _PROGRAM_H_
