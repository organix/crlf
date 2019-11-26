# ABCM - Actor Byte-Code Machine

The **Actor Byte-Code Machine** is a byte-code interpreter for _Actor_ programs.
Programs are stored and executed directly from Binary Octet-Stream Encoding representation (isomorphic to JSON).

## Actor Assignment-Machine Model

An Actor's behavior can be described using a sequential model of variable assignments. This is essentially the model of contemporary processor cores. However, in order to avoid the pitfalls of shared mutable state, we only allow assignment to actor-local (private) variables. All visible effects are captured in the asynchronous messages between actors.

A _Sponsor_ plays the role of a processor core, mediating access to computational resources and executing the instructions in an Actor's behavior script (the _program_). Each message delivery event is handled as if it was an atomic transaction. No effects are visible outside the Actor until message handling is completed. Message handling may be aborted by an exception, in which case the message is effectively ignored.

A _Dictionary_ mapping names to values is the primary conceptual data structure. Each Actor maintains a persistent Dictionary of local variables, representing it's private state. Each message is a read-only _Dictionary_ from which values may be retrieved by the Actor's behavior script. Information derived from the message may be assigned to the Actor's local persistent state.

## BART Program Elements

ABCM programs are represented in a _CRLF_-style _JSON_ vocabulary called _BART_ (for **Blockly Actor Run-Time**).
[BART](https://github.com/dalnefre/blockly/tree/explicit-message-dictionary) is an implementation of an Actor Assignment Machine using the [Blockly](https://developers.google.com/blockly/) web-based visual programming editor. BART programs are encoded as BOSE representations of JSON values. The top-level value is expected to be an Array of Sponsors.

An Actor program is a sequence of _Actions_ executing within the bounds of a _Sponsor_:
```javascript
// Containers
    { "kind":"actor_sponsor", "actors":<number>, "events":<number>, "script":[<action>, ...] }
// Actions
    { "kind":"actor_send", "message":<dictionary>, "actor":<address> }
    { "kind":"actor_become", "behavior":<behavior> }
    { "kind":"actor_ignore" }
    { "kind":"actor_assign", "name":<string>, "value":<expression> }
    { "kind":"actor_fail", "error":<expression> }
    { "kind":"conditional", "args":[{ "if":<expression>, "do":[<action>, ...] }, ...] }
    { "kind":"log_print", "level":<number>, "value":<expression> }  // --DEPRECATED--
```

Typed expressions include:
```javascript
// Address Expressions
    { "kind":"actor_create", "state":<dictionary>, "behavior":<behavior> }
    { "kind":"actor_self" }
// Behavior Expressions
    { "kind":"actor_behavior", "name":<string>, "script":[<action>, ...] }
// Value Expressions
    { "kind":"actor_state", "name":<string> }
    { "kind":"dict_get", "name":<string>, "in":<dictionary> }
    { "kind":"list_get", "at":<number>, "from":<list> }
    { "kind":"expr_literal", "const":<value> }
    { "kind":"expr_operation", "name":<string>, "args":[<expression>, ...] }
// Number Expressions
    { "kind":"list_length", "of":<list> }
// Boolean Expressions
    { "kind":"actor_has_state", "name":<string> }
    { "kind":"dict_has", "name":<string>, "in":<dictionary> }
// List (Array) Expressions
    { "kind":"list_add", "value":<expression>, "at":<number>, "to":<list> }
    { "kind":"list_remove", "value":<expression>, "at":<number>, "from":<list> }
// Dictionary (Object) Expressions
    { "kind":"actor_message" }
    { "kind":"dict_bind", "name":<string>, "value":<expression>, "with":<dictionary> }
```

Primitive operations include:
```javascript
// String operations
    { "kind":"string_length", "string":<string> }
    { "kind":"string_at", "index":<number>, "string":<string> }
    { "kind":"string_insert", "index":<number>, "value":<number>, "string":<string> }
// Array operations
    { "kind":"array_length", "array":<array> }
    { "kind":"array_at", "index":<number>, "array":<array> }
    { "kind":"array_insert", "index":<number>, "value":<expression>, "array":<array> }
```

## Resource Management

ABCM programs execute within a strictly confined context, controlled by a _Sponsor_.
All resources available to a computation are provided by and mediated by the Sponsor.
At a minimum, this appears in the form of limits on `actors` and `events`.
If a computation attempts to create more `actors` than specified,
or sends more than `events` messages, the computation will be halted.
_A means for requesting more resources is needed, but not yet defined._

### Memory

During the execution of an ABCM program, additional working storage is often required.
Various policies may apply to storage requests, based on their purpose and intent.

When an _Actor_ is created, it is likely to persist beyond the end of a particular computation.
Actors may hold references to other Actors, potentially forming circular reference graphs.
Since Actor-state is mutable, the set of references it holds may change over time.
Memory for an Actor can be reclaimed when it is no longer reachable,
either through other Actors or from a pending message.

When a _Message_ is created, it persist beyond the end of the computation, unless the computation throws.
Asynchronous messages created (sent, but not yet received) are a primary _effect_ of an Actor's behavior.
Memory for a Message can be reclaimed once the message has been delivered and processed by the target Actor.

Temporary working storage may be needed during Actor message processing.
The amount of memory available is not directly specified, but is controlled by the Sponsor.
Temporary memory can be reclaimed when an Actor finishes processing a message (or throws).

An Actor's state persists between message deliveries.
It can be reclaimed when the Actor is reclaimed.
Updates to the Actor's persistent state may copy information from temporary working storage, allowing it to persist.

### Structures

Ownership of resources, especially memory, must be carefully managed.
All computations must be provided a mechanism to access the resources they need.

#### Sponsor
A root object, providing access to resource-management mechanisms for computations.

Fields:
  * _Pool_: Working-memory pool
  * _Configuration_: A collection of _Actors_ and _Events_
  * _Event_: Current message-delivery event

Methods:
  * _(none)_

#### Pool
A pool of persistent managed storage.

Fields:
  * _Capacity_: Number of `BYTE`s available

Methods:
  * _Reserve_: Allocate a number of usable `BYTE`s
  * _Copy_: Make a copy of a Value, possibly from a different pool
  * _Release_: Mark an allocation for reclamation

#### Configuration
A collection of _Actors_ and (pending) _Events_.

Fields:
  * _Actors_: All Actors contained in this Configuration
  * _Events_: Pending message-delivery Events
  * _Limits_: Configuration resource limits

Methods:
  * _Dispatch_: Attempt to deliver a pending _Event_
  * _Commit_: Apply message-delivery Effects
  * _Rollback_: Abort message-delivery and clean up Effects

#### Event
An Actor message-delivery Event.

Fields
  * _Actor_: Target of the current event
  * _Message_: Message content (Object)
  * _Effect_: Message-delivery outcome

Methods:
  * _(TBD)_

### Effect
Effects of Actor Commands triggered by a message-delivery Event.

Fields:
  * _Actors_: New Actors created
  * _Events_: New message-delivery Events
  * _Behavior_: Actor's behavior for subsequent messages
  * _State_: Actor's State updates
  * _Error_: Error value, or `null` if none

Methods:
  * _Create_: Create a new Actor
  * _Send_: Send an asynchronous Message
  * _Become_: Define subsequent Actor Behavior
  * _Assign_: Define subsequent Actor State
  * _Fail_: Signal an Error

## Binary Octet-Stream Encoding

For more-efficient network transmission, we propose an octet-stream encoding which is isomorphic to JSON.
As with ASCII text, there are multiple ways to encode the same abstract JSON value.
All valid encodings produce equivalent JSON values, although some representation details are lost.
Every valid JSON value can be represented in Binary Octet-Stream Encoding.
As far as possible, values may be round-tripped without loss of information.
Floating-point values are problematic, even in JSON, due to translation between base-10 and base-2.
Binary Octet-Stream Encoding, by itself, is **not** lossy, since values remain in base-2 representation.

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
