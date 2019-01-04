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
[ ...                | `(` ... `)`     | [ ...                   | Immediate (unquoted) value
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

A _dictionary_ mapping names to values is the primary conceptual data structure. Each actor maintains a persistent dictionary of local variables, representing it's private state. The behavior of an actor is organized into message-handlers based on message patterns. Each pattern is associated with a script for handling matching messages. Each message is a _dictionary_ whose mappings are appended to the actor's private state, forming the environment in which script variables are resolved.

```
behavior-definition     <-  message-handler ('|' message-handler)*
message-handler         <-  message-pattern '->' handler-script
message-pattern         <-  '{' variable-pattern (',' variable-pattern)* '}'
variable-pattern        <-  name ':' type
handler-script          <-  '[' script-action* ']'
script-action           <-  assigment / ...
assignment              <-  name ':=' expression
```
