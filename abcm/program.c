/*
 * program.c -- bootstrap program
 */
#include <assert.h>

#include "program.h"
#include "bose.h"
#include "sponsor.h"
#include "array.h"
#include "object.h"
#include "equiv.h"
#include "print.h"
#include "abcm.h"

#define LOG_ALL // enable all logging
#include "log.h"


BYTE bootstrap[] = {
    0x06, 0x10, 0x82, 0x4e, 0x04, 0x81, 0x07, 0x10, 0x82, 0x48, 0x04, 0x84, 0x0a, 0x84, 0x6b, 0x69,  // ···N·····H····ki
    0x6e, 0x64, 0x0a, 0x8d, 0x61, 0x63, 0x74, 0x6f, 0x72, 0x5f, 0x73, 0x70, 0x6f, 0x6e, 0x73, 0x6f,  // nd··actor_sponso
    0x72, 0x0a, 0x86, 0x61, 0x63, 0x74, 0x6f, 0x72, 0x73, 0x81, 0x0a, 0x86, 0x65, 0x76, 0x65, 0x6e,  // r··actors···even
    0x74, 0x73, 0x82, 0x0a, 0x86, 0x73, 0x63, 0x72, 0x69, 0x70, 0x74, 0x06, 0x10, 0x82, 0x13, 0x04,  // ts···script·····
    0x83, 0x07, 0x10, 0x82, 0x60, 0x02, 0x83, 0x0a, 0x84, 0x6b, 0x69, 0x6e, 0x64, 0x0a, 0x8c, 0x61,  // ····`····kind··a
    0x63, 0x74, 0x6f, 0x72, 0x5f, 0x61, 0x73, 0x73, 0x69, 0x67, 0x6e, 0x0a, 0x84, 0x6e, 0x61, 0x6d,  // ctor_assign··nam
    0x65, 0x0a, 0x81, 0x61, 0x0a, 0x85, 0x76, 0x61, 0x6c, 0x75, 0x65, 0x07, 0x10, 0x82, 0x36, 0x02,  // e··a··value···6·
    0x83, 0x0a, 0x84, 0x6b, 0x69, 0x6e, 0x64, 0x0a, 0x8c, 0x61, 0x63, 0x74, 0x6f, 0x72, 0x5f, 0x63,  // ···kind··actor_c
    0x72, 0x65, 0x61, 0x74, 0x65, 0x0a, 0x85, 0x73, 0x74, 0x61, 0x74, 0x65, 0x07, 0x10, 0x82, 0x8a,  // reate··state····
    0x00, 0x84, 0x0a, 0x84, 0x6b, 0x69, 0x6e, 0x64, 0x0a, 0x89, 0x64, 0x69, 0x63, 0x74, 0x5f, 0x62,  // ····kind··dict_b
    0x69, 0x6e, 0x64, 0x0a, 0x84, 0x6e, 0x61, 0x6d, 0x65, 0x0a, 0x84, 0x70, 0x72, 0x65, 0x76, 0x0a,  // ind··name··prev·
    0x85, 0x76, 0x61, 0x6c, 0x75, 0x65, 0x07, 0xb0, 0x83, 0x0a, 0x84, 0x6b, 0x69, 0x6e, 0x64, 0x0a,  // ·value·°···kind·
    0x8c, 0x65, 0x78, 0x70, 0x72, 0x5f, 0x6c, 0x69, 0x74, 0x65, 0x72, 0x61, 0x6c, 0x0a, 0x84, 0x74,  // ·expr_literal··t
    0x79, 0x70, 0x65, 0x0a, 0x86, 0x53, 0x74, 0x72, 0x69, 0x6e, 0x67, 0x0a, 0x85, 0x63, 0x6f, 0x6e,  // ype··String··con
    0x73, 0x74, 0x0a, 0x84, 0x6e, 0x6f, 0x6e, 0x65, 0x0a, 0x84, 0x77, 0x69, 0x74, 0x68, 0x07, 0xab,  // st··none··with·«
    0x83, 0x0a, 0x84, 0x6b, 0x69, 0x6e, 0x64, 0x0a, 0x8c, 0x65, 0x78, 0x70, 0x72, 0x5f, 0x6c, 0x69,  // ···kind··expr_li
    0x74, 0x65, 0x72, 0x61, 0x6c, 0x0a, 0x84, 0x74, 0x79, 0x70, 0x65, 0x0a, 0x86, 0x4f, 0x62, 0x6a,  // teral··type··Obj
    0x65, 0x63, 0x74, 0x0a, 0x85, 0x63, 0x6f, 0x6e, 0x73, 0x74, 0x03, 0x0a, 0x88, 0x62, 0x65, 0x68,  // ect··const···beh
    0x61, 0x76, 0x69, 0x6f, 0x72, 0x07, 0x10, 0x82, 0x7c, 0x01, 0x83, 0x0a, 0x84, 0x6b, 0x69, 0x6e,  // avior···|····kin
    0x64, 0x0a, 0x8e, 0x61, 0x63, 0x74, 0x6f, 0x72, 0x5f, 0x62, 0x65, 0x68, 0x61, 0x76, 0x69, 0x6f,  // d··actor_behavio
    0x72, 0x0a, 0x84, 0x6e, 0x61, 0x6d, 0x65, 0x0f, 0x0a, 0x86, 0x73, 0x63, 0x72, 0x69, 0x70, 0x74,  // r··name···script
    0x06, 0x10, 0x82, 0x51, 0x01, 0x82, 0x07, 0x10, 0x82, 0xe6, 0x00, 0x83, 0x0a, 0x84, 0x6b, 0x69,  // ···Q·····æ····ki
    0x6e, 0x64, 0x0a, 0x89, 0x6c, 0x6f, 0x67, 0x5f, 0x70, 0x72, 0x69, 0x6e, 0x74, 0x0a, 0x85, 0x6c,  // nd··log_print··l
    0x65, 0x76, 0x65, 0x6c, 0x81, 0x0a, 0x85, 0x76, 0x61, 0x6c, 0x75, 0x65, 0x07, 0x10, 0x82, 0xc0,  // evel···value···À
    0x00, 0x83, 0x0a, 0x84, 0x6b, 0x69, 0x6e, 0x64, 0x0a, 0x8e, 0x65, 0x78, 0x70, 0x72, 0x5f, 0x6f,  // ····kind··expr_o
    0x70, 0x65, 0x72, 0x61, 0x74, 0x69, 0x6f, 0x6e, 0x0a, 0x84, 0x6e, 0x61, 0x6d, 0x65, 0x0a, 0x87,  // peration··name··
    0x6a, 0x6f, 0x69, 0x6e, 0x5b, 0x2a, 0x5d, 0x0a, 0x84, 0x61, 0x72, 0x67, 0x73, 0x06, 0x10, 0x82,  // join[*]··args···
    0x8f, 0x00, 0x83, 0x07, 0xa0, 0x82, 0x0a, 0x84, 0x6b, 0x69, 0x6e, 0x64, 0x0a, 0x8b, 0x61, 0x63,  // ···· ···kind··ac
    0x74, 0x6f, 0x72, 0x5f, 0x73, 0x74, 0x61, 0x74, 0x65, 0x0a, 0x84, 0x6e, 0x61, 0x6d, 0x65, 0x0a,  // tor_state··name·
    0x84, 0x70, 0x72, 0x65, 0x76, 0x07, 0xaf, 0x83, 0x0a, 0x84, 0x6b, 0x69, 0x6e, 0x64, 0x0a, 0x8c,  // ·prev·¯···kind··
    0x65, 0x78, 0x70, 0x72, 0x5f, 0x6c, 0x69, 0x74, 0x65, 0x72, 0x61, 0x6c, 0x0a, 0x84, 0x74, 0x79,  // expr_literal··ty
    0x70, 0x65, 0x0a, 0x86, 0x53, 0x74, 0x72, 0x69, 0x6e, 0x67, 0x0a, 0x85, 0x63, 0x6f, 0x6e, 0x73,  // pe··String··cons
    0x74, 0x0a, 0x83, 0x20, 0x26, 0x20, 0x07, 0xb9, 0x83, 0x0a, 0x84, 0x6b, 0x69, 0x6e, 0x64, 0x0a,  // t·· & ·¹···kind·
    0x88, 0x64, 0x69, 0x63, 0x74, 0x5f, 0x67, 0x65, 0x74, 0x0a, 0x84, 0x6e, 0x61, 0x6d, 0x65, 0x0a,  // ·dict_get··name·
    0x84, 0x6e, 0x61, 0x6d, 0x65, 0x0a, 0x82, 0x69, 0x6e, 0x07, 0x96, 0x81, 0x0a, 0x84, 0x6b, 0x69,  // ·name··in·····ki
    0x6e, 0x64, 0x0a, 0x8d, 0x61, 0x63, 0x74, 0x6f, 0x72, 0x5f, 0x6d, 0x65, 0x73, 0x73, 0x61, 0x67,  // nd··actor_messag
    0x65, 0x07, 0xe3, 0x83, 0x0a, 0x84, 0x6b, 0x69, 0x6e, 0x64, 0x0a, 0x8c, 0x61, 0x63, 0x74, 0x6f,  // e·ã···kind··acto
    0x72, 0x5f, 0x61, 0x73, 0x73, 0x69, 0x67, 0x6e, 0x0a, 0x84, 0x6e, 0x61, 0x6d, 0x65, 0x0a, 0x84,  // r_assign··name··
    0x70, 0x72, 0x65, 0x76, 0x0a, 0x85, 0x76, 0x61, 0x6c, 0x75, 0x65, 0x07, 0xb9, 0x83, 0x0a, 0x84,  // prev··value·¹···
    0x6b, 0x69, 0x6e, 0x64, 0x0a, 0x88, 0x64, 0x69, 0x63, 0x74, 0x5f, 0x67, 0x65, 0x74, 0x0a, 0x84,  // kind··dict_get··
    0x6e, 0x61, 0x6d, 0x65, 0x0a, 0x84, 0x6e, 0x61, 0x6d, 0x65, 0x0a, 0x82, 0x69, 0x6e, 0x07, 0x96,  // name··name··in··
    0x81, 0x0a, 0x84, 0x6b, 0x69, 0x6e, 0x64, 0x0a, 0x8d, 0x61, 0x63, 0x74, 0x6f, 0x72, 0x5f, 0x6d,  // ···kind··actor_m
    0x65, 0x73, 0x73, 0x61, 0x67, 0x65, 0x07, 0x10, 0x82, 0xd2, 0x00, 0x83, 0x0a, 0x84, 0x6b, 0x69,  // essage···Ò····ki
    0x6e, 0x64, 0x0a, 0x8a, 0x61, 0x63, 0x74, 0x6f, 0x72, 0x5f, 0x73, 0x65, 0x6e, 0x64, 0x0a, 0x87,  // nd··actor_send··
    0x6d, 0x65, 0x73, 0x73, 0x61, 0x67, 0x65, 0x07, 0x10, 0x82, 0x8b, 0x00, 0x84, 0x0a, 0x84, 0x6b,  // message········k
    0x69, 0x6e, 0x64, 0x0a, 0x89, 0x64, 0x69, 0x63, 0x74, 0x5f, 0x62, 0x69, 0x6e, 0x64, 0x0a, 0x84,  // ind··dict_bind··
    0x6e, 0x61, 0x6d, 0x65, 0x0a, 0x84, 0x6e, 0x61, 0x6d, 0x65, 0x0a, 0x85, 0x76, 0x61, 0x6c, 0x75,  // name··name··valu
    0x65, 0x07, 0xb1, 0x83, 0x0a, 0x84, 0x6b, 0x69, 0x6e, 0x64, 0x0a, 0x8c, 0x65, 0x78, 0x70, 0x72,  // e·±···kind··expr
    0x5f, 0x6c, 0x69, 0x74, 0x65, 0x72, 0x61, 0x6c, 0x0a, 0x84, 0x74, 0x79, 0x70, 0x65, 0x0a, 0x86,  // _literal··type··
    0x53, 0x74, 0x72, 0x69, 0x6e, 0x67, 0x0a, 0x85, 0x63, 0x6f, 0x6e, 0x73, 0x74, 0x0a, 0x85, 0x66,  // String··const··f
    0x69, 0x72, 0x73, 0x74, 0x0a, 0x84, 0x77, 0x69, 0x74, 0x68, 0x07, 0xab, 0x83, 0x0a, 0x84, 0x6b,  // irst··with·«···k
    0x69, 0x6e, 0x64, 0x0a, 0x8c, 0x65, 0x78, 0x70, 0x72, 0x5f, 0x6c, 0x69, 0x74, 0x65, 0x72, 0x61,  // ind··expr_litera
    0x6c, 0x0a, 0x84, 0x74, 0x79, 0x70, 0x65, 0x0a, 0x86, 0x4f, 0x62, 0x6a, 0x65, 0x63, 0x74, 0x0a,  // l··type··Object·
    0x85, 0x63, 0x6f, 0x6e, 0x73, 0x74, 0x03, 0x0a, 0x85, 0x61, 0x63, 0x74, 0x6f, 0x72, 0x07, 0x9d,  // ·const···actor··
    0x82, 0x0a, 0x84, 0x6b, 0x69, 0x6e, 0x64, 0x0a, 0x8b, 0x61, 0x63, 0x74, 0x6f, 0x72, 0x5f, 0x73,  // ···kind··actor_s
    0x74, 0x61, 0x74, 0x65, 0x0a, 0x84, 0x6e, 0x61, 0x6d, 0x65, 0x0a, 0x81, 0x61, 0x07, 0x10, 0x82,  // tate··name··a···
    0xd1, 0x00, 0x83, 0x0a, 0x84, 0x6b, 0x69, 0x6e, 0x64, 0x0a, 0x8a, 0x61, 0x63, 0x74, 0x6f, 0x72,  // Ñ····kind··actor
    0x5f, 0x73, 0x65, 0x6e, 0x64, 0x0a, 0x87, 0x6d, 0x65, 0x73, 0x73, 0x61, 0x67, 0x65, 0x07, 0x10,  // _send··message··
    0x82, 0x8a, 0x00, 0x84, 0x0a, 0x84, 0x6b, 0x69, 0x6e, 0x64, 0x0a, 0x89, 0x64, 0x69, 0x63, 0x74,  // ······kind··dict
    0x5f, 0x62, 0x69, 0x6e, 0x64, 0x0a, 0x84, 0x6e, 0x61, 0x6d, 0x65, 0x0a, 0x84, 0x6e, 0x61, 0x6d,  // _bind··name··nam
    0x65, 0x0a, 0x85, 0x76, 0x61, 0x6c, 0x75, 0x65, 0x07, 0xb0, 0x83, 0x0a, 0x84, 0x6b, 0x69, 0x6e,  // e··value·°···kin
    0x64, 0x0a, 0x8c, 0x65, 0x78, 0x70, 0x72, 0x5f, 0x6c, 0x69, 0x74, 0x65, 0x72, 0x61, 0x6c, 0x0a,  // d··expr_literal·
    0x84, 0x74, 0x79, 0x70, 0x65, 0x0a, 0x86, 0x53, 0x74, 0x72, 0x69, 0x6e, 0x67, 0x0a, 0x85, 0x63,  // ·type··String··c
    0x6f, 0x6e, 0x73, 0x74, 0x0a, 0x84, 0x6c, 0x61, 0x73, 0x74, 0x0a, 0x84, 0x77, 0x69, 0x74, 0x68,  // onst··last··with
    0x07, 0xab, 0x83, 0x0a, 0x84, 0x6b, 0x69, 0x6e, 0x64, 0x0a, 0x8c, 0x65, 0x78, 0x70, 0x72, 0x5f,  // ·«···kind··expr_
    0x6c, 0x69, 0x74, 0x65, 0x72, 0x61, 0x6c, 0x0a, 0x84, 0x74, 0x79, 0x70, 0x65, 0x0a, 0x86, 0x4f,  // literal··type··O
    0x62, 0x6a, 0x65, 0x63, 0x74, 0x0a, 0x85, 0x63, 0x6f, 0x6e, 0x73, 0x74, 0x03, 0x0a, 0x85, 0x61,  // bject··const···a
    0x63, 0x74, 0x6f, 0x72, 0x07, 0x9d, 0x82, 0x0a, 0x84, 0x6b, 0x69, 0x6e, 0x64, 0x0a, 0x8b, 0x61,  // ctor·····kind··a
    0x63, 0x74, 0x6f, 0x72, 0x5f, 0x73, 0x74, 0x61, 0x74, 0x65, 0x0a, 0x84, 0x6e, 0x61, 0x6d, 0x65,  // ctor_state··name
    0x0a, 0x81, 0x61,                                                                                // ··a
};

