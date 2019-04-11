# Actor Behavior

An actor behavior describes the effects of an actor receiving a message. The effects include new actors created, new messages sent, and the behavior to be used for handling subsequent messages. The initial value of the abstract _effects_ object is:

```
effects := {
    create: [],
    send: [],
    become: <current behavior>
}
```

## Primitive Actions

```javascript
{
    "lang": "actor",
    "ast": <behavior>
}
```

The `ast` represents a _behavior_, which contains a list of _actions_:

```javascript
{
    "kind": "behavior",
    "actions": [
        <action>,
        ...
    ]
}
```

A primitive action may be any of the following:

  * **Create** a new actor
  * **Send** a message
  * **Become** a new behavior

A new value for _effects_ is created as each primitive action is applied. The final value is a composite of all the accumulated effects.

### Create

The _create_ primitive constructs a new actor with an initial _behavior_.

```javascript
{
    "kind": "create",
    "behavior": <behavior>
}
```

The _effects_ object is augmented with the (opaque) _address_ of the newly-created actor and its initial _behavior_.

```
effects := effects.concatenate {
    create: effects.create.append {
        actor: <address>,
        behavior: <behavior>
    }
}
```

### Send

The _send_ primitive constructs a new send-event to deliver a specific _message_ to a _target_ actor.

```javascript
{
    "kind": "send",
    "target": <address>,
    "message": <value>
}
```

The _effects_ object is augmented with the new send-event.

```
effects := effects.concatenate {
    send: effects.send.append {
        target: <address>,
        message: <value>
    }
}
```

### Become

The _become_ primitive specifies a replacement _behavior_ for handling subsequent messages to the current actor.

```javascript
{
    "kind": "become",
    "behavior": <behavior>
}
```

The _effects_ object is updated with the new _behavior_.

```
effects := effects.concatenate {
    become: <behavior>
}
```

## Actor Expressions

The primitive actor actions imply the existance of several types:

  * Behavior
  * Address
  * Value

Anywhere these types are called for, there may be an _expression_ which yields an object of that type.

### Behavior

A _behavior_ describes the _actions_ of an actor in response to a _message_.

```javascript
{
    "kind": "behavior",
    "actions": [
        <action>,
        ...
    ]
}
```

In addition to the primitive _actions_ described above,
some actions have effects on the _context_ in which a _behavior_ is executing.

#### Assign

An _assign_ action associates a new _value_ with a variable _name_ in the execution _context_.

```javascript
{
    "kind": "assign",
    "name": <string>,
    "value": <value>
}
```

Each actor maintains a persistent set of bindings from _names_ to _values_.

### Address

An _address_ designates a _target_ actor to which messages may be sent. The actor _create_ primitive produces an _address_ for the newly created actor. The _SELF_ expression designates the _address_ of the currently-executing (target) actor.

### Value

Message contents are bindings of _names_ to immutable _values_ (which include actor _addresses_).

#### Variable

A _variable_ expression yields the _value_ currently associated with a variable _name_ in the execution _context_.

```javascript
{
    "kind": "variable",
    "name": <string>
}
```

When an actor receives a _message_, the bindings are concatented to the actor state, forming the execution _context_.

#### SELF

A _SELF_ expression yields the the _address_ of the currently-executing (target) actor.

```javascript
{
    "kind": "self"
}
```

_SELF_ is not a variable, but an expression that designates the _address_ of the currently-executing actor.

#### Literal

A _literal_ expression yields a specific embedded _value_ without further evaluation.

```javascript
{
    "kind": "literal",
    "value": <value>
}
```

#### Pipeline

A _pipeline_ expression evaluates a sequence of _value_ expressions, passing each successor to the accumulated _value_, yielding the final accumulated _value_.

```javascript
{
    "kind": "pipeline",
    "expressions": [
        <value>,
        ...
    ]
}
```

#### Method

**[FIXME]** : _Pipeline and Method should be replaced by a single abstraction-constructor that creates a Function which can be applied to a dictionary!_

