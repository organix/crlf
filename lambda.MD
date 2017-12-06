# Untyped Lambda-calculus

The untyped [Lambda-calculus](http://en.wikipedia.org/wiki/Lambda_calculus) is a universal notation for defining partial-recursive functions.

```javascript
{
    "lang": "lambda",
    "ast": <abstract syntax tree>
}
```

The `ast` represents an _expression_, which may be one of the following:

  * Variable
  * Abstraction
  * Application

## Variable

```javascript
{
    "kind": "variable",
    "name": <string>
}
```

## Abstraction

```javascript
{
    "kind": "abstraction",
    ...
}
```

## Application

```javascript
{
    "kind": "application",
    ...
}
```