// evaluate expression found in named object property
static BYTE property_eval(sponsor_t * sponsor, event_t * event, DATA_PTR object, DATA_PTR name, DATA_PTR * value) {
    LOG_TRACE("property_eval: object =", (WORD)object);
    DATA_PTR expression;
    if (!object_get(object, name, &expression)) {
        LOG_DEBUG("property_eval: missing property", (WORD)name);
        return false;  // missing property
    }
    if (!actor_eval(sponsor, event, expression, value)) {
        LOG_WARN("property_eval: bad expression!", (WORD)expression);
        return false;  // evaluation failed!
    }
    return true;  // success!
}

// evaluate array of expressions
BYTE array_eval(sponsor_t * sponsor, event_t * event, DATA_PTR exprs, DATA_PTR * result) {
    LOG_TRACE("array_eval: exprs", (WORD)exprs);
    WORD length;
    if (!array_length(exprs, &length)) return false;  // exprs array required!
    LOG_TRACE("array_eval: exprs.length", length);
    if (!COPY(result, a_)) return false;  // allocation failure!
    WORD i;
    for (i = 0; i < length; ++i) {
        DATA_PTR expression;
        if (!array_get(exprs, i, &expression)) return false;  // expression required!
        DATA_PTR value;
        if (!actor_eval(sponsor, event, expression, &value)) {
            LOG_WARN("array_eval: bad expression!", (WORD)expression);
            return false;  // evaluation failed!
        }
        DATA_PTR array;
        if (!array_add(sponsor, *result, value, i, &array)) return false;  // allocation failure!
        if (!RELEASE(result)) return false;  // reclamation failure!
        *result = TRACK(array);
    }
    return true;  // success!
}

