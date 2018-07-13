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

A small group of special values are encoded directly in a single octet. These include:

encoding     | value
-------------|-------
`2#00000000` | `null`
`2#00000001` | `true`
`2#00000010` | `false`
`2#00000011` | `0` (number zero)
`2#00000100` | `""` (empty string)
`2#00000101` | `[]` (empty array)
`2#00000110` | `{}` (empty object)
`2#xxxxx111` | extended value

Extended values occupy more than one octet. There are four types of extended value:

encoding     | type
-------------|------
`2#xxx00111` | Number
`2#xxx01111` | String
`2#xxx10111` | Array
`2#xxx11111` | Object

Extended values contain a _count_ indicating how many octets the value occupies (how many to skip if the value is ignored). If the first octet of a count is `2#00000001` through `2#11111111`, it directly represents the number of octets that follow. If the first octet of a count is `2#00000000`, then the count is contained in the next *two* octets, encoded MSB-first. However, if the MSB-octet is `2#00000000`, it is immediately followed (no LSB-octet) by an encoded Number containing the octet-count.

### Number

An extended Number begins with `2#srd00111` where _s_ is the sign, _r_ indicates a remainder, and _d_ indicates a decimal.

encoding     | meaning
-------------|---------
`2#srd00111` | Number
`2#0rd00111` | Positive number
`2#1rd00111` | Negative number, 2's-complement format (all-bits-set = `-1`)
`2#s0d00111` | No padding, all octets filled
`2#s1d00111` | Padding bits above the MSB (equal to the sign-bit)
`2#sr000111` | Integer value, no decimal-place or exponent
`2#sr100111` | Floating-point value (decimal-place and exponent)

Next is the _count_ of octets in the value, as defined above. The _value_ octets follow, LSB-first. If the _r_-bit is `1`, the _value_ is followed by a single octet containing the number of padding bits added to the MSB (useful values are `2#00000000` through `2#00000111`). Padding bits are equal to the _s_-bit (sign). If the _d_-bit is `1`, there are two Numbers encoded next. The first Number represents the bits following the decimal place. The second Number represents the power-of-10 exponent, which defines the decimal position.

### String

An extended String begins with `2#eem01111` where _ee_ indicates encoding, and _m_ indicates memoization.

encoding     | meaning
-------------|---------
`2#eem01111` | String
`2#00001111` | Raw binary octet-sequence
`2#00101111` | Memoized string reference
`2#01m01111` | UTF-8 character sequence
`2#10m01111` | UTF-16 character sequence
`2#11m01111` | Character sequence with named encoding

Next is the _count_ of octets in the value, as defined above. Unless this is a memoized string reference (`2#00101111`), in which case the octet is an index into the memoization table. The memoization table is treated as a ring-buffer, starting at `0` for each top-level Value in a stream. When the _m_-bit is `1`, an entry is stored and the current index and the index is incremented, wrapping around from `2#11111111` back to `2#00000000`. If the _ee_ value is `2#11`, the _count_ is followed by a String that names the encoding. A decoder will reject an encoding it does not recognize. If the _ee_ value is `2#10` the string value consists of octet-pairs, encoding 16-bit values MSB-first (per RFC 2781). The _value_ octets, in the specified encoding, follow. A UTF-16 encoded string value may begin with a byte-order-mark to signal MSB-first (`16#FEFF`) or LSB-first (`16#FFFE`) ordering of octets (included in the count, of course, but not in the string value).

### Array

An extended Array begins with `2#xxn10111` where _xx_ is reserved (default `2#00`), and _n_ indicates an element count.

encoding     | meaning
-------------|---------
`2#00010111` | Array
`2#00110111` | _n_-element Array

Next is the _count_ of octets, as defined above, in the entire array. If the _n_-bit is `1`, a Number of elements in the array follows. Encoded array element Values follow. The end of the array is reached when then specified number of octets have been consumed, which should corresponding to decoding the matching number of elements (if specified). A decoder may reject a mismatch.

### Object

An extended Object begins with `2#xxn11111` where _xx_ is reserved (default `2#00`), and _n_ indicates an property count.

encoding     | meaning
-------------|---------
`2#00011111` | Object
`2#00111111` | _n_-property Object

