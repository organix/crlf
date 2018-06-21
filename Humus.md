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
