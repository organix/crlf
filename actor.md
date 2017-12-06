# Actor Behavior

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

```javascript
{
    "kind": "create",
    "behavior": <behavior>
}
````

### Send

```javascript
{
    "kind": "send",
    "target": <address>,
    "message": <any>
}
````

### Become

```javascript
{
    "kind": "become",
    "behavior": <behavior>
}
````
