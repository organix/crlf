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
`2#01nnnnnn` |`40`..`7F`| -Int [-64..-1]
`2#1nnnnnnn` |`80`..`FE`| +Int [0..126]

The encoding of `0` falls naturally within this range. Negative integers are represented in 2's-complement format (all-bits-set = `-1`).

Extended values (Number, String, Array, and Object) occupy more than one octet:

encoding     | hex | value          | extension
-------------|-----|----------------|------------
`2#00000100` |`04` | `[`...`]`      | size::Number ::Value\*
`2#00000101` |`05` | `{`...`}`      | size::Number (name::String ::Value)\*
`2#00000110` |`06` | `[` _count_ `]`      | size::Number count::Number ::Value\*n
`2#00000111` |`07` | `{` _count_ `}`      | size::Number count::Number (name::String ::Value)\*n
`2#00001000` |`08` | Octet\*        | size::Number bytes::Octet\*
`2#00001001` |`09` | * Memo#        | index::Octet
`2#0000101m` |`0A`..`0B`| UTF-8          | size::Number chars::Octet\*
`2#0000110m` |`0C`..`0D`| UTF-16         | size::Number (hi::Octet lo::Octet)\*
`2#00001110` |`0E` | encoded        | size::Number name::String data::Octet\*
`2#0001sppp` |`10`..`1F`| Int+pad        | size::Number int::Octet\*
`2#001usppp` |`20`..`3F`| Unum+pad       | size::Number exp::Number int::Octet\*

Extended values (except memo refs) contain a _size_ indicating how many octets the value occupies (how many to skip if the value is ignored) after the size.

### Number

