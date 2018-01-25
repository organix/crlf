# Abstract Value Objects

Abstract Data Values are organized into a type-hierarchy:

* Value
    * Data
        * Null
        * Boolean
        * Number
        * Composite
            * String
            * Array
            * Object
    * Combiner

## Data

Various frequently-used data values are available by name.

### `VO.Null` type

* `VO.null`

### `VO.Boolean` type

* `VO.true`
* `VO.false`

### `VO.Number` type

* `VO.minusOne`
* `VO.zero`
* `VO.one`
* `VO.two`

### `VO.String` type

* `VO.emptyString`

### `VO.Array` type

* `VO.emptyArray`

### `VO.Object` type

* `VO.emptyObject`

## Combiners

Abstract values represent immutable state/data. In order for some computation to be done, there must be a way to specify a transformation between values. We call that transformation a _combiner_. The _combination_ of a combiner with an _input_ value determines an _output_ value (result).

```
           +----------+
input ---> | combiner | ---> output
           +----------+
```

If a value cannot be determined, an exception is thrown. So the application of a combiner to a value either produces a result, or signals an error.

Combiners may be chained together such that the output from one combiner is the input to another. This assumes that the output value from the first combiner is acceptable as input for the second combiner, otherwise an exception is thrown.

```
           +------------+      +------------+
input ---> | combiner A | ---> | combiner B | ---> output
           +------------+      +------------+
```

Chained combiners may be viewed as a single combiner which is the _concatenation_ of combiners (like _composition_ of _functions_).

```
           +------------------+
input ---> | combiner (A ^ B) | ---> output
           +------------------+
```

The output value of a combination may also be a combiner. The resulting combiner may be applied to additional input values.

```
             +------------+
input x ---> | combiner A | -------+
             +------------+        |
                                   v
                                 +------------+
                    input y ---> | combiner B | ---> output
                                 +------------+
```

### Abstract Combiners

Each abstract data value can be viewed as a _combiner_ which takes a _name_ (of type _String_) as input. The result value, naturally, may itself be a _combiner_.

For example, the `null` value, when given the input `"equals"`, returns a _combiner_. This combiner returns `true` if its input is `null`, or `false` otherwise. 

```
              +------+
"equals" ---> | null | -------+
              +------+        |
                              v
                            +-------------+
                  null ---> | null.equals | ---> true
                            +-------------+

              +------+
"equals" ---> | null | -------+
              +------+        |
                              v
                            +-------------+
                  true ---> | null.equals | ---> false
                            +-------------+
```

Note that the combiner we've labelled `null.equals` is **not** one of the previously specified abstract data values. Combiners representing intermediate results (or partial function applications) may have no corresponding abstract data value.

## Abstract Value Methods

Each abstract data value can be viewed as a value-object, with properties and methods. However, properties can only be value-objects, and methods may only return a value-object (and not mutate any objects). Methods with no parameters are indistinguishable from properties. One such method has already been described for each [abstract data type](JSON.md), the _equals_ predicate.

### Global (ambient) methods

#### `VO.throw <value>`

Signal an exceptional condition with an error `value` (see `<crlf>.try`).

#### `VO.ensure <boolean>`

Throw an exception if the parameter value is not `true`.

### Value methods

Some methods are common to all value types.

#### `<value>.equals <value>`

`true` if the target is equal to the parameter value, otherwise `false`.

#### `<value>.hasType <type>`

`true` if the target is a member of the specified type, otherwise `false`.

#### `<value>.combine <string>`

The `<combiner>` corresponding to the named abstract method. Throws an exception if the named method does not exist.

### Data methods

Data values are restricted to types with a defined JSON serialization.

#### `<data>.asJSON`

The JSON-encoded `<string>` representation of the target.

### Boolean methods

#### `<boolean>.not`

If target is `true`, return `false`, otherwise `true`.

#### `<boolean>.and <boolean>`

If target is `false`, return `false`, otherwise return the parameter value.

#### `<boolean>.or <boolean>`

If target is `true`, return `true`, otherwise return the parameter value.

#### `<boolean>.if { then:<crlf>, else:<crlf> }`

