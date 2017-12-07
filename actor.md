# Actor Behavior

An actor behavior describes the effects of an actor receiving a message. The effects include new actors created, new messages sent, and the behavior to be used for handling subsequent messages. The initial value of the abstract _effects_ object is:

```javascript
{
    "create": [],
    "send": [],
    "become": <current behavior>
}
```

## Syntax

```javascript
{
    "lang": "actor",
    "ast": <behavior>
}
```

The `ast` represents an _behavior_, which contains a list of _actions_:

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
effects := effects ^ { create: effects.create ^ [ <address> ] }
````

### Send

The _send_ primitive constructs a new send-event to deliver a specific _message_ to a target _address_.

```javascript
{
    "kind": "send",
    "target": <address>,
    "message": <any>
}
````

The _effects_ object is augmented with the new send-event.

```
effects := effects ^ { send: effects.send ^ [ { target: <address>, message: <any> } ] }
````

### Become

The _become_ primitive specified a replacement behavior for handling subsequent messages to the current actor.

```javascript
{
    "kind": "become",
    "behavior": <behavior>
}
````

The _effects_ object is updated with the new _behavior_.

```
effects := effects ^ { become: <behavior> }
````
