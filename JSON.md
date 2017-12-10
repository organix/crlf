# JSON - JavaScript Object Notation

[JSON](http://www.ecma-international.org/publications/files/ECMA-ST/ECMA-404.pdf) is a portable data-interchange format which represents common value-types as a sequence of Unicode characters.

```javascript
{
    "lang": "JSON",
    "ast": <string>
}
```

The `ast` string is the JSON-encoded transportable representation of a single JSON value.

## JSON Data

A JSON value can be an _object_, _array_, _number_, _string_, `true`, `false`, or `null`. The abstract value represented by a JSON-encoded string is defined by the [standards for JSON parsers](http://ftp.ripe.net/rfc/authors/rfc8259.txt). The JSON-encoded string representing abstract values is also specified by those same standards. These provide a bi-directional representation relation between abstract values and concrete Unicode character-sequences. Many commonly-used abstract values are representable in JSON, but there are also un-representable values (such as Infinity). The [crlf project](README.md) uses the abstract values of JSON as foundation for _representation_ of abstract values in other domains.

From a pattern-matching perspective, we take the set of all JSON values to be the alphabet of Terminal symbols.

### `null` value

`null` is a singular unique abstract value, often used to indicate the concept of "nothing". It is equal only to itself.

```javascript
{
    "lang": "PEG",
    "ast": {
        "kind": "grammar",
        "rules": {
            "Null": { "kind": "terminal", "value": null }
        }
    }
}
```

### Boolean type (`true` and `false` values)

The abstract values `true` and `false` are the two values of used in [Boolean logic](https://en.wikipedia.org/wiki/Boolean_algebra). Predicates (such as comparing two values for equality) produce a Boolean-type `true` or `false` result.

```javascript
{
    "lang": "PEG",
    "ast": {
        "kind": "grammar",
        "rules": {
            "Boolean": {
                "kind": "alternative",
                "of": [
                    { "kind": "terminal", "value": true },
                    { "kind": "terminal", "value": false }
                ]
            }
        }
    }
}
```

### String type

An abstract _string_ value is an ordered sequence of zero or more Unicode characters (code points). The String type contains an arbitrarily large number of values, since there is no bound on the length of the string. Strings are equal if they have the same length, and contain the same Unicode characters in the same order.

```javascript
{
    "lang": "PEG",
    "ast": {
        "kind": "grammar",
        "rules": {
            "String": {
                "kind": "star",
                "expr": {
                    "kind": "alternative",
                    "of": [
                        { "kind": "terminal", "value": 0 },
                        { "kind": "terminal", "value": 1 },
                        ...
                        { "kind": "terminal", "value": <maximum-Unicode-value> }
                    ]
                }
            }
        }
    }
}
```

### Number type

An abstract _number_ value is an arbitrary-precision integer or decimal floating-point value. It may denote a precise (in the case of integers) or imprecise (in the case of floating-point) mathematical object. Numbers are equal if they denote the same mathematical object.

```javascript
{
    "lang": "PEG",
    "ast": {
        "kind": "grammar",
        "rules": {
            "Number": { "kind": "terminal", "value": <number> }
        }
    }
}
```

### Array type

### Object type
