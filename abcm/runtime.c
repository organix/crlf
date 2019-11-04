/*
 * runtime.c -- runtime execution/evaluation engine
 */
#include <stddef.h>  // for NULL, size_t, et. al.
#include <assert.h>

#include "runtime.h"
#include "bose.h"
#include "pool.h"
#include "sponsor.h"
#include "event.h"
#include "string.h"
#include "array.h"
#include "object.h"
#include "equiv.h"
#include "print.h"
#include "abcm.h"

#define LOG_ALL // enable all logging
#include "log.h"

// FIXME: these should be in `number.c`, when it exists...
BYTE get_small(WORD integer, DATA_PTR * new) {
    WORD index = (64 + integer);
    if (index > (64 + 126)) return false;
    *new = (i_ + index);
    return true;
}
BYTE make_integer(WORD i, DATA_PTR * new) {
    LOG_TRACE("make_integer: i =", i);
    long n = (long)i;  // treat it as a signed value...
    DATA_PTR data;
    WORD size;
/* --- should be calling `get_small` first ---
    if ((-64 <= n) && (n <= 126)) {  // small integer range
        size = 1;
        if (!RESERVE(&data, size)) return false;  // out of memory!
        data[0] = (n + n_0);  // encode small integer value
        *new = data;
        LOG_DEBUG("make_integer: encoded small", *new);
        return true;  // success!
    }
*/
    if ((0 <= n) && (n <= 65535)) {  // positive 16-bit integer range
        size = 4;
        if (!RESERVE(&data, size)) return false;  // out of memory!
        data[0] = p_int_0;   // prefix: +integer (padding bits: 0)
        data[1] = n_2;       // size: 2 bytes
        data[2] = i & 0xFF;  //   value (LSB)
        data[3] = i >> 8;    //   value (MSB)
        *new = data;
        LOG_DEBUG("make_integer: positive 16-bit", i);
        return true;  // success!
    }
    if ((-32768 <= n) && (n <= -1)) {  // negative 16-bit integer range
        size = 4;
        if (!RESERVE(&data, size)) return false;  // out of memory!
        data[0] = m_int_1;   // prefix: -integer (padding bits: 1)
        data[1] = n_2;       // size: 2 bytes
        data[2] = n & 0xFF;  //   value (LSB)
        data[3] = n >> 8;    //   value (MSB)
        *new = data;
        LOG_DEBUG("make_integer: negative 16-bit", i);
        return true;  // success!
    }
    LOG_WARN("make_integer: value too large!", (WORD)i);
    return false;  // value too large!
}

// evaluate expression found in named object property
static BYTE property_eval(event_t * event, DATA_PTR object, DATA_PTR name, DATA_PTR * value) {
    LOG_TRACE("property_eval: object =", (WORD)object);
    DATA_PTR expression;
    if (!object_get(object, name, &expression)) {
        LOG_DEBUG("property_eval: missing property", (WORD)name);
        return false;  // missing property
    }
    if (!actor_eval(event, expression, value)) {
        LOG_WARN("property_eval: bad expression!", (WORD)expression);
        return false;  // evaluation failed!
    }
    return true;  // success!
}

// evaluate array of expressions
static BYTE array_eval(event_t * event, DATA_PTR exprs, DATA_PTR * result) {
    LOG_TRACE("array_eval: exprs", (WORD)exprs);
    WORD length;
    if (!array_length(exprs, &length)) return false;  // exprs array required!
    LOG_TRACE("array_eval: exprs.length", length);
    DATA_PTR array = a_;  // start with an empty array
    WORD i;
    for (i = 0; i < length; ++i) {
        DATA_PTR expression;
        if (!array_get(exprs, i, &expression)) return false;  // expression required!
        DATA_PTR value;
        if (!actor_eval(event, expression, &value)) {
            LOG_WARN("array_eval: bad expression!", (WORD)expression);
            IF_WARN(value_print(expression, 0));
            return false;  // evaluation failed!
        }
        if (!array_add(array, value, i, &array)) return false;  // allocation failure!
        array = TRACK(array);
    }
    *result = array;
    LOG_DEBUG("array_eval: result @", (WORD)*result);
    IF_DEBUG(value_print(*result, 0));
    return true;  // success!
}

