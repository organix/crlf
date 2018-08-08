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
                    "kind": "range",
                    "from": 0,
                    "to": <maximum-Unicode-value>
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

An abstract _object_ value is an unordered collection of zero or more name/value pairs. Names are represented as strings. Name values should be unique within a given object. The value associated with a name can be any aribtrary JSON value. The Object type contains an arbitrarily large number of values, since there is no bound on the number of name/value pairs. Objects are equal if they have the same number of name/value pairs, and for each property name the associated values are also equal.

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

----

## Binary Octet-Stream Encoding

For more-efficient network transmission, we propose an octet-stream encoding which is isomorphic to JSON.

There are six types of abstract data values representable in JSON:
 * Null
 * Boolean
 * Number
 * String
 * Array
 * Object

A small set of special values are encoded directly in a single octet:

encoding     | hex | value
-------------|-----|----------------
`2#00000000` |`00` | `false`
`2#00000001` |`01` | `true`
`2#00000010` |`02` | `[]`
`2#00000011` |`03` | `{}`
`2#00001111` |`0F` | `""`
`2#10000000` |`80` | `0`
`2#11111111` |`FF` | `null`

Small integer values (from `-64` through `126`) are also encoded in a single octet:

encoding     | hex | value
-------------|-----|----------------
`2#01nnnnnn` |`40`..`7F`| -Int [-1..-64]
`2#1nnnnnnn` |`80`..`FE`| +Int [0..126]

The encoding of `0` falls naturally within this range. Negative integers are represented in 2's-complement format (all-bits-set = `-1`).

Extended values (Number, String, Array, and Object) occupy more than one octet:

encoding     | hex | value          | extension
-------------|-----|----------------|------------
`2#00000100` |`04` | `[`...`]`      | count::Number ::Value\*
`2#00000101` |`05` | `{`...`}`      | count::Number (name::String ::Value)\*
`2#00000110` |`06` | `[` n `]`      | count::Number n::Number ::Value\*n
`2#00000111` |`07` | `{` n `}`      | count::Number n::Number (name::String ::Value)\*n
`2#00001000` |`08` | Octet\*        | count::Number bytes::Octet\*
`2#00001001` |`09` | * Memo#        | index::Octet
`2#0000101m` |`0A`..`0B`| UTF-8          | count::Number chars::Octet\*
`2#0000110m` |`0C`..`0D`| UTF-16         | count::Number (hi::Octet lo::Octet)\*
`2#00001110` |`0E` | encoded        | count::Number name::String data::Octet\*
`2#0001sppp` |`10`..`1F`| Int+pad        | count::Number int::Octet\*
`2#0010sppp` |`20`..`2F`| Dec+pad        | count::Number int::Octet\* frac::+Int
`2#0011sppp` |`30`..`3F`| Flt+exp+pad    | count::Number int::Octet\* frac::+Int exp::Int

Extended values (except memo refs) contain a _count_ indicating how many octets the value occupies (how many to skip if the value is ignored) after the count.

### Number

An extended Number may be an Integer (`2#0001sppp`), Decimal (`2#0010sppp`), or Float (`2#0011sppp`). The _s_ field is the sign (`0` for positive, `1` for negative). The _ppp_ field is the number of padding bits added to the MSB (`0` through `7`). Padding bits match the sign.

encoding     | hex | value          | extension
-------------|-----|----------------|------------
`2#00010ppp` |`10`..`17`| +Int &pad      | count::Number int::Octet\*
`2#00011ppp` |`18`..`1F`| -Int &pad      | count::Number int::Octet\*
`2#00100ppp` |`20`..`27`| +Dec &pad      | count::Number int::Octet\* frac::+Int
`2#00101ppp` |`28`..`2F`| -Dec &pad      | count::Number int::Octet\* frac::+Int
`2#00110ppp` |`30`..`37`| +Flt+exp &pad  | count::Number int::Octet\* frac::+Int exp::Int
`2#00111ppp` |`38`..`3F`| -Flt+exp &pad  | count::Number int::Octet\* frac::+Int exp::Int
`2#01nnnnnn` |`40`..`7F`| -Int [-1..-64] | -
`2#1nnnnnnn` |`80`..`FE`| +Int [0..126]  | -

The octets of the _int_ portion are stored LSB first, with the MSB padded as described above. Negative _int_ values are represented in 2's-complement format (all-bits-set = `-1`). 

