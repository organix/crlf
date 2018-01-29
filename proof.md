# Formal Proofs

This [crlf](README.md) language encodes the components of a proof.

```javascript
{
    "lang": "proof",
    "ast": <abt>
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

A combiner whose result has a given `sort`. The `argument` maps named properties to _abt_'s, each with an expected _sort_.

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
    "name": <string>,
    "premises": <array>,
    "conclusion": <judgement>
}
```

The `premises` are a list of zero or more _judgement_'s. The `name` is optional documentation used in a derivation.

### Derivation

```javascript
{
    "kind": "derivation",
    "rule": <string>,
    "premises": <array>,
    "conclusion": <judgement>
}
```

A _derivation_ is tree of _rule_'s where the leaves are _judgement_'s. The `premises` are a list of zero or more _derivation_'s or _judgement_'s. The `rule` is optional documentation of the _rule_ applied.

#### Example: nat

```javascript
{
    "kind": "rule",
    "premises": [],
    "conclusion": {
        "kind": "judgement",
        "relation": "nat",
        "abt": {
            "kind": "operator",
            "sort": "Exp",
            "name": "zero",
            "argument": {}
        }
    }
}
```

```javascript
{
    "kind": "rule",
    "premises": [
        {
            "kind": "judgement",
            "relation": "nat",
            "abt": {
                "kind": "variable",
                "sort": "Exp",
                "name": "a"
            }
        }
    ],
    "conclusion": {
        "kind": "judgement",
        "relation": "nat",
        "abt": {
            "kind": "operator",
            "sort": "Exp",
            "name": "succ",
            "argument": {
                "n": {
                    "kind": "variable",
                    "sort": "Exp",
                    "name": "a"
                }
            }
        }
    }
}
```

#### Example: tree

```javascript
{
    "kind": "rule",
    "premises": [],
    "conclusion": {
        "kind": "judgement",
        "relation": "tree",
        "abt": {
            "kind": "operator",
            "sort": "Exp",
            "name": "empty",
            "argument": {}
        }
    }
}
```

```javascript
{
    "kind": "rule",
    "premises": [
        {
            "kind": "judgement",
            "relation": "tree",
            "abt": {
                "kind": "variable",
                "sort": "Exp",
                "name": "a_1"
            }
        },
        {
            "kind": "judgement",
            "relation": "tree",
            "abt": {
                "kind": "variable",
                "sort": "Exp",
                "name": "a_2"
            }
        }
    ],
    "conclusion": {
        "kind": "judgement",
        "relation": "tree",
        "abt": {
            "kind": "operator",
            "sort": "Exp",
            "name": "node",
            "argument": {
                "left": {
                    "kind": "variable",
                    "sort": "Exp",
                    "name": "a_1"
                },
                "right": {
                    "kind": "variable",
                    "sort": "Exp",
                    "name": "a_2"
                }
            }
        }
    }
}
```

## References

* Harper, Robert. [_Practical Foundations for Programming Languages_, 2nd edition](https://www.cs.cmu.edu/~rwh/pfpl/2nded.pdf). Cambridge University Press, NY (2012).
