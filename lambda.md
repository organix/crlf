# Untyped Lambda-calculus

The untyped [Lambda-calculus](http://en.wikipedia.org/wiki/Lambda_calculus) is a universal notation for defining partial-recursive functions.

## Syntax

```javascript
{
    "lang": "lambda",
    "ast": <expression>
}
```

The `ast` represents an _expression_, which may be one of the following:

  * Variable
  * Abstraction
  * Application

### Variable

```javascript
{
    "kind": "variable",
    "name": <string>
}
```

### Abstraction

```javascript
{
    "kind": "abstraction",
    "parameter": <string>,
    "body": <expression>
}
```

### Application

```javascript
{
    "kind": "application",
    "operator": <expression>,
    "operand": <expression>
}
```
