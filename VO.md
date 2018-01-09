# JSON Value Objects

Abstract Data Values are organized into a type-hierarchy:

 * Value
   * Null
   * Boolean
   * Number
   * String
   * Array
   * Object

## Abstract Value Methods

Each abstract data value can be viewed as a value-object, with properties and methods. However, properties can only be JSON values, and methods may only return a JSON value (and not mutate any objects). Methods with no parameters are indistinguishable from properties. One such method has already been described for each abstract data type, the _equals_ predicate.

### Global (ambient) methods

#### `VO.throw [<value>]`

Signal an exceptional condition with an error `value` (see `<crlf>.try`).

#### `VO.ensure [<boolean>]`

If the parameter value is `true`, return the target, otherwise throw an exception.

### Value methods

Some methods are common to all value types.

#### `<value>.equals [<value>]`

`true` if the target is equal to the parameter value, otherwise `false`.

#### `<value>.hasType [<type>]`

`true` if the target is a member of the specified type, otherwise `false`.

#### `<value>.toJSON`

The JSON-encoded `<string>` representation of the target.

### Boolean methods

#### `<boolean>.not`

If target is `true`, return `false`, otherwise `true`.

#### `<boolean>.and [<boolean>]`

If target is `false`, return `false`, otherwise return the parameter value.

#### `<boolean>.or [<boolean>]`

If target is `true`, return `true`, otherwise return the parameter value.

#### `<boolean>.if { then:<crlf>, else:<crlf> }`

If target is `true`, return `then.value`, otherwise `else.value` (see `<crlf>.value`).

### Number methods

#### `<number>.lessThan [<number>]`

`true` if target is numerically less than the parameter value, otherwise `false`.

#### `<number>.plus [<number>]`

The `<number>` that is the mathematical sum of the target and the parameter value.

### String methods

#### `<string>.length`

The `<number>` of characters in the target.

#### `<string>.value [<number>]`

The character `<number>` at the parameter value 0-based index in the target.

#### `<string>.parseJSON`

`{ "ok": true, "value": <value> }` on success, otherwise `{ "ok": false, "error": <value> }`.

#### `<string>.append [<number>]`

The `<string>` consisting of the characters of target followed by the new character parameter.

#### `<string>.concatenate [<string>]`

The `<string>` consisting of the characters of target followed by the characters of the parameter value.

#### `<string>.extract [<number>, <number>]`

The `<string>` consisting of the characters of target from the first parameter value to the second parameter value as a 0-based half-open interval.

### Array methods

#### `<array>.length`

The `<number>` of elements in the target.

#### `<array>.value [<number>]`

The element `<value>` at the parameter value 0-based index in the target.

#### `<array>.append [<value>]`

The `<array>` consisting of the elemtns of target followed by the new element parameter.

#### `<array>.concatenate [<array>]`

The `<array>` consisting of the elements of target followed by the elements of the parameter value.

#### `<array>.extract [<number>, <number>]`

The `<array>` consisting of the elements of target from the first parameter value to the second parameter value as a 0-based half-open interval.

### Object methods

#### `<object>.hasProperty [<string>]`

`true` if the target has a property whose name matches the parameter value, otherwise `false`.

#### `<object>.value [<string>]`

The `<value>` associated with the parameter value name in the target.

#### `<object>.concatenate [<object>]`

The `<object>` consisting of the properties of target, either augmented or replaced by properties of the parameter value.

#### `<object>.append [<name>, <value>]`

The `<object>` consisting of the properties of target followed by the new property name/value.

#### `<object>.extract [<string>, ...]`

The `<object>` consisting of the properties of target named by the parameter value(s).

#### `<object>.names`

The `<array>` of property names defined by the target.

### CRLF methods

#### `<crlf>.value`

Evaluate the target (according to `lang`) to produce a value.

#### `<crlf>.try { catch:<crlf>, default:<value> }`

Evaluate the target (according to `lang`) to produce a value. If an exception is thrown, evaluate `catch` or return `default` value.

## Value Object Expressions (VO)

_VO_ is a crlf-encoding for _expressions_ involving JSON value-objects.

The Expression type derives from the Value type:

  * Value
    * Expression
      * ValueExpr
      * VariableExpr
      * CombineExpr
      * MethodExpr

### Value expression (ValueExpr)

Value expressions are wrappers for abstract data values. These are constant expressions.

```javascript
{
    "lang": "VO",
    "ast": {
        "kind": "value",
        "value": <value>
    }
}
```

### Variable expression (VariableExpr)

A variable expression is a named reference to a value-object. During evaluation, the run-time _context_ provides a mapping from names to values.

```javascript
{
    "lang": "VO",
    "ast": {
        "kind": "variable",
        "name": <string>
    }
}
```

### Combination expression (CombineExpr)

A combination expression represents an operative combination. The _operative_ expression is evaluated to determine the _combiner_ to which the _parameter_ value is passed. The value returned by the combiner is the value of the expression.

```javascript
{
    "lang": "VO",
    "ast": {
        "kind": "combination",
        "operative": <expression>,
        "parameter": <value>
    }
}
```

Note: in the common case where the combiner is _applicative_, the parameter value is an _array_ of _expressions_ which are evaluated to produce an _array_ of _arguments_ for the applicative.

### Method expression (MethodExpr)

A method expression selects a named method from a target object. The _target_ expression is evaluated to determine the target _object_. The _selector_ expression is evaluated to determine the method _name_, which must be a _string_. The value returned is the _combiner_ selected.

```javascript
{
    "lang": "VO",
    "ast": {
        "kind": "method",
        "target": <expression>,
        "selector": <expression>
    }
}
```