If target is `true`, return `then.value`, otherwise `else.value` (see `<crlf>.value`).

### Number methods

#### `<number>.lessThan <number>`

`true` if target is numerically less than the parameter value, otherwise `false`.

#### `<number>.lessEqual <number>`

`true` if target is numerically less than or equal to the parameter value, otherwise `false`.

#### `<number>.greaterEqual <number>`

`true` if target is numerically greater than or equal to the parameter value, otherwise `false`.

#### `<number>.greaterThan <number>`

`true` if target is numerically greater than the parameter value, otherwise `false`.

#### `<number>.plus <number>`

The `<number>` that is the mathematical sum of the target and the parameter value.

#### `<number>.times <number>`

The `<number>` that is the mathematical product of the target and the parameter value.

### Composite methods

#### `<composite>.value <data>`

The component `<value>` corresponding to the selector `<data>` value in the target.

### String methods

#### `<string>.length`

The `<number>` of characters in the target.

#### `<string>.value <number>`

The character `<number>` at the parameter value 0-based index in the target.

#### `<string>.parseJSON`

`{ "ok": true, "value": <value> }` on success, otherwise `{ "ok": false, "error": <value> }`.

#### `<string>.append <number>`

The `<string>` consisting of the characters of target followed by the new character parameter.

#### `<string>.concatenate <string>`

The `<string>` consisting of the characters of target followed by the characters of the parameter value.

#### `<string>.skip <number>`

The `<string>` consisting of the characters of target after skipping over the parameter `<number>` of characters from the begining.

#### `<string>.take <number>`

The `<string>` consisting of the parameter `<number>` of characters taken from the begining of the target.

#### `<string>.extract { "from": <number>, "upto": <number> }`

The `<string>` consisting of the characters of target from the 0-based half-open interval specified by the parameter.

#### `<string>.asArray`

The `<array>` consisting of the characters of target as `<number>`s.

#### `<string>.bind <value>`

The `<object>` where the target name `<string>` is associated with the parameter `<value>`.

### Array methods

#### `<array>.length`

The `<number>` of elements in the target.

#### `<array>.value <number>`

The element `<value>` at the parameter value 0-based index in the target.

#### `<array>.append [<value>]`

The `<array>` consisting of the elemtns of target followed by the new element parameter.

#### `<array>.concatenate [<array>]`

The `<array>` consisting of the elements of target followed by the elements of the parameter value.

#### `<array>.extract [<number>, <number>]`

The `<array>` consisting of the elements of target from the first parameter value to the second parameter value as a 0-based half-open interval.

#### `<array>.asString`

The `<string>` consisting of the elements of target (which must be `<number>`s).

### Object methods

#### `<object>.hasProperty [<string>]`

`true` if the target has a property whose name matches the parameter value, otherwise `false`.

#### `<object>.value <string>`

The `<value>` associated with the parameter value name in the target.

#### `<object>.concatenate [<object>]`

The `<object>` consisting of the properties of target, either augmented or replaced by properties of the parameter value.

#### `<object>.append [<name>, <value>]`

The `<object>` consisting of the properties of target followed by the new property name/value.

#### `<object>.extract [<string>, ...]`

The `<object>` consisting of the properties of target named by the parameter value(s).

#### `<object>.names`

The `<array>` of property names defined by the target.

### Combiner methods

#### `<combiner>.combine <value>`

The output `<value>` resulting from applying the combiner to the input `<value>`.

#### `<combiner>.concatenate <combiner>`

The `<combiner>` representing the composition of the target and the parameter. The output of the target `<combiner>` is the input to the parameter `<combiner>`.

### CRLF methods

#### `<crlf>.compile`

Transform the target (according to `lang`) to produce a `<value>`.

#### `<crlf>.try { catch:<crlf>, default:<value> }`

Evaluate the target (according to `lang`) to produce a value. If an exception is thrown, evaluate `catch` or return `default` value.

## Value Object Expressions (VO)

`VO` is a [crlf-encoding](READMD.md) for _expressions_ involving [JSON](JSON.md) value-objects.

```javascript
{
    "lang": "VO",
    "ast": <expression>
}
```

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