// evaluate unary operator expression
static BYTE op_1_eval(event_t * event, DATA_PTR exprs, DATA_PTR * x) {
    WORD length;
    if (!array_length(exprs, &length)) return false;  // exprs array required!
    if (length != 1) {
        LOG_WARN("op_1_eval: must have exactly 1 argument!", length);
        return false;  // exprs array must have exactly 1 element!
    }
    DATA_PTR result;
    if (!array_eval(event, exprs, &result)) {
        LOG_WARN("op_1_eval: argument evaluation failed!", (WORD)exprs);
        IF_WARN(value_print(exprs, 0));
        return false;  // evaluation failed!
    }
    LOG_TRACE("op_1_eval: result @", (WORD)result);
    IF_TRACE(value_print(result, 0));
    if (!array_get(result, 0, x)) return false;  // x value required!
    return true;  // success!
}

// evaluate binary operator expression
static BYTE op_2_eval(event_t * event, DATA_PTR exprs, DATA_PTR * x, DATA_PTR * y) {
    WORD length;
    if (!array_length(exprs, &length)) return false;  // exprs array required!
    if (length != 2) {
        LOG_WARN("op_2_eval: must have exactly 2 arguments!", length);
        return false;  // exprs array must have exactly 2 elements!
    }
    DATA_PTR result;
    if (!array_eval(event, exprs, &result)) {
        LOG_WARN("op_2_eval: argument evaluation failed!", (WORD)exprs);
        IF_WARN(value_print(exprs, 0));
        return false;  // evaluation failed!
    }
    LOG_TRACE("op_2_eval: result @", (WORD)result);
    IF_TRACE(value_print(result, 0));
    if (!array_get(result, 0, x)) return false;  // x value required!
    if (!array_get(result, 1, y)) return false;  // y value required!
    return true;  // success!
}

BYTE op_list[] = { utf8, n_4, 'l', 'i', 's', 't' };
BYTE op_length[] = { utf8, n_9, 'l', 'e', 'n', 'g', 't', 'h', '[', '1', ']' };
BYTE op_charAt[] = { utf8, n_20, 'c', 'h', 'a', 'r', 'A', 't', '_', 'F', 'R', 'O', 'M', '_', 'S', 'T', 'A', 'R', 'T', '[', '2', ']' };
BYTE op_join[] = { utf8, n_7, 'j', 'o', 'i', 'n', '[', '*', ']' };
BYTE op_conditional[] = { utf8, n_14, 'c', 'o', 'n', 'd', 'i', 't', 'i', 'o', 'n', 'a', 'l', '[', '*', ']' };

BYTE op_EQ_2[] = { utf8, n_5, 'E', 'Q', '[', '2', ']' };
BYTE op_NEQ_2[] = { utf8, n_6, 'N', 'E', 'Q', '[', '2', ']' };
BYTE op_LT_2[] = { utf8, n_5, 'L', 'T', '[', '2', ']' };
BYTE op_LTE_2[] = { utf8, n_6, 'L', 'T', 'E', '[', '2', ']' };
BYTE op_GT_2[] = { utf8, n_5, 'G', 'T', '[', '2', ']' };
BYTE op_GTE_2[] = { utf8, n_6, 'G', 'T', 'E', '[', '2', ']' };

BYTE op_NOT_1[] = { utf8, n_6, 'N', 'O', 'T', '[', '1', ']' };
BYTE op_AND_2[] = { utf8, n_6, 'A', 'N', 'D', '[', '2', ']' };
BYTE op_OR_2[] = { utf8, n_5, 'O', 'R', '[', '2', ']' };

BYTE op_ADD_2[] = { utf8, n_6, 'A', 'D', 'D', '[', '2', ']' };
BYTE op_MINUS_2[] = { utf8, n_8, 'M', 'I', 'N', 'U', 'S', '[', '2', ']' };
BYTE op_MULTIPLY_2[] = { utf8, n_11, 'M', 'U', 'L', 'T', 'I', 'P', 'L', 'Y', '[', '2', ']' };
BYTE op_DIVIDE_2[] = { utf8, n_9, 'D', 'I', 'V', 'I', 'D', 'E', '[', '2', ']' };

