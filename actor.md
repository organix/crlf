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
````

The _effects_ object is augmented with the (opaque) _address_ of the newly-created actor and its initial _behavior_.

```
effects := effects.concatenate {
    create: effects.create.append {
        actor: <address>,
        behavior: <behavior>
    }
}
````

### Send

The _send_ primitive constructs a new send-event to deliver a specific _message_ to a _target_ actor.

```javascript
{
    "kind": "send",
    "target": <address>,
    "message": <any>
}
````

The _effects_ object is augmented with the new send-event.

```
effects := effects.concatenate {
    send: effects.send.append {
        target: <address>,
        message: <any>
    }
}
````

### Become

The _become_ primitive specifies a replacement _behavior_ for handling subsequent messages to the current actor.

```javascript
{
    "kind": "become",
    "behavior": <behavior>
}
````

The _effects_ object is updated with the new _behavior_.

```
effects := effects.concatenate {
    become: <behavior>
}
````

## Actor Stack-Machine

There are many possible models for describing an actor behavior. One simple model is a [imperative](https://en.wikipedia.org/wiki/Imperative_programming) stack-oriented machine (similar to [FORTH](https://en.wikipedia.org/wiki/Forth_(programming_language))).

Program source is provided as a stream of _words_ (whitespace separated in text format). Each word is looked up in the current _dictionary_ and the corresponding _block_ is executed. Literal values are pushed on the data _stack_, which is used to provide parameters and return values for executing blocks. Some blocks also consume words from the source stream.

Input                | Operation       | Output                  | Description
---------------------|-----------------|-------------------------|------------
_block_              | CREATE          | _actor_                 | Create a new actor with _block_ behavior
..._message_ _actor_ | SEND            | &mdash;                 | Send _message_ to _actor_
_block_              | BECOME          | &mdash;                 | Replace current actor's behavior with _block_
&mdash;              | SELF            | _actor_                 | Push the current actor's address on the data stack
_value_              | = _word_        | &mdash;                 | Bind _value_ to _word_ in the current dictionary
&mdash;              | ' _word_        | _word_                  | Push (literal) _word_ on the data stack
&mdash;              | @ _word_        | _value_                 | Lookup _value_ bound to _word_ in the current dictionary
&mdash;              | [ ..._word_ ]   | _block_                 | Create block (quoted) value
[ ...                | ( ..._word_ )   | [ ...                   | Immediate (unquoted) value
_bool_               | IF [] ELSE []   | &mdash;                 | Conditional execution of blocks
_v_                  | DROP            | &mdash;                 | Drop the top element
_v_                  | DUP             | _v_ _v_                 | Duplicate the top element
_v_2_ _v_1_          | SWAP            | _v_1_ _v_2_             | Swap the top two elements
_v_n_ ... _v_1_ _n_  | ROT             | _v_n-1_ ... _v_1_ _v_n_ | Rotate stack elements (negative = reverse)
_v_n_ ... _v_1_ _n_  | PICK            | _v_n_ ... _v_1_ _v_n_   | Duplicate element _n_
_n_ _m_              | ADD             | _n+m_                   | Numeric addition
_n_ _m_              | MUL             | _n*m_                   | Numeric multiplication
_n_ _m_              | COMPARE         | _n-m_                   | Compare numeric values
_n_                  | LT?             | _bool_                  | TRUE if _n_ < 0
_n_                  | EQ?             | _bool_                  | TRUE if _n_ = 0
_n_                  | GT?             | _bool_                  | TRUE if _n_ > 0
_address_            | ?               | _value_                 | Load _value_ from _address_
_value_ _address_    | !               | &mdash;                 | Store _value_ into _address_
