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

  * Create a new actor
  * Send a message
  * Become a new behavior

A new value for _effects_ is created as each primitive action is applied. The final value is a composite of all the accumulated effects.

### Create

The _create_ primitive constructs a new actor with an initial _behavior_.

```javascript
{
    "kind": "create",
    "behavior": <behavior>
}
````

The _effects_ object is augmented with the (opaque) _address_ of the newly-created actor.

```
effects.concatenate[{
    create: effects.create.concatenate[[
        <address>
    ]]
}]
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
effects.concatenate[{
    send: effects.send.concatenate[[
        { target: <address>, message: <any> }
    ]]
}]
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
effects.concatenate[{ become: <behavior> }]
````