A _method_ evaluates a _pipeline_ by inserting a parameter _value_ as the initial accumulator. A _method_ is constructed by sending `"method"` to an Array. Consider the following example:

```javascript
{
    "kind": "assign",
    "name": "succ",
    "value": {
        "kind": "pipeline",
        "expressions": [
            [
                "plus",
                1
            ],
            "method"
        ]
    }
}
```

This _statement_ defines a numeric successor method and binds it to the identifier "succ" in the current environment.

### Examples

An example of a `send` effect, as previously described:

```
effects := effects.concatenate {
    send: effects.send.append {
        target: cust,
        message: self
    }
}
```

An equivalent VO statement would be:

```javascript
{
    "kind": "assign",
    "name": "effects",
    "value": {
        "kind": "pipeline",
        "expressions": [
            { "kind": "variable", "name": "effects" },
            "concatenate",
            {
                "send": {
                    "kind": "pipeline",
                    "expressions": [
                        { "kind": "variable", "name": "effects" },
                        "value",
                        "send",
                        "append",
                        {
                            "target": { "kind": "variable", "name": "cust" },
                            "message": { "kind": "variable", "name": "self" }
                        }
                    ]
                }
            }
        ]
    }
}
```

## Actor Stack-Machine

There are many possible models for describing an actor's behavior. One simple model is an [imperative](https://en.wikipedia.org/wiki/Imperative_programming) stack-oriented machine with a dictionary (similar to [FORTH](https://en.wikipedia.org/wiki/Forth_(programming_language))).

Program source is provided as a stream of _words_ (whitespace separated in text format). Each word is looked up in the current _dictionary_ and the corresponding _block_ is executed. Literal values are pushed on the data _stack_, which is used to provide parameters and return values for executing blocks. Some blocks also consume words from the source stream.

An actor's behavior is described with a _block_. The message received by the actor is the contents of the data stack. The `SEND` primitive sends the current stack contents, clearing the stack. Values may be saved in the dictionary by binding them to a word. All dictionary changes are local to the executing behavior.