BYTE op_list[] = { utf8, n_4, 'l', 'i', 's', 't' };
BYTE op_join[] = { utf8, n_7, 'j', 'o', 'i', 'n', '[', '*', ']' };

// evaluate operation expression
BYTE operation_eval(sponsor_t * sponsor, event_t * event, DATA_PTR name, DATA_PTR args, DATA_PTR * result) {
    LOG_TRACE("operation_eval: name", (WORD)name);
    LOG_TRACE("operation_eval: args", (WORD)args);
    if (value_equiv(name, op_list)) {
        if (!array_eval(sponsor, event, args, result)) return false;  // evaluation failed!
    } else if (value_equiv(name, op_join)) {
        if (!array_eval(sponsor, event, args, result)) return false;  // evaluation failed!
        // TODO: join the results into a single string...
    } else {
        LOG_WARN("operation_eval: unknown 'name' of operation", (WORD)name);
        // FIXME: probably want to return `false` here and fail the script execution, but we just ignore it...
        *result = v_null;
    }
    return true;  // success!
}

// evaluate actor expression (expression -> value)
BYTE actor_eval(sponsor_t * sponsor, event_t * event, DATA_PTR expression, DATA_PTR * result) {
    LOG_TRACE("actor_eval: expression", (WORD)expression);
    prints("  ");
    if (!value_print(expression, 0)) return false;  // print failed!
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
        if (!operation_eval(sponsor, event, name, args, result)) return false;  // operation failed!
    } else if (value_equiv(kind, k_actor_has_state)) {
        // { "kind":"actor_has_state", "name":<string> }
        LOG_DEBUG("actor_eval: actor_has_state expression", (WORD)kind);
        DATA_PTR name;
        if (!object_get(expression, s_name, &name)) {
            LOG_DEBUG("actor_eval: missing 'name' property", (WORD)expression);
            return false;  // missing property
        }
        // FIXME: make sure `name` is a String...
        *result = b_false;  // default value is `false`
        if (event_has_binding(sponsor, event, name)) {
            *result = b_true;  // default value is `true`
        }
    } else if (value_equiv(kind, k_actor_state)) {
        // { "kind":"actor_state", "name":<string> }
        LOG_DEBUG("actor_eval: actor_state expression", (WORD)kind);
        DATA_PTR name;
        if (!object_get(expression, s_name, &name)) {
            LOG_DEBUG("actor_eval: missing 'name' property", (WORD)expression);
            return false;  // missing property
        }
        // FIXME: make sure `name` is a String...
        if (!event_lookup_binding(sponsor, event, name, result)) return false;  // lookup failed!
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
        if (!property_eval(sponsor, event, expression, s_in, &dict)) return false;  // evaluation failed!
        // FIXME: make sure `dict` is an Object...
        *result = b_false;  // default value is `false`
        if (object_has(dict, name)) {
            *result = b_true;  // default value is `true`
        }
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
        if (!property_eval(sponsor, event, expression, s_in, &dict)) return false;  // evaluation failed!
        // FIXME: make sure `dict` is an Object...
        *result = v_null;  // default value is `null`
        if (!object_get(dict, name, result)) {
            LOG_WARN("actor_eval: undefined variable!", (WORD)name);
            // FIXME: we may want to "throw" an exception here...
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
        if (!property_eval(sponsor, event, expression, s_value, &value)) return false;  // evaluation failed!
        DATA_PTR dict;
        if (!property_eval(sponsor, event, expression, s_with, &dict)) return false;  // evaluation failed!
        // FIXME: make sure `dict` is an Object...
        *result = v_null;  // default value is `null`
        if (!object_add(sponsor, dict, name, value, result)) return false;  // allocation failure!
    } else if (value_equiv(kind, k_actor_message)) {
        // { "kind":"actor_message" }
        LOG_DEBUG("actor_eval: actor_message expression", (WORD)kind);
        if (!event_lookup_message(sponsor, event, result)) return false;  // lookup failed!
    } else if (value_equiv(kind, k_actor_self)) {
        // { "kind":"actor_self" }
        LOG_DEBUG("actor_eval: actor_self expression", (WORD)kind);
        if (!event_lookup_actor(sponsor, event, result)) return false;  // lookup failed!
    } else if (value_equiv(kind, k_actor_create)) {
        // { "kind":"actor_create", "state":<dictionary>, "behavior":<behavior> }
        LOG_DEBUG("actor_eval: actor_create expression", (WORD)kind);
        DATA_PTR state;
        if (!property_eval(sponsor, event, expression, s_state, &state)) return false;  // evaluation failed!
        DATA_PTR behavior;
        if (!property_eval(sponsor, event, expression, s_behavior, &behavior)) return false;  // evaluation failed!
        DATA_PTR address;
        if (!sponsor_create(sponsor, state, behavior, &address)) {
            LOG_WARN("actor_eval: create failed!", (WORD)expression);
            return false;  // create failed!
        }
        *result = address;
    } else if (value_equiv(kind, k_actor_behavior)) {
        // { "kind":"actor_behavior", "name":<string>, "script":[<action>, ...] }
        LOG_DEBUG("actor_eval: actor_behavior expression", (WORD)kind);
        *result = expression;  // self-evaluating expression
    } else {
        LOG_WARN("actor_eval: unknown 'kind' of expression", (WORD)kind);
        // FIXME: probably want to return `false` here and fail the script execution, but we just ignore it...
        *result = v_null;
    }
    LOG_TRACE("actor_eval: result", (WORD)(*result));
    prints("  -> ");
    if (!value_print(*result, 0)) return false;  // print failed!
    return true;  // success!
}

