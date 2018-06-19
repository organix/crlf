# Humus - An Actor-Programming Language

[Humus](http://www.dalnefre.com/wp/humus/) is a pure Actor-based programming language.

```javascript
{
    "lang": "Humus",
    "ast": <statement>
}
```

The `ast` is a top-level Humus-language statement.

## Statement

In Humus, _statements_ are _executed_ to produce _effects_.

### CREATE

```javascript
{
    "kind": "create_stmt",
    "ident": <string>,
    "expr": <expression>
}
```

### SEND

```javascript
{
    "kind": "send_stmt",
    "msg": <expression>,
    "to": <expression>
}
```

### BECOME

```javascript
{
    "kind": "become_stmt",
    "expr": <expression>
}
```

### LET

```javascript
{
    "kind": "let_stmt",
    "eqtn": {
        "kind": "eqtn",
        "left": <pattern>,
        "right": <pattern>
    }
}
```

### Pair

```javascript
{
    "kind": "stmt_pair",
    "head": <statement>,
    "tail": <statement>
}
```

### Empty

```javascript
{
    "kind": "empty_stmt"
}
```

### Expression

```javascript
{
    "kind": "expr_stmt",
    "expr": <expression>
}
```

### THROW

```javascript
{
    "kind": "throw_stmt",
    "expr": <expression>
}
```

## Expression

In Humus, _expressions_ are _evaluated_ to produce (immutable) _values_.

### Constant

```javascript
{
    "kind": "const_expr",
    "value": <value>
}
```

### Variable

```javascript
{
    "kind": "ident_expr",
    "ident": <string>
}
```

### Pair

```javascript
{
    "kind": "pair_expr",
    "head": <expression>,
    "tail": <expression>
}
```

### (Lambda) Abstraction

```javascript
{
    "kind": "abs_expr",
    "ptrn": <pattern>,
    "body": <expression>
}
```

### (Function) Application

```javascript
{
    "kind": "app_expr",
    "abs": <expression>,
    "arg": <expression>
}
```

### CASE / OF / END

```javascript
{
    "kind": "case_expr",
    "expr": <expression>,
    "next": <choice/end>
}
```

```javascript
{
    "kind": "case_choice",
    "ptrn": <pattern>,
    "expr": <expression>,
    "next": <choice/end>
}
```

```javascript
{
    "kind": "case_end"
}
```

### IF / ELIF / ELSE

```javascript
{
    "kind": "if_expr",
    "eqtn": {
        "kind": "eqtn",
        "left": <pattern>,
        "right": <pattern>
    },
    "expr": <expression>,
    "next": <expression>
}
```

### LET / IN

```javascript
{
    "kind": "let_expr",
    "eqtn": <equation>,
    "expr": <expression>
}
```

### Block

```javascript
{
    "kind": "block_expr",
    "vars": [...<string>],
    "stmt": <statement>
}
```

### Now

```javascript
{
    "kind": "now_expr"
}
```

### SELF

```javascript
{
    "kind": "self_expr"
}
```

### NEW

```javascript
{
    "kind": "new_expr",
    "expr": <expression>
}
```

## Pattern

In Humus, _patterns_ are _matched_ (usually against _values_) to produce _bindings_.

### Constant

```javascript
{
    "kind": "const_ptrn",
    "value": <value>
}
```

### Variable

```javascript
{
    "kind": "ident_ptrn",
    "ident": <string>
}
```

### Any

```javascript
{
    "kind": "any_ptrn"
}
```

### Pair

```javascript
{
    "kind": "pair_ptrn",
    "head": <pattern>,
    "tail": <pattern>
}
```

### Value

```javascript
{
    "kind": "value_ptrn",
    "expr": <expression>
}
```

## Compact Representation

```javascript
{ "kind":"create_stmt", "ident":<string>, "expr":<expression> }
{ "kind":"send_stmt", "msg":<expression>, "to":<expression> }
{ "kind":"become_stmt", "expr":<expression> }
{ "kind":"let_stmt", "left":<pattern>, "right":<pattern> }
{ "kind":"stmt_pair", "head":<statement>, "tail":<statement> }
{ "kind":"empty_stmt" }
{ "kind":"expr_stmt", "expr":<expression> }
{ "kind":"throw_stmt", "expr":<expression> }
```
```javascript
{ "kind":"const_expr", "value":<value> }
{ "kind":"ident_expr", "ident":<string> }
{ "kind":"pair_expr", "head":<expression>, "tail":<expression> }
{ "kind":"abs_expr", "ptrn":<pattern>, "body":<expression> }
{ "kind":"app_expr", "abs":<expression>, "arg":<expression> }
{ "kind":"case_expr", "expr":<expression>, "next":<choice/end> }
{ "kind":"case_choice", "ptrn":<pattern>, "expr":<expression>, "next":<choice/end> }
{ "kind":"case_end" }
{ "kind":"if_expr", "left":<pattern>, "right":<pattern>, "expr":<expression>, "next":<expression> }
{ "kind":"let_expr", "eqtn":<equation>, "expr":<expression> }
{ "kind":"block_expr", "vars":[...<string>], "stmt":<statement> }
{ "kind":"now_expr" }
{ "kind":"self_expr" }
{ "kind":"new_expr", "expr":<expression> }
```
```javascript
{ "kind":"const_ptrn", "value":<value> }
{ "kind":"ident_ptrn", "ident":<string> }
{ "kind":"any_ptrn" }
{ "kind":"pair_ptrn", "head":<pattern>, "tail":<pattern> }
{ "kind":"value_ptrn", "expr":<expression> }
```

## Humus Stack-Machine

Input                     | Operation  | Output        | Description
--------------------------|------------|---------------|---------------------------
_none_                    | TRUE       | TRUE          | TRUE constant
_none_                    | FALSE      | FALSE         | FALSE constant
_none_                    | NIL        | NIL           | NIL constant
_none_                    | ?          | ?             | "undefined" constant
_none_                    | _number_   | _number_      | numeric constant
_none_                    | _string_   | _string_      | string constant
_none_                    | VAR _name_ | _value_       | named variable
_none_                    | [          | _none_        | begin block (quote)
_none_                    | ]          | _block_       | end block (quote)
_none_                    | (          | _none_        | begin expression (unquote)
_none_                    | )          | _value_       | end expression (unquote)
_pattern_ _block_         | \          | _function_    | (lambda) abstraction
_value(s)_... _function_  | $          | _value(s)_... | (function) application
_value(s)_...             | CASE       | _value(s)_... | begin case matching
_pattern_ _block_         | OF         | _none_        | conditional case
_none_                    | END        | _none_        | end case matching
_block_                   | CREATE     | _actor_       | create actor
_value_ _actor_           | SEND       | _none_        | send asynchonrous message
_block_                   | BECOME     | _none_        | update actor behavior