*[WARNING: encodings for Decimal and Float are reserved, but the specification is not yet stable]* 
If the Number is a Decimal or Float, the _frac_ portion is encoded as a separate (positive Integer) Number. If the single-octet encoding is used for _frac_, the values `0`..`99` represent two decimal places. If the Number is a Float, the _exp_ portion is encoded as an additional Integer, representing the power-of-10 by which the Number is scaled. *[FIXME: how many decimal places does an extended fraction occupy?]*

### String

An extended String begins with `2#00001eem` where _ee_ indicates encoding, and _m_ indicates memoization.

encoding     | hex | value          | extension
-------------|-----|----------------|------------
`2#00001000` |`08` | Octet\*        | count::Number bytes::Octet\*
`2#00001001` |`09` | * Memo#        | index::Octet
`2#0000101m` |`0A`..`0B`| UTF-8          | count::Number chars::Octet\*
`2#0000110m` |`0C`..`0D`| UTF-16         | count::Number (hi::Octet lo::Octet)\*
`2#00001110` |`0E` | encoded        | count::Number name::String data::Octet\*
`2#00001111` |`0F` | `""`           | -

Next is the _count_ of octets in the value, as defined above. Unless this is a memoized string reference (`2#00001001`), in which case the octet is an index into the memoization table. The memoization table is treated as a ring-buffer, starting at `0` for each top-level Value in a stream. For UTF-8 and UTF-16 values, when the _m_-bit is `1` an entry is stored at the current memo index and the index is incremented, wrapping around from `2#11111111` back to `2#00000000`. If _eem_ is `2#110`, the _count_ is followed by a String that names the encoding. A decoder will reject an encoding it does not recognize. If _ee_ is `2#10` the string value consists of octet-pairs, encoding 16-bit values MSB-first (per [RFC 2781](https://tools.ietf.org/html/rfc2781)). A UTF-16 encoded string value may begin with a byte-order-mark to signal MSB-first (`16#FEFF`) or LSB-first (`16#FFFE`) ordering of octets (included in the count, of course, but not in the string value).

#### Capability (recommendation)

In applications that require the transmission of [_capabilities_](https://en.wikipedia.org/wiki/Capability-based_security), a String may be used to represent capabilities in transit. This requires some way to distinguish these capabilities from normal Strings. One recommendation is to mark each String with a distinguishing prefix. For normal Strings, a [byte-order-mark](https://en.wikipedia.org/wiki/Byte_order_mark) is a safe choice. For capability Strings, a [data-link-escape](https://en.wikipedia.org/wiki/C0_and_C1_control_codes) (which is `2#00010000` or `^P` in ASCII) can provide the desired semantics, interpreting the rest of the String a binary-octet data.

### Array

An extended Array may (`2#00000110`), or may not (`2#00000100`), specify an element count. However, a _count_ of octets is always provided for non-empty Arrays.

encoding     | hex | value          | extension
-------------|-----|----------------|------------
`2#00000010` |`02` | `[]`           | -
`2#00000100` |`04` | `[`...`]`      | count::Number ::Value\*
`2#00000110` |`06` | `[` n `]`      | count::Number n::Number ::Value\*n

The end of the array is reached when then specified number of octets have been consumed, which should corresponding to decoding the matching number of elements (if specified). A decoder may reject a mismatch.

### Object

An extended Object may (`2#00000111`), or may not (`2#00000101`), specify a property count. However, a _count_ of octets is always provided for non-empty Objects.

encoding     | hex | value          | extension
-------------|-----|----------------|------------
`2#00000011` |`03` | `{}`           | -
`2#00000101` |`05` | `{`...`}`      | count::Number (name::String ::Value)\*
`2#00000111` |`07` | `{` n `}`      | count::Number n::Number (name::String ::Value)\*n

Properties are encoded as a String (property name) followed by an encoded Value. Note that the property name strings may be memoized, reducing the octet-count. The end of the object is reached when then specified number of octets have been consumed, which should corresponding to decoding the matching number of properties (if specified). A decoder may reject a mismatch.

### Encoding Matrix

The following table summarizes the meaning of the first octet in a Value:

Hi \ Lo  | `2#000` | `2#001` | `2#010` | `2#011` | `2#100` | `2#101` | `2#110` | `2#111`
---------|---------|---------|---------|---------|---------|---------|---------|---------
`2#00000`| `false` | `true`  | `[]`    | `{}`    |`[`...`]`|`{`...`}`|`[` n `]`|`{` n `}`
`2#00001`| octets  | * memo# | UTF-8   | UTF-8*  | UTF-16  | UTF-16* | encoded | `""`
`2#00010`| +Int &0 | +Int &1 | +Int &2 | +Int &3 | +Int &4 | +Int &5 | +Int &6 | +Int &7
`2#00011`| -Int &0 | -Int &1 | -Int &2 | -Int &3 | -Int &4 | -Int &5 | -Int &6 | -Int &7
`2#00100`| +Dec &0 | +Dec &1 | +Dec &2 | +Dec &3 | +Dec &4 | +Dec &5 | +Dec &6 | +Dec &7
`2#00101`| -Dec &0 | -Dec &1 | -Dec &2 | -Dec &3 | -Dec &4 | -Dec &5 | -Dec &6 | -Dec &7
`2#00110`| +Flt &0 | +Flt &1 | +Flt &2 | +Flt &3 | +Flt &4 | +Flt &5 | +Flt &6 | +Flt &7
`2#00111`| -Flt &0 | -Flt &1 | -Flt &2 | -Flt &3 | -Flt &4 | -Flt &5 | -Flt &6 | -Flt &7
`2#01000`| `-64`   | `-63`   | `-62`   | `-61`   | `-60`   | `-59`   | `-58`   | `-57`
`2#01001`| `-56`   | `-55`   | `-54`   | `-53`   | `-52`   | `-51`   | `-50`   | `-49`
`2#01010`| `-48`   | `-47`   | `-46`   | `-45`   | `-44`   | `-43`   | `-42`   | `-41`
`2#01011`| `-40`   | `-39`   | `-38`   | `-37`   | `-36`   | `-35`   | `-34`   | `-33`
`2#01100`| `-32`   | `-31`   | `-30`   | `-29`   | `-28`   | `-27`   | `-26`   | `-25`
`2#01101`| `-24`   | `-23`   | `-22`   | `-21`   | `-20`   | `-19`   | `-18`   | `-17`
`2#01110`| `-16`   | `-15`   | `-14`   | `-13`   | `-12`   | `-11`   | `-10`   | `-9`
`2#01111`| `-8`    | `-7`    | `-6`    | `-5`    | `-4`    | `-3`    | `-2`    | `-1`
`2#10000`| `0`     | `1`     | `2`     | `3`     | `4`     | `5`     | `6`     | `7`
`2#10001`| `8`     | `9`     | `10`    | `11`    | `12`    | `13`    | `14`    | `15`
`2#10010`| `16`    | `17`    | `18`    | `19`    | `20`    | `21`    | `22`    | `23`
`2#10011`| `24`    | `25`    | `26`    | `27`    | `28`    | `29`    | `30`    | `31`
`2#10100`| `32`    | `33`    | `34`    | `35`    | `36`    | `37`    | `38`    | `39`
`2#10101`| `40`    | `41`    | `42`    | `43`    | `44`    | `45`    | `46`    | `47`
`2#10110`| `48`    | `49`    | `50`    | `51`    | `52`    | `53`    | `54`    | `55`
`2#10111`| `56`    | `57`    | `58`    | `59`    | `60`    | `61`    | `62`    | `63`
`2#11000`| `64`    | `65`    | `66`    | `67`    | `68`    | `69`    | `70`    | `71`
`2#11001`| `72`    | `73`    | `74`    | `75`    | `76`    | `77`    | `78`    | `79`
`2#11010`| `80`    | `81`    | `82`    | `83`    | `84`    | `85`    | `86`    | `87`
`2#11011`| `88`    | `89`    | `90`    | `91`    | `92`    | `93`    | `94`    | `95`
`2#11100`| `96`    | `97`    | `98`    | `99`    | `100`   | `101`   | `102`   | `103`
`2#11101`| `104`   | `105`   | `106`   | `107`   | `108`   | `109`   | `110`   | `111`
`2#11110`| `112`   | `113`   | `114`   | `115`   | `116`   | `117`   | `118`   | `119`
`2#11111`| `120`   | `121`   | `122`   | `123`   | `124`   | `125`   | `126`   | `null`

### Canonical Encoding

It is desirable to define a canonical encoding such that digital signatures will match for equivalent values.