// evaluate operation expression
BYTE operation_eval(event_t * event, DATA_PTR name, DATA_PTR args, DATA_PTR * result) {
    LOG_TRACE("operation_eval: name", (WORD)name);
    LOG_TRACE("operation_eval: args", (WORD)args);
    if (value_equiv(name, op_list)) {
        if (!array_eval(event, args, result)) return false;  // evaluation failed!
    } else if (value_equiv(name, op_length)) {
        DATA_PTR x;
        if (!op_1_eval(event, args, &x)) return false;  // bad arg!
        WORD length;
        if (!string_length(x, &length)) return false;  // bad length!
        if (!get_small(length, result)) {
            if (!make_integer(length, result)) return false;  // bad allocation!
            *result = TRACK(*result);
        }
    } else if (value_equiv(name, op_charAt)) {
        DATA_PTR s;
        DATA_PTR n;
        if (!op_2_eval(event, args, &s, &n)) return false;  // bad args!
        WORD i;
        if (!value_integer(n, &i)) return false;  // number required!
        --i;  // adjust for 1-based index
        if (!string_get(s, i, &i)) return false;  // get failed!
        // FIXME: Blocky semantics return a single-character String (we return a codepoint)...
        if (!get_small(i, result)) {
            if (!make_integer(i, result)) return false;  // bad allocation!
            *result = TRACK(*result);
        }
    } else if (value_equiv(name, op_join)) {
        if (!array_eval(event, args, result)) return false;  // evaluation failed!
        // TODO: join the results into a single string...
    } else if (value_equiv(name, op_conditional)) {  // FIXME: "conditional" is a command, but it _could_ be an expression...
        WORD length;
        if (!array_length(args, &length)) return false;  // args array required!
        if ((length & 1) == 0) return false;  // args array must have an even number of elements!
        WORD i;
        for (i = 0; i < length; i += 2) {  // iterate over "if" conditions...
            DATA_PTR if_expr;
            if (!array_get(args, i, &if_expr)) return false;  // if_expr required!
            DATA_PTR value;
            if (!actor_eval(event, if_expr, &value)) {
                LOG_WARN("operation_eval: bad if_expr!", (WORD)if_expr);
                return false;  // evaluation failed!
            }
            if (value_equiv(value, b_true)) {
                DATA_PTR do_expr;
                if (!array_get(args, i+1, &do_expr)) return false;  // do_expr required!
                if (!actor_eval(event, do_expr, result)) {
                    LOG_WARN("operation_eval: bad do_expr!", (WORD)do_expr);
                    return false;  // evaluation failed!
                }
                return true;  // success! -- early exit matching clause.
            }
        }
        *result = v_null;  // default value if no matching clause...
    } else if (value_equiv(name, op_EQ_2)) {
        DATA_PTR x;
        DATA_PTR y;
        if (!op_2_eval(event, args, &x, &y)) return false;  // bad args!
        *result = value_equiv(x, y) ? b_true : b_false;
    } else if (value_equiv(name, op_NEQ_2)) {
        DATA_PTR x;
        DATA_PTR y;
        if (!op_2_eval(event, args, &x, &y)) return false;  // bad args!
        *result = !value_equiv(x, y) ? b_true : b_false;
    } else if (value_equiv(name, op_LT_2)) {
        DATA_PTR x;
        DATA_PTR y;
        if (!op_2_eval(event, args, &x, &y)) return false;  // bad args!
        WORD i;
        if (!value_integer(x, &i)) return false;  // number required!
        WORD j;
        if (!value_integer(y, &j)) return false;  // number required!
        *result = ((long)i < (long)j) ? b_true : b_false;
    } else if (value_equiv(name, op_LTE_2)) {
        DATA_PTR x;
        DATA_PTR y;
        if (!op_2_eval(event, args, &x, &y)) return false;  // bad args!
        WORD i;
        if (!value_integer(x, &i)) return false;  // number required!
        WORD j;
        if (!value_integer(y, &j)) return false;  // number required!
        *result = ((long)i <= (long)j) ? b_true : b_false;
    } else if (value_equiv(name, op_GT_2)) {
        DATA_PTR x;
        DATA_PTR y;
        if (!op_2_eval(event, args, &x, &y)) return false;  // bad args!
        WORD i;
        if (!value_integer(x, &i)) return false;  // number required!
        WORD j;
        if (!value_integer(y, &j)) return false;  // number required!
        *result = ((long)i > (long)j) ? b_true : b_false;
    } else if (value_equiv(name, op_GTE_2)) {
        DATA_PTR x;
        DATA_PTR y;
        if (!op_2_eval(event, args, &x, &y)) return false;  // bad args!
        WORD i;
        if (!value_integer(x, &i)) return false;  // number required!
        WORD j;
        if (!value_integer(y, &j)) return false;  // number required!
        *result = ((long)i >= (long)j) ? b_true : b_false;
    } else if (value_equiv(name, op_NOT_1)) {
        DATA_PTR x;
        if (!op_1_eval(event, args, &x)) return false;  // bad arg!
        *result = value_equiv(x, b_false) ? b_true : b_false;
    } else if (value_equiv(name, op_AND_2)) {
        DATA_PTR x;
        DATA_PTR y;
        if (!op_2_eval(event, args, &x, &y)) return false;  // bad args!
        if (value_equiv(x, b_false)) {
            *result = x;
        } else {
            *result = y;
        }
    } else if (value_equiv(name, op_OR_2)) {
        DATA_PTR x;
        DATA_PTR y;
        if (!op_2_eval(event, args, &x, &y)) return false;  // bad args!
        if (!value_equiv(x, b_false)) {
            *result = x;
        } else {
            *result = y;
        }
    } else if (value_equiv(name, op_ADD_2)) {
        DATA_PTR x;
        DATA_PTR y;
        if (!op_2_eval(event, args, &x, &y)) return false;  // bad args!
        WORD i;
        if (!value_integer(x, &i)) return false;  // number required!
        WORD j;
        if (!value_integer(y, &j)) return false;  // number required!
        i += j;  // calculate sum...
        if (!get_small(i, result)) {
            if (!make_integer(i, result)) return false;  // bad allocation!
            *result = TRACK(*result);
        }
    } else if (value_equiv(name, op_MINUS_2)) {
        DATA_PTR x;
        DATA_PTR y;
        if (!op_2_eval(event, args, &x, &y)) return false;  // bad args!
        WORD i;
        if (!value_integer(x, &i)) return false;  // number required!
        WORD j;
        if (!value_integer(y, &j)) return false;  // number required!
        i -= j;  // calculate difference...
        if (!get_small(i, result)) {
            if (!make_integer(i, result)) return false;  // bad allocation!
            *result = TRACK(*result);
        }
    } else if (value_equiv(name, op_MULTIPLY_2)) {
        DATA_PTR x;
        DATA_PTR y;
        if (!op_2_eval(event, args, &x, &y)) return false;  // bad args!
        WORD i;
        if (!value_integer(x, &i)) return false;  // number required!
        WORD j;
        if (!value_integer(y, &j)) return false;  // number required!
        i *= j;  // calculate product...
        if (!get_small(i, result)) {
            if (!make_integer(i, result)) return false;  // bad allocation!
            *result = TRACK(*result);
        }
    } else if (value_equiv(name, op_DIVIDE_2)) {
        DATA_PTR x;
        DATA_PTR y;
        if (!op_2_eval(event, args, &x, &y)) return false;  // bad args!
        WORD i;
        if (!value_integer(x, &i)) return false;  // number required!
        WORD j;
        if (!value_integer(y, &j)) return false;  // number required!
        i /= j;  // calculate quotient...
        if (!get_small(i, result)) {
            if (!make_integer(i, result)) return false;  // bad allocation!
            *result = TRACK(*result);
        }
    } else {
        LOG_WARN("operation_eval: unknown 'name' of operation", (WORD)name);
        // FIXME: probably want to return `false` here and fail the script execution, but we just ignore it...
        *result = v_null;
    }
    return true;  // success!
}

