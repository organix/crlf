# BART (Blockly Actor Run-Time)

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

The `ast` represents a list of _sponsors_, which encapsulate actor configurations.

## Actor Assignment-Machine

An actor's behavior can be described using a sequential model of variable assignments. This is essentially the model of contemporary processor cores. However, in order to avoid the pitfalls of shared mutable state, we only allow assignment to actor-local (private) variables. All visible effects are captured in the asynchronous messages between actors.

A _sponsor_ plays the role of a processor core, mediating access to computational resources and executing the instructions in an actor's behavior script (the _program_). Each message delivery event is handled as if it was an atomic transaction. No effects are visible outside the actor until message handling is completed. Message handling may be aborted by an exception, in which case the message is effectively ignored.

A _dictionary_ mapping names to values is the primary conceptual data structure. Each actor maintains a persistent dictionary of local variables, representing it's private state. Each message is a read-only _dictionary_ from which values may be retrieved by the actor's behavior script. Information derived from the message may be assigned to the actor's local persistent state.

### BART Progam Elements

 The following is a compact summary of BART program elements:

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
    { "kind":"expr_operation", "name":<string>, "args":[<expression>, ...] }
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

This _expression_ evaluates to a constant _value_ (with an optional _type_).

```javascript
{
    "kind": "expr_literal",
    "type": <type>,
    "const": <value>
}
```

#### Operation

This _expression_ evaluates to the result (with an optional _type_) of calling operation _name_ with _args_.

```javascript
{
    "kind": "expr_operation",
    "type": <type>,
    "name": <string>,
    "args": [
        <expression>,
        ...
    ]
}
```
