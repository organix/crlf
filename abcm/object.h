/*
 * object.h -- Object (Dictionary) operations
 */
#ifndef _OBJECT_H_
#define _OBJECT_H_

#include "bose.h"

/*
    { "kind":"actor_state", "name":<string> }
    { "kind":"dict_get", "name":<string>, "in":<dictionary> }
    { "kind":"actor_has_state", "name":<string> }
    { "kind":"dict_has", "name":<string>, "in":<dictionary> }
    { "kind":"actor_message" }
    { "kind":"dict_bind", "name":<string>, "value":<expression>, "with":<dictionary> }
*/

BYTE object_has(DATA_PTR object, DATA_PTR name);
BYTE object_get(DATA_PTR object, DATA_PTR name, DATA_PTR * value);

#endif // _OBJECT_H_