// evaluate actor expression (expression -> value)
BYTE actor_eval(event_t * event, DATA_PTR expression, DATA_PTR * result) {
    LOG_TRACE("actor_eval: expression", (WORD)expression);
    IF_DEBUG({
        prints("  ");
        value_print(expression, 0);
    });
    DATA_PTR kind;
    if (!object_get(expression, s_kind, &kind)) {
        LOG_WARN("actor_eval: missing 'kind' property", (WORD)expression);
        return false;  // missing property
    }
    if (value_equiv(kind, k_expr_literal)) {
        // { "kind":"expr_literal", "const":<value> }
        LOG_DEBUG("actor_eval: literal expression", (WORD)kind);
        DATA_PTR constant;
        if (!object_get(expression, s_const, &constant)) {
            LOG_DEBUG("actor_eval: missing 'const' property", (WORD)expression);
            return false;  // missing property
        }
        *result = constant;
    } else if (value_equiv(kind, k_expr_operation)) {
        // { "kind":"expr_operation", "name":<string>, "args":[<expression>, ...] }
        LOG_DEBUG("actor_eval: operation expression", (WORD)kind);
        DATA_PTR name;
        if (!object_get(expression, s_name, &name)) {
            LOG_DEBUG("actor_eval: missing 'name' property", (WORD)expression);
            return false;  // missing property
        }
        // FIXME: make sure `name` is a String...
        DATA_PTR args;
        if (!object_get(expression, s_args, &args)) {
            LOG_DEBUG("actor_eval: missing 'args' property", (WORD)expression);
            return false;  // missing property
        }
        // FIXME: make sure `args` is an Array...
        if (!operation_eval(event, name, args, result)) return false;  // operation failed!
    } else if (value_equiv(kind, k_actor_has_state)) {
        // { "kind":"actor_has_state", "name":<string> }
        LOG_DEBUG("actor_eval: actor_has_state expression", (WORD)kind);
        DATA_PTR name;
        if (!object_get(expression, s_name, &name)) {
            LOG_DEBUG("actor_eval: missing 'name' property", (WORD)expression);
            return false;  // missing property
        }
        // FIXME: make sure `name` is a String...
        scope_t * scope = EFFECT_SCOPE(EVENT_EFFECT(event));
        *result = scope_has_binding(scope, name) ? b_true : b_false;
    } else if (value_equiv(kind, k_actor_state)) {
        // { "kind":"actor_state", "name":<string> }
        LOG_DEBUG("actor_eval: actor_state expression", (WORD)kind);
        DATA_PTR name;
        if (!object_get(expression, s_name, &name)) {
            LOG_DEBUG("actor_eval: missing 'name' property", (WORD)expression);
            return false;  // missing property
        }
        // FIXME: make sure `name` is a String...
        scope_t * scope = EFFECT_SCOPE(EVENT_EFFECT(event));
        if (!scope_lookup_binding(scope, name, result)) return false;  // lookup failed!
    } else if (value_equiv(kind, k_dict_has)) {
        // { "kind":"dict_has", "name":<string>, "in":<dictionary> }
        LOG_DEBUG("actor_eval: dict_has expression", (WORD)kind);
        DATA_PTR name;
        if (!object_get(expression, s_name, &name)) {
            LOG_DEBUG("actor_eval: missing 'name' property", (WORD)expression);
            return false;  // missing property
        }
        // FIXME: make sure `name` is a String...
        DATA_PTR dict;
        if (!property_eval(event, expression, s_in, &dict)) return false;  // evaluation failed!
        // FIXME: make sure `dict` is an Object...
        *result = object_has(dict, name) ? b_true : b_false;
    } else if (value_equiv(kind, k_dict_get)) {
        // { "kind":"dict_get", "name":<string>, "in":<dictionary> }
        LOG_DEBUG("actor_eval: dict_get expression", (WORD)kind);
        DATA_PTR name;
        if (!object_get(expression, s_name, &name)) {
            LOG_DEBUG("actor_eval: missing 'name' property", (WORD)expression);
            return false;  // missing property
        }
        // FIXME: make sure `name` is a String...
        DATA_PTR dict;
        if (!property_eval(event, expression, s_in, &dict)) return false;  // evaluation failed!
        // FIXME: make sure `dict` is an Object...
        BYTE cond = object_get(dict, name, result);
        if (!cond) {
            LOG_WARN("actor_eval: undefined variable!", (WORD)name);
            // FIXME: we may want to "throw" an exception here...
            *result = v_null;  // ...instead we return the `null` value.
        }
    } else if (value_equiv(kind, k_dict_bind)) {
        // { "kind":"dict_bind", "name":<string>, "value":<expression>, "with":<dictionary> }
        LOG_DEBUG("actor_eval: dict_bind expression", (WORD)kind);
        DATA_PTR name;
        if (!object_get(expression, s_name, &name)) {
            LOG_DEBUG("actor_eval: missing 'name' property", (WORD)expression);
            return false;  // missing property
        }
        // FIXME: make sure `name` is a String...
        DATA_PTR value;
        if (!property_eval(event, expression, s_value, &value)) return false;  // evaluation failed!
        DATA_PTR dict;
        if (!property_eval(event, expression, s_with, &dict)) return false;  // evaluation failed!
        // FIXME: make sure `dict` is an Object...
        if (!object_add(dict, name, value, result)) return false;  // allocation failure!
        *result = TRACK(*result);
    } else if (value_equiv(kind, k_actor_message)) {
        // { "kind":"actor_message" }
        LOG_DEBUG("actor_eval: actor_message expression", (WORD)kind);
        *result = EVENT_MESSAGE(event);
    } else if (value_equiv(kind, k_actor_self)) {
        // { "kind":"actor_self" }
        LOG_DEBUG("actor_eval: actor_self expression", (WORD)kind);
        *result = ACTOR_SELF(EVENT_ACTOR(event));
    } else if (value_equiv(kind, k_actor_create)) {
        // { "kind":"actor_create", "state":<dictionary>, "behavior":<behavior> }
        LOG_DEBUG("actor_eval: actor_create expression", (WORD)kind);
        DATA_PTR state;
        if (!property_eval(event, expression, s_state, &state)) return false;  // evaluation failed!
        DATA_PTR behavior;
        if (!property_eval(event, expression, s_behavior, &behavior)) return false;  // evaluation failed!
        DATA_PTR address;
        if (!effect_create(EVENT_EFFECT(event), state, behavior, &address)) {
            LOG_WARN("actor_eval: create failed!", (WORD)expression);
            return false;  // create failed!
        }
        *result = address;
    } else if (value_equiv(kind, k_actor_behavior)) {
        // { "kind":"actor_behavior", "name":<string>, "script":[<action>, ...] }
        LOG_DEBUG("actor_eval: actor_behavior expression", (WORD)kind);
        // self-evaluating expression
        *result = expression;
    } else {
        LOG_WARN("actor_eval: unknown 'kind' of expression", (WORD)kind);
        // FIXME: probably want to return `false` here and fail the script execution, but we just ignore it...
        *result = v_null;
    }
    LOG_TRACE("actor_eval: result", (WORD)(*result));
    IF_DEBUG({
        prints("  -> ");
        value_print(*result, 0);
    });
    return true;  // success!
}

