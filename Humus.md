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
Evalute `expr` to produce a _block_ describing the initial behavior for the new actor. Bind `ident` in the current environment to the new actor's address.

### SEND

```javascript
{
    "kind": "send_stmt",
    "msg": <expression>,
    "to": <expression>
}
```
Evalute `msg` and `to` expressions. Send the result of `msg` to the actor address resulting from `to`.

### BECOME

```javascript
{
    "kind": "become_stmt",
    "expr": <expression>
}
```
Evaluate `expr` to produce a _block_ describing the replacement behavior for the current actor.

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
Match/unify the `left` and `right` patterns, possibly binding identifiers in the current environment.

### Pair

```javascript
{
    "kind": "stmt_pair",
    "head": <statement>,
    "tail": <statement>
}
```
Execute `head` and `tail` statements concurrently.

### Empty

```javascript
{
    "kind": "empty_stmt"
}
```
The `Pair` and `Empty` statements allow a concurrent collection of statements to be executed anywhere a single statement is allowed. The `Empty` statement has no effect.

### Expression

```javascript
{
    "kind": "expr_stmt",
    "expr": <expression>
}
```
Evalute `expr` and if the result is a _block_, execute it.

### THROW

```javascript
{
    "kind": "throw_stmt",
    "expr": <expression>
}
```
Evalute `expr` and raise an exception with the resulting value. All other accumulated effects are replaced by the exception.

## Expression

In Humus, _expressions_ are _evaluated_ to produce (immutable) _values_.

### Constant

```javascript
{
    "kind": "const_expr",
    "value": <value>
}
```
Return a constant `value`.

### Variable

```javascript
{
    "kind": "ident_expr",
    "ident": <string>
}
```
Lookup `ident` in the current environment and return the value bound to it.

### Pair

```javascript
{
    "kind": "pair_expr",
    "head": <expression>,
    "tail": <expression>
}
```
Evaluate `head` and `tail` concurrently and return a ordered-pair of their resulting values.

### (Lambda) Abstraction

```javascript
{
    "kind": "abs_expr",
    "ptrn": <pattern>,
    "body": <expression>
}
```
Return an _abstraction_ closed in the current environment.

### (Function) Application

```javascript
{
    "kind": "app_expr",
    "abs": <expression>,
    "arg": <expression>
}
```
Evaluate `abs` and `arg` concurrently. The result from `abs` must be an _abstraction_. The result from `arg` is matched against the `ptrn` in the abstraction. If successful, return the result of evaluating the abstraction `body` in the extended environment. Otherwise, return the undefined value `?`.

### CASE / OF / END

```javascript
{
    "kind": "case_expr",
    "expr": <expression>,
    "next": <choice/end>
}
```
Evaluate `expr` to produce a value to be matched. Pass the resulting value to `next`.

```javascript
{
    "kind": "case_choice",
    "ptrn": <pattern>,
    "expr": <expression>,
    "next": <choice/end>
}
```
Match the case value against `ptrn`. If successful, return the result of evaluating `expr` in the extended environment. Otherwise, pass the case value (and original environment) on to `next`.

```javascript
{
    "kind": "case_end"
}
```
Return the undefined value `?`.

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
Match/unify the `left` and `right` patterns, possibly extending the environment with new bindings. If successful, return the result of evaluating `expr` in the extended environment. Otherwise, evaluate `next` in the original environment. Note that `next` can be another `IF` expression (making it an `ELIF`), a final expression (making it an `ELSE`), or a constant expression returning undefined value `?`.

### LET / IN

```javascript
{
    "kind": "let_expr",
    "eqtn": {
        "kind": "eqtn",
        "left": <pattern>,
        "right": <pattern>
    },
    "expr": <expression>
}
```
Match/unify the `left` and `right` patterns, possibly extending the environment with new bindings. If successful, return the result of evaluating `expr` in the extended environment. Otherwise, return undefined value `?`.

### Block

```javascript
{
    "kind": "block_expr",
    "vars": [...<string>],
    "stmt": <statement>
}
```
Return a _block_ value closed in the current environment. `vars` is a list of identifiers which will be locally bound during execution of `stmt`. These variables are essentially created on entry to the block, to be bound by concurrent pattern matching actions. Data dependencies are resolved by deferring readers until a value has been written.

### Now

```javascript
{
    "kind": "now_expr"
}
```
Return the current value of the real-time clock.

### SELF

```javascript
{
    "kind": "self_expr"
}
```
Return the address of the currently-executing actor.

### NEW

```javascript
{
    "kind": "new_expr",
    "expr": <expression>
}
```
Evalute `expr` to produce a _block_ describing the initial behavior for the new actor. Return the new actor's address.

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