// execute actor command (action -> effects)
BYTE actor_exec(sponsor_t * sponsor, event_t * event, DATA_PTR command) {
    LOG_TRACE("actor_exec: command", (WORD)command);
    if (!value_print(command, 1)) return false;  // print failed!
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
        if (!property_eval(sponsor, event, command, s_value, &value)) return false;  // evaluation failed!
        //if (!value_print(value, 0)) return false;  // print failed
        if (!event_update_binding(sponsor, event, name, value)) return false;  // update failed!
    } else if (value_equiv(kind, k_actor_send)) {
        // { "kind":"actor_send", "message":<dictionary>, "actor":<address> }
        LOG_DEBUG("actor_exec: send action", (WORD)kind);
        DATA_PTR address;
        if (!property_eval(sponsor, event, command, s_actor, &address)) return false;  // evaluation failed!
        if (value_equiv(address, v_null)) {  // shortcut -- if address is null, don't even evaluate message...
            LOG_INFO("actor_exec: ignoring message to null.", null);
            return true;  // success!
        }
        // FIXME: make sure `actor` is a Capability...
        DATA_PTR message;
        if (!property_eval(sponsor, event, command, s_message, &message)) return false;  // evaluation failed!
        if (!sponsor_send(sponsor, address, message)) {
            LOG_WARN("actor_exec: send failed!", (WORD)command);
            return false;  // send failed!
        }
    } else if (value_equiv(kind, k_actor_ignore)) {
        // { "kind":"actor_ignore" }
        LOG_DEBUG("actor_exec: ignore action", (WORD)kind);
    } else if (value_equiv(kind, k_actor_fail)) {
        // { "kind":"actor_fail", "error":<expression> }
        LOG_DEBUG("actor_exec: fail action", (WORD)kind);
        DATA_PTR error;
        if (!property_eval(sponsor, event, command, s_error, &error)) {
            LOG_INFO("actor_exec: DOUBLE-FAULT!", (WORD)command);
            if (!value_print(command, 0)) return false;  // print failed
            return false;  // evaluation failed!
        }
        // FIXME: probably want this to be a WARN, and not always print the error value...
        LOG_INFO("actor_exec: FAIL!", (WORD)error);
        sponsor_fail(sponsor, error);
        return false;  // force failure!
    } else if (value_equiv(kind, k_log_print)) {
        // { "kind":"log_print", "level":<number>, "value":<expression> }  // --DEPRECATED--
        LOG_DEBUG("actor_exec: print action", (WORD)kind);
        DATA_PTR value;
        if (!property_eval(sponsor, event, command, s_value, &value)) return false;  // evaluation failed!
        if (!value_print(value, 0)) return false;  // print failed
    } else {
        LOG_WARN("actor_exec: unknown 'kind' of command", (WORD)kind);
        // FIXME: probably want to return `false` here and fail the script execution, but we just ignore it...
    }
    return true;  // success!
}

