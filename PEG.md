# Parsing Expression Grammars

A [Parsing Expression Grammar](https://en.wikipedia.org/wiki/Parsing_expression_grammar) (PEG) is a pattern matching language.

## Primitive Expressions

```javascript
{
    "lang": "PEG",
    "ast": <grammar>
}
```

The `ast` represents a _grammar_, which contains a set of named _rules_:

```javascript
{
    "kind": "grammar",
    "rules": {
        <name>: <expression>,
        ...
    }
}
```

A parsing _expression_ is applied to an _source_ array value. If the expression fails to match, it produces a `false` _result_ value. If the expression matches, it produces a match _result_ object with an arbitrary _value_ and a _remainder_ array containing the unmatched portion of the source array:

```
{
    value: <any>,
    remainder: <array>
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

Matches without consuming any input. The result _value_ is `[]`.

### Terminal symbol

```javascript
{
    "kind": "terminal",
    "value": <any>
}
```

If _source_ is empty, fail. Otherwise consume the first element of _source_ and match if it equals _value_. The result _value_ is the _value_ matched.

### Rule reference

```javascript
{
    "kind": "rule",
    "name": <string>
}
```

Apply the named rule to _source_. If matched, the result _value_ is the _value_ matched by the rule.

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

Match all of the _expressions_ in _of_ to successive portions of the _source_. If matched, the result _value_ is an array of the _values_ matched by each expression.

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

Attempt to match each of the _expressions_ in _of_ in order until one matches the current _source_. If matched, the result _value_ is the _value_ matched.

### Negation (look-ahead)

```javascript
{
    "kind": "negation",
    "next": <expression>
}
```

Matches, with a result value `null` and _remainder_ = _source_, if _next_ **fails** to match.

## Derived Expressions

Many common (and useful) parsing expressions can be defined in terms of the primitives already specified.

Name | Description | Derivation
-----|-------------|-----------
anything | matches any single _value_ | alternative( _...all possible values..._ )
star(expr) | zero-or-more repetition | alternative(sequence(expr, star(expr)), empty)
plus(expr) | one-or-more repetition | sequence(expr, star(expr))
optional(expr) | zero-or-one occurance | alternative(expr, empty)
suffix(expr) | positive look-ahead | negation(negation(expr))
