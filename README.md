# Consistent Representation Language Framework (crlf)

Use `JSON` encoding to communicate language-specific abstract syntax trees. Possible uses include:

* Packaging code for remote execution
* Intermediate format for language tool-chains
* Embedding language fragments within another language

```javascript
{
    "lang": <language identifier>,
    "ast": <abstract syntax tree>
}
```

The value of the `lang` property specifies the interpretation of the value of the `ast` property.

`lang` value | Description
-------------|------------
[JSON](JSON.md) | JavaScript Object Notation
[VO](VO.md) | Abstract Value-Object Expressions
[lambda](lambda.md) | Untyped Lambda-calculus
[PEG](PEG.md) | Parsing Expression Grammars
[actor](actor.md) | Primitive actions for the Actor-model of computation
[Humus](Humus.md) | Humus Actor-Programming Language

## Abstract Compilation

The `ast` property value represents the _source_ language for a particular type of _crlf_ object. This source consists entirely of abstract _values_ which can be represented by [JSON](JSON.md):

  * `null`
  * Boolean (`true` and `false`)
  * Number
  * String
  * Array
  * Object

The `lang` property value specifies a _compiler_ for this type of _crlf_ object. The compiler translates the _source_ value into an implementation-specific object. The abstract semantics of this object are defined by each language.