Input                | Operation       | Output                  | Description
---------------------|-----------------|-------------------------|------------
_block_              | `CREATE`        | _actor_                 | Create a new actor with _block_ behavior
..._message_ _actor_ | `SEND`          | &mdash;                 | Send _message_ to _actor_
_block_              | `BECOME`        | &mdash;                 | Replace current actor's behavior with _block_
&mdash;              | `SELF`          | _actor_                 | Push the current actor's address on the data stack
_value_              | `=` _word_      | &mdash;                 | Bind _value_ to _word_ in the current dictionary
&mdash;              | `'` _word_      | _word_                  | Push (literal) _word_ on the data stack
&mdash;              | `@` _word_      | _value_                 | Lookup _value_ bound to _word_ in the current dictionary
&mdash;              | `[` ... `]`     | _block_                 | Create block (quoted) value
[ ...                | `(` ... `)`     | [ ... _value_           | Immediate (unquoted) value
_bool_               | `IF` [ ] `ELSE` [ ] | &mdash;             | Conditional execution of blocks
_v_                  | `DROP`          | &mdash;                 | Drop the top element
_v_                  | `DUP`           | _v_ _v_                 | Duplicate the top element (same as `1 PICK`)
_v_<sub>2</sub> _v_<sub>1</sub> | `SWAP` | _v_<sub>1</sub> _v_<sub>2</sub> | Swap the top two elements (same as `2 ROLL`)
_v_<sub>n</sub> ... _v_<sub>1</sub> _n_ | `PICK` | _v_<sub>n</sub> ... _v_<sub>1</sub> _v_<sub>n</sub> | Duplicate element _n_
_v_<sub>n</sub> ... _v_<sub>1</sub> _n_ | `ROLL` | _v_<sub>n-1</sub> ... _v_<sub>1</sub> _v_<sub>n</sub> | Rotate stack elements (negative for reverse)
&mdash;              | `DEPTH`         | _n_                     | Number of items on the data stack
_n_ _m_              | `ADD`           | _n+m_                   | Numeric addition
_n_ _m_              | `MUL`           | _n*m_                   | Numeric multiplication
_n_ _m_              | `COMPARE`       | _n-m_                   | Compare numeric values
&mdash;              | `TRUE`          | TRUE                    | All bits set (1)
&mdash;              | `FALSE`         | FALSE                   | All bits clear (0)
_n_                  | `LT?`           | _bool_                  | `TRUE` if _n_ < 0; otherwise `FALSE`
_n_                  | `EQ?`           | _bool_                  | `TRUE` if _n_ = 0; otherwise `FALSE`
_n_                  | `GT?`           | _bool_                  | `TRUE` if _n_ > 0; otherwise `FALSE`
_n_                  | `NOT`           | ~_n_                    | Bitwise negation
_n_ _m_              | `AND`           | _n_&_m_                 | Bitwise and
_n_ _m_              | `OR`            | _n_\|_m_                | Bitwise or
_n_ _m_              | `XOR`           | _n_^_m_                 | Bitwise xor
_address_            | `?`             | _value_                 | Load _value_ from _address_
_value_ _address_    | `!`             | &mdash;                 | Store _value_ into _address_
_address_            | `??`            | _value_                 | Atomic load _value_ from volatile _address_
_value_ _address_    | `!!`            | &mdash;                 | Atomic store _value_ into volatile _address_

### Example

```
[ ] = sink_beh 
sink_beh CREATE = sink

[ # cust
  UART0_RXDATA ??  # read UART receive fifo status/data
  DUP LT? IF [
    DROP
    SELF SEND  # retry
  ] ELSE [
    16#FF AND SWAP SEND
  ]
] DUP = serial_read_beh CREATE = serial_read

[ # cust octet
  UART0_TXDATA ??  # read UART transmit fifo status
  LT? IF [
    SELF SEND  # retry
  ] ELSE [
    UART0_TXDATA !!  # write UART fifo data
    SELF SWAP SEND
  ]
] DUP = serial_write_beh CREATE = serial_write

[ # octet
  DUP = octet '\r' COMPARE
  EQ? IF [
    ' Stop dispatcher SEND
  ]
  SELF octet serial_write SEND
  @ serial_busy_beh BECOME
] = serial_echo_beh
[ # $serial_write
  serial_write COMPARE
  EQ? IF [
    SELF serial_read SEND
    @ serial_echo_beh BECOME
  ]
] = serial_busy_beh
serial_echo_beh CREATE serial_echo

[ # output
  = output
  [ # cust octet
    SWAP = cust
    SELF SWAP output SEND
    SELF cust SEND
    output empty-Q serial_empty_beh BECOME
  ]
] = serial_empty_beh
[ # output queue
  = queue
  = output
  [ # $output | cust octet
    DUP output COMPARE
    EQ? IF [ # $output
      DROP
      queue Q-empty IF [
        output serial_empty_beh BECOME
      ] ELSE [
        queue Q-TAKE
        output SWAP serial_buffer_beh BECOME
        SELF SWAP SEND
      ]
    ] ELSE [ # cust octet
      queue Q-PUT
      output SWAP serial_buffer_beh BECOME
      SELF SWAP SEND
    ]
  ]
] = serial_buffer_beh
serial_write serial_empty_beh CREATE = serial_buffer

serial_echo serial_read SEND  # start echo listen-loop
```

## Actor Assignment-Machine

An actor's behavior can also be decribed using a sequential model of variable assignments. This is essentially the model of contemporary processor cores. However, in order to avoid the pitfalls of shared mutable state, we only allow assignment to actor-local (private) variables. All visible effects are captured in the asynchronous messages between actors.

A _sponsor_ plays the role of a processor core, mediating access to computational resources and executing the instructions in an actor's behavior script (the _program_). Each message delivery event is handled as if it was an atomic transaction. No effects are visible outside the actor until message handling is completed. Message handling may be aborted by an exception, in which case the message is effectively ignored.

A _dictionary_ mapping names to values is the primary conceptual data structure. Each actor maintains a persistent dictionary of local variables, representing it's private state. Each message is a read-only _dictionary_ from which values may be retrieved by the actor's behavior script. Information derived from the message may be assigned to the actor's local persistent state.

### BART (Blockly Actor Run-Time)

[BART](https://github.com/dalnefre/blockly/tree/explicit-message-dictionary) is an implementation of an Actor Assignment Machine using the [Blockly](https://developers.google.com/blockly/) web-based visual programming editor. BART programs can be represented in JSON/CRLF as follows:

```javascript
{
    "lang": "BART",
    "ast": [
        <sponsor>,
        ...
    ]
}
```

The `ast` represents a list of _sponsors_, which encapsulate actor configurations. The following is a compact summary of BART program elements:

```javascript
// Containers
    { "kind":"actor_sponsor", "actors":<number>, "events":<number>, "script":[<action>, ...] }
// Actions
    { "kind":"actor_send", "message":<dictionary>, "actor":<address> }
    { "kind":"actor_send_after", "delay":<number>, "message":<dictionary>, "actor":<address> }
    { "kind":"actor_become", "behavior":<behavior> }
    { "kind":"actor_ignore" }
    { "kind":"actor_assign", "name":<string>, "value":<expression> }
    { "kind":"actor_fail", "error":<expression> }
// Address Expressions
    { "kind":"actor_create", "state":<dictionary>, "behavior":<behavior> }
    { "kind":"actor_self" }
// Behavior Expressions
    { "kind":"actor_behavior", "name":<string>, "script":[<action>, ...] }
// Value Expressions
    { "kind":"actor_state", "name":<string> }
    { "kind":"dict_get", "name":<string>, "in":<dictionary> }
    { "kind":"expr_literal", "const":<value> }
// Boolean Expressions
    { "kind":"actor_has_state", "name":<string> }
    { "kind":"dict_has", "name":<string>, "in":<dictionary> }
// Dictionary Expressions
    { "kind":"actor_message" }
    { "kind":"dict_empty" }
    { "kind":"dict_bind", "name":<string>, "value":<expression>, "with":<dictionary> }
// Number Expressions
    { "kind":"device_now" }
```

Now we describe each BART program element in detail.

#### Sponsor

An actor configuration executing a _script_, with limits on _actors_ and _events_.

```javascript
{
    "kind": "actor_sponsor",
    "actors": <number>,
    "events": <number>,
    "script": [
        <action>,
        ...
    ]
}
```

The actions in the sponsor's script initialize the actor configuration, creating initial actors and sending initial messages.

#### Send

This _action_ constructs a new send-event to deliver a specific _message_ to a target _actor_.

```javascript
{
    "kind": "actor_send",
    "message": <dictionary>,
    "actor": <address>
}
```

#### Create

This _expression_ constructs a new actor with an initial _state_ and _behavior_, and returns it's _address_.

```javascript
{
    "kind": "actor_create",
    "state": <dictionary>,
    "behavior": <behavior>
}
```

The actor's _behavior_ script may **retrieve** and **update** bindings in the private _state_ of this actor.

#### Behavior

This _expression_ constructs an actor behavior with an optional _name_.

```javascript
{
    "kind": "actor_behavior",
    "name": <string>,
    "script": [
        <action>,
        ...
    ]
}
```

Actor behavior scripts can be shared between actors, and each actor may use different scripts over time.

#### Become

This _action_ specifies the behavior this actor will use to process **subsequent** messages.

```javascript
{
    "kind": "actor_become",
    "behavior": <behavior> 
}
```

**NOTE:** Changing an actor's behavior script has **no effect** on how it handles the current message.

#### Assign

This _action_ updates the private state of an actor (or sponsor), binding a new _value_ to _name_.

```javascript
{
    "kind": "actor_assign",
    "name": <string>,
    "value": <expression>
}
```

#### State

This _expression_ retrieves the current _value_ bound to _name_ in the current actor (or sponsor).

```javascript
{
    "kind": "actor_state",
    "name": <string>
}
```

#### Has State

This _expression_ evaluates to `true` if _name_ is bound to a _value_ in the current actor (or sponsor), or `false` otherwise.

```javascript
{
    "kind": "actor_has_state",
    "name": <string>
}
```

#### Message

This _expression_ retrieves the (immutable) _dictionary_ representing the current message being processed.

```javascript
{
    "kind": "actor_message"
}
```

#### Self

This _expression_ evaluates to the _address_ of the current actor.

```javascript
{
    "kind": "actor_self"
}
```

#### Ignore

This _action_ has no effect.

```javascript
{
    "kind": "actor_ignore"
}
```

#### Fail

This _action_ aborts processing the current message and reverts all accumulated effects.

```javascript
{
    "kind": "actor_fail",
    "error": <expression>
}
```

#### Send After

This _action_ constructs a new send-event to deliver a specific _message_ to a target _actor_ after a _delay_.

```javascript
{
    "kind": "actor_send_after",
    "delay": <number>,
    "message": <dictionary>,
    "actor": <address>
}
```

#### Now

This _expression_ evaluates to the current time.

```javascript
{
    "kind": "device_now"
}
```

#### Empty

This _expression_ evaluates to an (immutable) empty dictionary.

```javascript
{
    "kind": "dict_empty"
}
```

#### Bind

This _expression_ evaluates to an immutable _dictionary_ where, _value_ is bound to _name_, extending or overriding bindings in _with_.

```javascript
{
    "kind": "dict_bind",
    "name": <string>,
    "value": <expression>,
    "with": <dictionary>
}
```

#### Get

This _expression_ retrieves the current _value_ bound to _name_ in this _dictionary_.

```javascript
{
    "kind": "dict_get",
    "name": <string>,
    "in": <dictionary>
}
```

#### Has

This _expression_ evaluates to `true` if _name_ is bound to a _value_ in this _dictionary_, or `false` otherwise.

```javascript
{
    "kind": "dict_has",
    "name": <string>,
    "in": <dictionary>
}
```

#### Literal

This _expression_ evaluates to a constant _value_.

```javascript
{
    "kind": "expr_literal",
    "const": <value>
}
```

----

### AAM Grammar -- DEPRECATED!
```
behavior-definition     <-  message-handler ('|' message-handler)*
message-handler         <-  message-pattern '->' handler-script
message-pattern         <-  '{' variable-pattern (',' variable-pattern)* '}'
variable-pattern        <-  variable ':' type
type                    <-  type-literal
                        /   ...
handler-script          <-  '[' script-action* ']'
script-action           <-  assigment
                        /   ...
assignment              <-  variable ':=' expression
variable                <-  name
expression              <-  object-expression
                        /   array-expression
                        /   string-expression
                        /   number-expression
                        /   boolean-expression
                        /   unit-expression
                        /   variable
                        /   expression '.' expression           # FIXME: remove left recursion
                        /   expression '[' expression ']'       # FIXME: remove left recursion
                        /   expression '(' expression ')'       # FIXME: remove left recursion
                        /   ...
                        /   '(' expression ')'
object-expression       <-  object-constructor
array-expression        <-  array-constructor
string-expression       <-  string-literal
number-expression       <-  number-literal
boolean-expression      <-  boolean-literal
unit-expression         <-  unit-literal
object-constructor      <-  '{' property-constructor (',' property-constructor)* '}'
property-constructor    <-  (name / string-literal) ':' expression
array-constructor       <-  '[' expression (',' expression)* ']'
type-literal            <-  'Unit' / 'Boolean' / 'Number' / 'String' / 'Array' / 'Object' / 'Any'
string-literal          <-  open-quote character-literal* close-quote
number-literal          <-  integer ('.' fraction)? (('e' / 'E') exponent)?
boolean-literal         <-  'true' / 'false'
unit-literal            <-  'null'
name                    <-  [-_A-Za-z0-9]+
integer                 <-  ('+' / '-')? natural
natural                 <-  base10-natural / base2-natural / base8-natural / base16-natural / ...
base2-natural           <-  '2#' ('0' / '1')+
base8-natural           <-  '8#' [0-7]+
base10-natural          <-  '10#'? [0-9]+
base16-natural          <-  '16#' [0-9A-Fa-f]+
```