/*
 * WARNING! All the `run_...` procedures return 0 on success, and 1 on failure. (not true/false)
 */

int run_actor_script(sponsor_t * sponsor, event_t * event) {
    LOG_TRACE("run_actor_script: sponsor =", (WORD)sponsor);
    LOG_TRACE("run_actor_script: event =", (WORD)event);
    DATA_PTR behavior;
    if (!event_lookup_behavior(sponsor, event, &behavior)) return false;  // lookup failed!
    DATA_PTR script;
    if (!object_get(behavior, s_script, &script)) {
        LOG_WARN("run_actor_script: script required!", (WORD)behavior);
        return false;  // script required!
    }
    LOG_DEBUG("run_actor_script: script =", (WORD)script);
    //if (!value_print(script, 1)) return 1;  // print failed!
    WORD length;
    if (!array_length(script, &length)) {
        LOG_WARN("run_actor_script: script array required!", (WORD)script);
        return 1;  // top-level array required!
    }
    LOG_INFO("run_actor_script: script array length", length);
    WORD i;
    for (i = 0; i < length; ++i) {
        DATA_PTR command;
        if (!array_get(script, i, &command)
        ||  !object_has(command, s_kind)) {
            LOG_WARN("run_actor_script: command object required!", (WORD)command);
            return 1;  // command object required!
        }
        if (!actor_exec(sponsor, event, command)) {
            LOG_WARN("run_actor_script: failed executing command!", (WORD)command);
            return 1;  // failed executing command!
        }
    }
    LOG_INFO("run_actor_script: completed successfully", i);
    return 0;  // success!
}