Next is the _count_ of octets, as defined above, in the entire object. If the _n_-bit is `1`, a Number of properties in the object follows. Encoded proerties follow, encoded as a String (property name) followed by an encoded Value. Note that the property name strings may be memoized, reducing the octet-count. The end of the object is reached when then specified number of octets have been consumed, which should corresponding to decoding the matching number of properties (if specified). A decoder may reject a mismatch.

### Encoding Matrix

The first (possibly only) octet encodes self-describing information about a Value.

Hi \ Lo  |`2#000`|`2#001`|`2#010`|`2#011`|`2#100`|`2#101`|`2#110`|`2#111`
---------|-------|-------|-------|-------|-------|-------|-------|-------
`2#00000`|`null` |`true` |`false`| `0`   | `""`  | `[]`  | `{}`  | +Integer/full
`2#00001`| -     | -     | -     | -     | -     | -     | -     | Binary octets
`2#00010`| -     | -     | -     | -     | -     | -     | -     | `[` ... `]`
`2#00011`| -     | -     | -     | -     | -     | -     | -     | `{` ... `}`
`2#00100`| -     | -     | -     | -     | -     | -     | -     | +Float/full
`2#00101`| -     | -     | -     | -     | -     | -     | -     | Memo string ref.
`2#00110`| -     | -     | -     | -     | -     | -     | -     | `[` _n_ `]`
`2#00111`| -     | -     | -     | -     | -     | -     | -     | `{` _n_ `}`
`2#01000`| -     | -     | -     | -     | -     | -     | -     | +Integer/part
`2#01001`| -     | -     | -     | -     | -     | -     | -     | UTF-8 string
`2#01010`| -     | -     | -     | -     | -     | -     | -     | `[` ? `]`
`2#01011`| -     | -     | -     | -     | -     | -     | -     | `{` ? `}`
`2#01100`| -     | -     | -     | -     | -     | -     | -     | +Float/part
`2#01101`| -     | -     | -     | -     | -     | -     | -     | UTF-8 string +memo
`2#01110`| -     | -     | -     | -     | -     | -     | -     | `[` ? `]`
`2#01111`| -     | -     | -     | -     | -     | -     | -     | `{` ? `}`
`2#10000`| -     | -     | -     | -     | -     | -     | -     | -Integer/full
`2#10001`| -     | -     | -     | -     | -     | -     | -     | UTF-16 string
`2#10010`| -     | -     | -     | -     | -     | -     | -     | `[` ? `]`
`2#10011`| -     | -     | -     | -     | -     | -     | -     | `{` ? `}`
`2#10100`| -     | -     | -     | -     | -     | -     | -     | -Float/full
`2#10101`| -     | -     | -     | -     | -     | -     | -     | UTF-16 string +memo
`2#10110`| -     | -     | -     | -     | -     | -     | -     | `[` ? `]`
`2#10111`| -     | -     | -     | -     | -     | -     | -     | `{` ? `}`
`2#11000`| -     | -     | -     | -     | -     | -     | -     | -Integer/part
`2#11001`| -     | -     | -     | -     | -     | -     | -     | coded string
`2#11010`| -     | -     | -     | -     | -     | -     | -     | `[` ? `]`
`2#11011`| -     | -     | -     | -     | -     | -     | -     | `{` ? `}`
`2#11100`| -     | -     | -     | -     | -     | -     | -     | -Float/part
`2#11101`| -     | -     | -     | -     | -     | -     | -     | coded string +memo
`2#11110`| -     | -     | -     | -     | -     | -     | -     | `[` ? `]`
`2#11111`| -     | -     | -     | -     | -     | -     | -     | `{` ? `}`

*This matrix is surprisingly sparse...*

### Alternate Encoding

Try again, with better packing this time...

encoding     | value
-------------|-------
`2#00000000` | `false`
`2#00000001` | `true`
`2#00000010` | `[]`
`2#00000011` | `{}`
`2#00000100` | `[`...`]`
`2#00000101` | `{`...`}`
`2#00000110` | `[` n `]`
`2#00000111` | `{` n `}`
`2#00001eem` | String
`2#00001111` | `""`
`2#0001sppp` | Integer+pad
`2#001esppp` | Decimal+exp+pad
`2#01nnnnnn` | -Integer [-1..-64]
`2#10000000` | `0`
`2#1nnnnnnn` | +Integer [0..126]
`2#11111111` | `null`

The first (possibly only) octet encodes self-describing information about a Value.

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