// execute actor command (action -> effects)
BYTE actor_exec(event_t * event, DATA_PTR command) {
    LOG_DEBUG("actor_exec: command", (WORD)command);
    IF_DEBUG(value_print(command, 1));
    DATA_PTR kind;
    if (!object_get(command, s_kind, &kind)) {
        LOG_WARN("actor_exec: missing 'kind' property", (WORD)command);
        return false;  // missing property
    }
    if (value_equiv(kind, k_actor_assign)) {
        // { "kind":"actor_assign", "name":<string>, "value":<expression> }
        LOG_DEBUG("actor_exec: assign action", (WORD)kind);
        DATA_PTR name;
        if (!object_get(command, s_name, &name)) {
            LOG_DEBUG("actor_exec: missing 'name' property", (WORD)command);
            return false;  // missing property
        }
        // FIXME: make sure `name` is a String...
        DATA_PTR value;
        if (!property_eval(event, command, s_value, &value)) return false;  // evaluation failed!
        scope_t * scope = EFFECT_SCOPE(EVENT_EFFECT(event));
        if (!scope_update_binding(scope, name, value)) return false;  // update failed!
    } else if (value_equiv(kind, k_conditional)) {
        // { "kind":"conditional", "args":[{ "if":<expression>, "do":[<action>, ...] }, ...] }
        LOG_DEBUG("actor_exec: conditional action", (WORD)kind);
        DATA_PTR args;
        if (!object_get(command, s_args, &args)) {
            LOG_DEBUG("actor_exec: missing 'args' property", (WORD)command);
            return false;  // missing property
        }
        // FIXME: make sure `args` is an Array...
        WORD length;
        if (!array_length(args, &length)) return false;  // args array required!
        WORD i;
        for (i = 0; i < length; i += 1) {  // iterate over conditional clauses...
            DATA_PTR clause;
            if (!array_get(args, i, &clause)) return false;  // clause required!
            DATA_PTR value;
            if (!property_eval(event, clause, s_if, &value)) return false;  // evaluation failed!
            if (value_equiv(value, b_true)) {
                DATA_PTR do_expr;
                if (!object_get(clause, s_do, &do_expr)) {
                    LOG_DEBUG("actor_exec: missing 'do' property", (WORD)clause);
                    return false;  // missing property
                }
                if (!script_exec(event, do_expr)) {
                    LOG_WARN("actor_exec: bad do_expr!", (WORD)do_expr);
                    return false;  // execution failed!
                }
                return true;  // success! -- early exit on matching clause.
            }
        }
        LOG_DEBUG("actor_exec: no matching clause", i);
    } else if (value_equiv(kind, k_actor_send)) {
        // { "kind":"actor_send", "message":<dictionary>, "actor":<address> }
        LOG_DEBUG("actor_exec: send action", (WORD)kind);
        DATA_PTR address;
        if (!property_eval(event, command, s_actor, &address)) return false;  // evaluation failed!
        if (value_equiv(address, v_null)) {  // shortcut -- if address is null, don't even evaluate message...
            LOG_INFO("actor_exec: ignoring message to null.", null);
            return true;  // success! -- early exit on send-to-null.
        }
        // FIXME: make sure `actor` is a Capability...
        DATA_PTR message;
        if (!property_eval(event, command, s_message, &message)) return false;  // evaluation failed!
        if (!effect_send(EVENT_EFFECT(event), address, message)) {
            LOG_WARN("actor_exec: send failed!", (WORD)command);
            return false;  // send failed!
        }
    } else if (value_equiv(kind, k_actor_become)) {
        // { "kind":"actor_become", "behavior":<behavior> }
        LOG_DEBUG("actor_exec: become action", (WORD)kind);
        DATA_PTR behavior;
        if (!property_eval(event, command, s_behavior, &behavior)) return false;  // evaluation failed!
        if (!effect_become(EVENT_EFFECT(event), behavior)) {
            LOG_WARN("actor_exec: become failed!", (WORD)command);
            return false;  // become failed!
        }
    } else if (value_equiv(kind, k_actor_ignore)) {
        // { "kind":"actor_ignore" }
        LOG_DEBUG("actor_exec: ignore action", (WORD)kind);
    } else if (value_equiv(kind, k_actor_fail)) {
        // { "kind":"actor_fail", "error":<expression> }
        LOG_DEBUG("actor_exec: fail action", (WORD)kind);
        DATA_PTR error;
        if (!property_eval(event, command, s_error, &error)) {
            LOG_INFO("actor_exec: DOUBLE-FAULT!", (WORD)command);
            IF_WARN(value_print(command, 0));
            return false;  // evaluation failed!
        }
        LOG_WARN("actor_exec: FAIL!", (WORD)error);
        if (!effect_fail(EVENT_EFFECT(event), error)) return false;  // double fault!?
        return false;  // stop script execution...
    } else if (value_equiv(kind, k_log_print)) {
        // { "kind":"log_print", "level":<number>, "value":<expression> }  // --DEPRECATED--
        LOG_DEBUG("actor_exec: print action", (WORD)kind);
        DATA_PTR value;
        if (!property_eval(event, command, s_value, &value)) return false;  // evaluation failed!
        prints("LOG: ");
        if (!value_print(value, 1)) return false;  // print failed
    } else {
        LOG_WARN("actor_exec: unknown 'kind' of command", (WORD)kind);
        // FIXME: probably want to return `false` here and fail the script execution, but we just ignore it...
    }
    return true;  // success!
}

// execute actor script (array of commands)
BYTE script_exec(event_t * event, DATA_PTR script) {
    LOG_TRACE("script_exec: script =", (WORD)script);
    IF_TRACE(value_print(script, 1));
    WORD length;
    if (!array_length(script, &length)) {
        LOG_WARN("script_exec: script array required!", (WORD)script);
        return false;  // top-level array required!
    }
    LOG_DEBUG("script_exec: script array length", length);
    WORD i;
    for (i = 0; i < length; ++i) {
        DATA_PTR command;
        if (!array_get(script, i, &command)
        ||  !object_has(command, s_kind)) {
            LOG_WARN("script_exec: command object required!", (WORD)command);
            return false;  // command object required!
        }
        if (!actor_exec(event, command)) {
            LOG_WARN("script_exec: failed executing command!", (WORD)command);
            return false;  // failed executing command!
        }
    }
    LOG_DEBUG("script_exec: completed successfully", i);
    return true;  // success!
}