int run_actor_config(DATA_PTR item) {
    LOG_TRACE("run_actor_config @", (WORD)item);
    if (!value_print(item, 1)) return 1;  // print failed!
    DATA_PTR value;
    WORD actors = 0;
    if (object_get(item, s_actors, &value)
    &&  value_integer(value, &actors)) {
        LOG_INFO("run_actor_config: actors =", actors);
    }
    WORD events = 0;
    if (object_get(item, s_events, &value)
    &&  value_integer(value, &events)) {
        LOG_INFO("run_actor_config: events =", events);
    }
    DATA_PTR script;
    if (!object_get(item, s_script, &script)) {
        LOG_WARN("run_actor_config: script required!", (WORD)item);
        return 1;  // script required!
    }
    LOG_INFO("run_actor_config: script =", (WORD)script);
    sponsor_t * sponsor = new_bounded_sponsor(actors, events, heap_pool);
    assert(sponsor);
    LOG_DEBUG("run_actor_config: sponsor =", (WORD)sponsor);
    /*
     * --WARNING-- THIS CODE HAS INTIMATE KNOWLEDGE OF THE ACTOR AND EVENT STRUCTURES
     */
    BYTE behavior_template[] = { object_n, n_30, n_2,
        utf8, n_4, 'k', 'i', 'n', 'd', utf8, n_14, 'a', 'c', 't', 'o', 'r', '_', 'b', 'e', 'h', 'a', 'v', 'i', 'o', 'r',
        utf8, n_4, 'n', 'a', 'm', 'e', string_0
    };
    DATA_PTR behavior;
    if (!object_add(sponsor, behavior_template, s_script, script, &behavior)) return false;  // allocation failure!
    behavior = TRACK(behavior);
    LOG_LEVEL(LOG_LEVEL_TRACE+1, "run_actor_config: behavior =", (WORD)behavior);
    //if (!value_print(behavior, 1)) return 1;  // print failed!
    actor_t bootstrap_actor = {
        .capability = { null },  // can't send messages to bootstrap
        .behavior = behavior
    };
    if (!COPY(&bootstrap_actor.state, o_)) return false;  // allocation failure!
    event_t bootstrap_event = {
        .actor = &bootstrap_actor
    };
    if (!COPY(&bootstrap_event.message, o_)) return false;  // allocation failure!
    if (run_actor_script(sponsor, &bootstrap_event) != 0) {
        LOG_WARN("run_actor_config: boot-script execution failed!", (WORD)&bootstrap_event);
        return 1;  // boot-script execution failed!
    }
    while (sponsor_dispatch(sponsor))  // dispatch message-events
        ;
    return 0;  // success!
}

int run_program(DATA_PTR program) {
    LOG_INFO("bootstrap", (WORD)program);
    memo_clear();
    WORD length;
    if (!array_length(program, &length)) {
        LOG_WARN("run_program: top-level array required!", (WORD)program);
        return 1;  // top-level array required!
    }
    LOG_INFO("run_program: top-level array length", length);
    for (WORD i = 0; i < length; ++i) {
        DATA_PTR item;
        DATA_PTR kind;
        if (!array_get(program, i, &item)
        ||  !object_get(item, s_kind, &kind)
        ||  !value_equiv(kind, k_actor_sponsor)) {
            LOG_WARN("run_program: sponsor object required!", (WORD)item);
            return 1;  // sponsor object required!
        }
        if (run_actor_config(item) != 0) {
            LOG_WARN("run_program: failed to start actor configuration!", (WORD)item);
            return 1;  // failed to start!
        }
    }
    return 0;  // success
}
