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
[lambda](lambda.md) | Untyped Lambda-calculus
[PEG](PEG.md) | Parsing Expression Grammars
[actor](actor.md) | Primitive actions for the Actor-model of computation

## Semantic Notation

The abstract semantics of these languages are decribed using a few common notational conventions. We start with the data-types which can be represented by [JSON](JSON.md), which are:

  * String
  * Number
  * Object
  * Array
  * Boolean (`true` and `false`)
  * `null`

Named meta-variables may be given any value, using assigment notation:

```
<name> := <value>
```

Object fields may be accessed by name:

```
<object>.<name>
```

Objects fields may also be accessed by string key:

```
<object>[<string>]
```

Array elements may be accessed by number index (0-based):

```
<array>[<number>]
```

Array length may be accessed as a named field:

```
<array>.length
```

New array values may be created by concatenation:

```
<prefix> ^ <suffix>
```

New object values may be created by concatenation:

```
<original> ^ <updates>
```
