# Parsing Expression Grammars

A [Parsing Expression Grammar](https://en.wikipedia.org/wiki/Parsing_expression_grammar) (PEG) is a pattern matching language.

## Syntax

```javascript
{
    "lang": "PEG",
    "ast": <grammar>
}
```

The `ast` represents an _grammar_, which contains a set of named _rules_:

```javascript
{
    "kind": "grammar",
    "rules": {
        <name>: <expression>,
        ...
    }
}
```

A primitive parsing expression may be any of the following:

  * Empty string
  * Terminal symbol
  * Rule reference
  * Sequence
  * Alternative (ordered choice)
  * Negation (look-ahead)

### Empty string

```javascript
{
    "kind": "empty"
}
```

### Terminal symbol

```javascript
{
    "kind": "terminal",
    "value": <any>
}
```

### Rule reference

```javascript
{
    "kind": "rule",
    "name": <string>
}
```

### Sequence

```javascript
{
    "kind": "sequence",
    "of": [
        <expression>,
        ...
    ]
}
```

### Alternative (ordered choice)

```javascript
{
    "kind": "alternative",
    "of": [
        <expression>,
        ...
    ]
}
```

### Negation (look-ahead)

```javascript
{
    "kind": "negation",
    "next": <expression>
}
```
