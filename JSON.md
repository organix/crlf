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

```javascript
{
    "lang": "PEG",
    "ast": {
        "kind": "grammar",
        "rules": {
            "Value": {
                "kind": "alternative",
                "of": [
                    { "kind": "rule", "name": "Null" },
                    { "kind": "rule", "name": "Boolean" },
                    { "kind": "rule", "name": "Number" },
                    { "kind": "rule", "name": "String" },
                    { "kind": "rule", "name": "Array" },
                    { "kind": "rule", "name": "Object" }
                ]
            },
            ...
        }
    }
}
```

From a pattern-matching perspective, we take the set of all JSON values to be the alphabet of Terminal symbols. Also, each value belongs to exactly one of the six base types.

### `null` value

`null` is a singular unique abstract value, often used to indicate the concept of "nothing". It is equal only to itself.

```javascript
{
    "lang": "PEG",
    "ast": {
        "kind": "grammar",
        "rules": {
            ...
            "Null": { "kind": "terminal", "value": null },
            ...
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
            ...
            "Boolean": {
                "kind": "alternative",
                "of": [
                    { "kind": "terminal", "value": true },
                    { "kind": "terminal", "value": false }
                ]
            },
            ...
        }
    }
}
```

### Number type

An abstract _number_ value is an arbitrary-precision integer or decimal floating-point value. It may denote a precise (in the case of integers) or imprecise (in the case of floating-point) mathematical object. The Number type contains an arbitrarily large number of values, but cannot represent all [Rational](https://en.wikipedia.org/wiki/Rational_number) numbers, [Real](https://en.wikipedia.org/wiki/Real_number) numbers, [Complex](https://en.wikipedia.org/wiki/Complex_number) numbers, etc. Numbers are equal if they denote the same mathematical object.

```javascript
{
    "lang": "PEG",
    "ast": {
        "kind": "grammar",
        "rules": {
            ...
            "Number": { "kind": "terminal", "value": <number> },
            ...
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
            ...
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
            },
            ...
        }
    }
}
```

### Array type

An abstract _array_ value is an ordered sequence of zero or more JSON values. The Array type contains an arbitrarily large number of values, since there is no bound on the length of the array. Arrays are equal if they have the same length, and contain the same JSON values in the same order.

```javascript
{
    "lang": "PEG",
    "ast": {
        "kind": "grammar",
        "rules": {
            ...
            "Array": {
                "kind": "star",
                "expr": { "kind": "rule", "name": "Value" }
            },
            ...
        }
    }
}
```

### Object type

An abstract _object_ value is an unordered collection of zero or more name/value pairs. Names are represented as strings. Name values should be unique within a given object. The value associated with a name can be any aribtrary JSON value. The Object type contains an arbitrarily large number of values, since there is no bound on the number of name/value pairs. Objects are equal if they have the same number of name/value pairs, and the values associated with equal names are also equal.

```javascript
{
    "lang": "PEG",
    "ast": {
        "kind": "grammar",
        "rules": {
            ...
            "Object": {
                "kind": "star",
                "expr": {
                    "kind": "sequence",
                    "of": [
                        { "kind": "rule", "name": "String" },
                        { "kind": "rule", "name": "Value" }
                    ]
                }
            }
        }
    }
}
```

## JSON Functions

The JSON standards describe a function from strings to JSON values and vice-versa. We would like to describe additional useful abstract functions which operate on JSON data values. All of these are "pure" functions. They never mutate any data, they only create new values. One such function has already been described within each datatype above, the _equal_ predicate.

### `equal`

Input: `{ "first": <value>, "second": <value> }`

Output: `<boolean>`

Returns `true` if the `first` value is equal (as defined above) to the `second` value. Otherwise returns `false`.

### `lessThan`

Input: `{ "first": <number>, "second": <number> }`

Output: `<boolean>`

Returns `true` if the `first` number is numerically less than the `second` number. Otherwise returns `false`.

### `length`

Input: `<string>`

Output: `<number>`

Returns the number of characters in the input string (>= 0).

### `string->JSON`

Input: `<string>`

Success: `{ "ok": true, "value": <value> }`

Failure: `{ "ok": false, "error": <value> }`

Attempts to parse the input string as a valid JSON-encoding of an abstract value.

### `JSON->string`

Input: `<value>`

Output: `<string>`

Returns the JSON-encoded string representing the input value.

### `isNull`

Input: `<value>`

Output: `<boolean>`

Returns `true` if the input value is `null`. Otherwise returns `false`.

### `isBoolean`

Input: `<value>`

Output: `<boolean>`

Returns `true` if the input value is one of the Boolean values, `true` or `false`. Otherwise returns `false`.

### `isNumber`

Input: `<value>`

Output: `<boolean>`

Returns `true` if the input value is a number. Otherwise returns `false`.

### `isString`

Input: `<value>`

Output: `<boolean>`

Returns `true` if the input value is a string. Otherwise returns `false`.

### `isArray`

Input: `<value>`

Output: `<boolean>`

Returns `true` if the input value is an array. Otherwise returns `false`.

### `isObject`

Input: `<value>`

Output: `<boolean>`

Returns `true` if the input value is an object. Otherwise returns `false`.

### `concatenate` (string)

Input: `{ "first": <string>, "second": <string> }`

Output: `<string>`

Returns a string consisting of the characters of `first` followed by the characters of `second`.

### `concatenate` (array)

Input: `{ "first": <array>, "second": <array> }`

Output: `<array>`

Returns an array consisting of the elements of `first` followed by the elements of `second`.

### `concatenate` (object)

Input: `{ "first": <object>, "second": <object> }`

Output: `<object>`

Returns an object value computed as follows:

1. Begin with a _result_ object value equal to `first`
2. For each _name_ in the `second` object:
    1. If the same _name_ exists in `first`, replace the associated _value_ with the _value_ from _second_
    2. Otherwise, add the _name_ and associated _value_ from _second_ to the _result_
3. Return the final _result_ object value

## JSON Value Objects

Each abstract JSON value can be viewed as an object, with properties and methods. However, properties can only be JSON values, and methods may only return a JSON value (and not mutate any objects). Methods with no arguments are indistinguishable from properties. Arguments may be provided either as an _array_ of values, or a parameter _object_.

### Value methods

Some methods are common to all value types.

#### `<value>.equals[<value>]`

`true` if the target is equal to the parameter value, otherwise `false`.

#### `<value>.isNull`

`true` if the target is `null`, otherwise `false`.

#### `<value>.isBoolean`

`true` if the target is `true` or `false`, otherwise `false`.

#### `<value>.isNumber`

`true` if the target is a JSON `<number>` value, otherwise `false`.

#### `<value>.isString`

`true` if the target is a JSON `<string>` value, otherwise `false`.

#### `<value>.isArray`

`true` if the target is a JSON `<array>` value, otherwise `false`.

#### `<value>.isObject`

`true` if the target is a JSON `<object>` value, otherwise `false`.

#### `<value>.toJSON`

The `<string>` JSON-encoded representation of the target.

#### `<value>.throw[<value>]`

Signal an exceptional condition with an error `value` (see `<crlf>.try`).

### Boolean methods

#### `<boolean>.not`

If target is `true`, return `false`, otherwise `true`.

#### `<boolean>.and[<boolean>]`

If target is `false`, return `false`, otherwise return the parameter value.

#### `<boolean>.or[<boolean>]`

If target is `true`, return `true`, otherwise return the parameter value.

#### `<boolean>.if { then:<crlf>, else: <crlf> }`

If target is `true`, evaluate `then`, otherwise evaluate `else`.

### Number methods

#### `<number>.lessThan[<number>]`

`true` if target is numerically less than the parameter value, otherwise `false`.

#### `<number>.plus[<number>]`

The `<number>` that is the mathematical sum of the target and the parameter value.

### String methods

#### `<string>.length`

The `<number>` of characters in the target.

#### `<string>.parseJSON`

`{ "ok": true, "value": <value> }` on success, otherwise `{ "ok": false, "error": <value> }`.

#### `<string>.concatenate[<string>]`

The `<string>` consisting of the characters of target followed by the characters of the parameter value.

#### `<string>.extract[<number>, <number>]`

The `<string>` consisting of the characters of target from the first parameter value to the second parameter value as a 0-based half-open interval.

### Array methods

#### `<array>.length`

The `<number>` of elements in the target.

#### `<array>.concatenate[<array>]`

The `<array>` consisting of the elements of target followed by the elements of the parameter value.

#### `<array>.extract[<number>, <number>]`

The `<array>` consisting of the elements of target from the first parameter value to the second parameter value as a 0-based half-open interval.

### Object methods

#### `<object>.hasProperty[<string>]`

`true` if the target has a property whose name matches the parameter value, otherwise `false`.

#### `<object>.concatenate[<object>]`

The `<object>` consisting of the properties of target, either augmented or replaced by properties of the parameter value.

#### `<object>.extract[<string>, ...]`

The `<object>` consisting of the properties of target named by the parameter value(s).

### CRLF methods

#### `<crlf>.value`

Evaluate the target (according to `lang`) to produce a value.

#### `<crlf>.try { catch:<crlf>, default:<value> }`

Evaluate the target (according to `lang`) to produce a value. If an exception is thrown, evaluate `catch` or return `default` value.
