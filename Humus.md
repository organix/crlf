# Humus - An Actor-Programming Language

[Humus](http://www.dalnefre.com/wp/humus/) is a pure Actor-based programming language.

```javascript
{
    "lang": "Humus",
    "ast": <block>
}
```

The `ast` is a top-level statement block containing Humus-language statements.

## Statement

In Humus, _statements_ are _executed_ to produce _effects_.

### SEND

```javascript
{
    "kind": "send_stmt",
    "msg": <expression>,
    "to": <expression>
}
```

### CREATE

```javascript
{
    "kind": "create_stmt",
    "ident": <string>,
    "expr": <expression>
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
    "eqtn": <equation>
}
```

## Expression

In Humus, _expressions_ are _evaluated_ to produce (immutable) _values_.

### Constant

### Variable

### Pair

### (Function) Application

## Pattern

In Humus, _patterns_ are _matched_ (usually against _values_) to produce _bindings_.

### Constant

### Variable

### Pair

### Value

