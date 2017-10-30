# Consistent Representation Language Framework (crlf)

Use `JSON` encoding to communicate language-specific abstract syntax trees. Possible uses include:

* Packaging code for remote execution
* Intermediate format for language tool-chains
* Embedding language fragments within another language

```javascript
{
    "lang": <language identifier>
    "ast": <abstract syntax tree>
}
```

The value of the `lang` property specifies the interpretation of the value of the `ast` property.