An extended Number may be an Integer (`2#0001sppp`) or [Unum](https://en.wikipedia.org/wiki/Unum_(number_format)) (`2#001usppp`). The _s_ field is the sign (`0` for positive, `1` for negative). The _u_ field, if present, is uncertainty (`0` for exact, `1` for range). The _ppp_ field is the number of padding bits added to the MSB (`0` through `7`). Padding bits match the sign.

encoding     | hex | value          | extension
-------------|-----|----------------|------------
`2#00010ppp` |`10`..`17`| +Int &pad      | size::Number int::Octet\*
`2#00011ppp` |`18`..`1F`| -Int &pad      | size::Number int::Octet\*
`2#00100ppp` |`20`..`27`| exact +Unum &pad  | size::Number exp::Number bits::Octet\*
`2#00101ppp` |`28`..`2F`| exact -Unum &pad  | size::Number exp::Number bits::Octet\*
`2#00110ppp` |`30`..`37`| range +Unum &pad  | size::Number exp::Number bits::Octet\*
`2#00111ppp` |`38`..`3F`| range -Unum &pad  | size::Number exp::Number bits::Octet\*
`2#01nnnnnn` |`40`..`7F`| -Int [-64..-1] | -
`2#1nnnnnnn` |`80`..`FE`| +Int [0..126]  | -

The octets of the _int_ or _bits_ portion are stored LSB first, with the MSB padded as described above. Negative _int_ values are represented in 2's-complement format (all-bits-set = `-1`). Unums are stored in sign-magnitude format with padding bits matching the sign. The _exp_ field defines the number of bits designated for the exponent (from the MSB). The remaining bits are designated for the significand.

#### IEEE 754 Floating Point (recommendation)

A 64-bit "double-precision" [IEEE 754 floating point](https://en.wikipedia.org/wiki/IEEE_754-1985) value can be encoded without changing any of the bits in the 8-byte standard LSB-first representation. The first octet is `2#0010s001` where _s_ is a copy of the sign bit. The second octet is `2#10001001` indicating there are 9 bytes following. The third octet is `2#10001011` indicating 11 bits of exponent. The fourth through eleventh octets are the 8-byte standard representation, LSB first: `2#ffffffff` `2#ffffffff` `2#ffffffff` `2#ffffffff` `2#ffffffff` `2#ffffffff` `2#ffffeeee` `2#seeeeeee` (52-bit fraction _f_, 11-bit exponent _e_, 1-bit sign _s_).

### String

An extended String begins with `2#00001eem` where _ee_ indicates encoding, and _m_ indicates memoization.

encoding     | hex | value          | extension
-------------|-----|----------------|------------
`2#00001000` |`08` | Octet\*        | size::Number bytes::Octet\*
`2#00001001` |`09` | * Memo#        | index::Octet
`2#0000101m` |`0A`..`0B`| UTF-8          | size::Number chars::Octet\*
`2#0000110m` |`0C`..`0D`| UTF-16         | size::Number (hi::Octet lo::Octet)\*
`2#00001110` |`0E` | encoded        | size::Number name::String data::Octet\*
`2#00001111` |`0F` | `""`           | -

Next is the _size_ of the value in octets, as defined above. Unless this is a memoized string reference (`2#00001001`), in which case the next octet is an _index_ into the memoization table. The memoization table is treated as a ring-buffer, starting at `0` for each top-level Value in a stream. Memoized strings are often used for Object property names. For UTF-8 and UTF-16 values, when the _m_-bit is `1` an entry is stored at the current memo index and the index is incremented, wrapping around from `2#11111111` back to `2#00000000`. If _eem_ is `2#110`, the _size_ is followed by a String that names the encoding. A decoder will reject an encoding it does not recognize. If _ee_ is `2#10` the string value consists of octet-pairs, encoding 16-bit values MSB-first (per [RFC 2781](https://tools.ietf.org/html/rfc2781)). A UTF-16 encoded string value may begin with a byte-order-mark (included in the _size_, of course, but not in the string value) to signal MSB-first (`16#FEFF`) or LSB-first (`16#FFFE`) ordering of octets.

#### Capability (recommendation)

In applications that require the transmission of [_capabilities_](https://en.wikipedia.org/wiki/Capability-based_security), a String may be used to represent capabilities in transit. This requires some way to distinguish these capabilities from normal Strings. Our recommendation is to mark each String with a distinguishing prefix. For normal Strings, a [byte-order-mark](https://en.wikipedia.org/wiki/Byte_order_mark) is a safe choice. For capability Strings, a [data-link-escape](https://en.wikipedia.org/wiki/C0_and_C1_control_codes) (which is `2#00010000`, or `^P` in ASCII) can provide the desired semantics, interpreting the rest of the String as binary-octet data.

### Array

An extended Array may (`2#00000110`), or may not (`2#00000100`), specify an element _count_. However, a _size_ in octets is always provided for non-empty Arrays.

encoding     | hex | value          | extension
-------------|-----|----------------|------------
`2#00000010` |`02` | `[]`           | -
`2#00000100` |`04` | `[`...`]`      | size::Number ::Value\*
`2#00000110` |`06` | `[` _count_ `]`      | size::Number count::Number ::Value\*n

The end of the array is reached when then specified number of octets have been consumed, which should corresponding to decoding the matching count of elements (if specified). A decoder may reject a mismatch.

#### Homogeneous Arrays (proposal)

If all the values in an Array are encoded the same way, some efficiency and compactness may be gained by eliding the encoding prefix from each value. This could be done by specifying the encoding before the octet _size_ field, along with a _value_ of `0` (since a zero-length Array would be encoded as simply `2#00000010`). Unfortunately, this complicates decoding slightly for readers that just want to skip the Array, but is required to avoid ambiguity.

The encoding marker for Integer Arrays is `2#0001sppp`, followed by a Number indicating the _size_ of each value in octets, and an encoded `0` value. Usually the padding (`ppp`) will be `2#000`, since common machine-words sizes are multiples of 8 bits. However, if there is padding, it will match the sign bit (`s`).

The encoding marker for Floating-Point Arrays is `2#00100000`, followed by a Number indicating the _size_ of each value in octets, a Number indicating how many bits are _exponent_, and an encoded `+0.0` value. Note that the padding, sign, and uncertainty are all `0`, meaning each value is a multiple of 8 bits, representing an "exact" number. The sign is ignored because it is encoded as the most-significant-bit of the last byte (MSB) of each value. Note that this does not leave room for the Unum uncertainty bit in each value (maybe use the encoding marker `u`-bit to mean full-Unum?).

The encoding marker for a String Array is `2#00001ee0`, followed by `2#10000000` (indicating a _size_ of `0`), and if the encoding (`ee`) is `2#11`, a String naming the encoding. Each String value will still need to start with its own _size_ in octets, but will not have to re-state the String type and encoding.

The encoding marker for a Symbol Array (memoized Strings) is `2#00001001`. Each octet of the _value_ is an _index_ into the memoization table.

### Object

An extended Object may (`2#00000111`), or may not (`2#00000101`), specify a property _count_. However, a _size_ in octets is always provided for non-empty Objects.

encoding     | hex | value          | extension
-------------|-----|----------------|------------
`2#00000011` |`03` | `{}`           | -
`2#00000101` |`05` | `{`...`}`      | size::Number (name::String ::Value)\*
`2#00000111` |`07` | `{` _count_ `}`      | size::Number count::Number (name::String ::Value)\*n

Properties are encoded as a String (property name) followed by an encoded Value. Note that the property name strings may be memoized, reducing the octet-size of the object. The end of the object is reached when then specified number of octets have been consumed, which should corresponding to decoding the matching count of properties (if specified). A decoder may reject a mismatch.

### Encoding Matrix

The following table summarizes the meaning of the first octet in a Value:

Hi \ Lo   | `2#_000` | `2#_001` | `2#_010` | `2#_011` | `2#_100` | `2#_101` | `2#_110` | `2#_111`
----------|----------|----------|----------|----------|----------|----------|----------|----------
`2#00000_`| `false`  | `true`   | `[]`     | `{}`     |`[`...`]` |`{`...`}` |`[` _n_ `]`|`{` _n_ `}`
`2#00001_`| octets   | * memo#  | UTF-8    | UTF-8*   | UTF-16   | UTF-16*  | encoded  | `""`
`2#00010_`| +Int &0  | +Int &1  | +Int &2  | +Int &3  | +Int &4  | +Int &5  | +Int &6  | +Int &7
`2#00011_`| -Int &0  | -Int &1  | -Int &2  | -Int &3  | -Int &4  | -Int &5  | -Int &6  | -Int &7
`2#00100_`| +Flt &0  | +Flt &1  | +Flt &2  | +Flt &3  | +Flt &4  | +Flt &5  | +Flt &6  | +Flt &7
`2#00101_`| -Flt &0  | -Flt &1  | -Flt &2  | -Flt &3  | -Flt &4  | -Flt &5  | -Flt &6  | -Flt &7
`2#00110_`| +Rng &0  | +Rng &1  | +Rng &2  | +Rng &3  | +Rng &4  | +Rng &5  | +Rng &6  | +Rng &7
`2#00111_`| -Rng &0  | -Rng &1  | -Rng &2  | -Rng &3  | -Rng &4  | -Rng &5  | -Rng &6  | -Rng &7
`2#01000_`| `-64`    | `-63`    | `-62`    | `-61`    | `-60`    | `-59`    | `-58`    | `-57`
`2#01001_`| `-56`    | `-55`    | `-54`    | `-53`    | `-52`    | `-51`    | `-50`    | `-49`
`2#01010_`| `-48`    | `-47`    | `-46`    | `-45`    | `-44`    | `-43`    | `-42`    | `-41`
`2#01011_`| `-40`    | `-39`    | `-38`    | `-37`    | `-36`    | `-35`    | `-34`    | `-33`
`2#01100_`| `-32`    | `-31`    | `-30`    | `-29`    | `-28`    | `-27`    | `-26`    | `-25`
`2#01101_`| `-24`    | `-23`    | `-22`    | `-21`    | `-20`    | `-19`    | `-18`    | `-17`
`2#01110_`| `-16`    | `-15`    | `-14`    | `-13`    | `-12`    | `-11`    | `-10`    | `-9`
`2#01111_`| `-8`     | `-7`     | `-6`     | `-5`     | `-4`     | `-3`     | `-2`     | `-1`
`2#10000_`| `0`      | `1`      | `2`      | `3`      | `4`      | `5`      | `6`      | `7`
`2#10001_`| `8`      | `9`      | `10`     | `11`     | `12`     | `13`     | `14`     | `15`
`2#10010_`| `16`     | `17`     | `18`     | `19`     | `20`     | `21`     | `22`     | `23`
`2#10011_`| `24`     | `25`     | `26`     | `27`     | `28`     | `29`     | `30`     | `31`
`2#10100_`| `32`     | `33`     | `34`     | `35`     | `36`     | `37`     | `38`     | `39`
`2#10101_`| `40`     | `41`     | `42`     | `43`     | `44`     | `45`     | `46`     | `47`
`2#10110_`| `48`     | `49`     | `50`     | `51`     | `52`     | `53`     | `54`     | `55`
`2#10111_`| `56`     | `57`     | `58`     | `59`     | `60`     | `61`     | `62`     | `63`
`2#11000_`| `64`     | `65`     | `66`     | `67`     | `68`     | `69`     | `70`     | `71`
`2#11001_`| `72`     | `73`     | `74`     | `75`     | `76`     | `77`     | `78`     | `79`
`2#11010_`| `80`     | `81`     | `82`     | `83`     | `84`     | `85`     | `86`     | `87`
`2#11011_`| `88`     | `89`     | `90`     | `91`     | `92`     | `93`     | `94`     | `95`
`2#11100_`| `96`     | `97`     | `98`     | `99`     | `100`    | `101`    | `102`    | `103`
`2#11101_`| `104`    | `105`    | `106`    | `107`    | `108`    | `109`    | `110`    | `111`
`2#11110_`| `112`    | `113`    | `114`    | `115`    | `116`    | `117`    | `118`    | `119`
`2#11111_`| `120`    | `121`    | `122`    | `123`    | `124`    | `125`    | `126`    | `null`

### Comparison to CBOR

The [Concise Binary Object Representation](https://tools.ietf.org/html/rfc7049) (CBOR) is a data format whose design goals include the possibility of extremely small code size, fairly small message size, and extensibility without the need for version negotiation. It has similar design goals, but makes different implementation decisions.

