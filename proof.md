# Formal Proofs

This [crlf](README.md) language encodes the components of a proof.

```javascript
{
    "lang": "proof",
    "ast": <...>
}
```

## Abstract Binding Trees (abt)

Abstract Binding Trees support a _replace_ operation `<abt>.replace { "name":<string>, "abt":<abt> }` which replaces all occurances of the named variable in the target with the `abt` parameter. Note that the `sort` of the abt must match the `sort` of the variable.

### Variable

```javascript
{
    "kind": "variable",
    "sort": <string>,
    "name": <string>
}
```

Stands for an unspecified piece of syntax of a given `sort`. Variables are given meaning by substitution.

### Operator

```javascript
{
    "kind": "operator",
    "sort": <string>,
    "name": <string>,
    "argument": <object>
}
```

A combiner whose result has a given `sort`. The `argument` maps named properties to _abt_s, each with an expected _sort_.

### Abstractor

```javascript
{
    "kind": "abstractor",
    "names": <array>,
    "abt": <abt>
}
```

Binds zero or more named variables in `abt`.

## Proof Components

### Judgement

```javascript
{
    "kind": "judgement",
    "relation": <string>,
    "abt": <abt>
}
```

Asserts that `abt` has a specific property or conforms the specifed relation.

### Rule

```javascript
{
    "kind": "rule",
    "name": <string>
    "premises": <array>,
    "conclusion": <abt>
}
```

The `premises` are a list of zero or more _abt_s. The `name` is optional documentation in a derivation.

## References

* Harper, Robert. [_Practical Foundations for Programming Languages_, 2nd edition](https://www.cs.cmu.edu/~rwh/pfpl/2nded.pdf). Cambridge University Press, NY (2012).
