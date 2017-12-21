/*

VO.js - "crlf/VO" JSON Value Objects

The MIT License (MIT)

Copyright (c) 2017 Dale Schumacher

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

*/
"use strict";

var VO = module.exports;
VO.version = "0.0.0";

//VO.log = console.log;
VO.log = function () {};

VO.Value = (function (self) {
    self = self || {};
    self.equals = function equals(other) {
        if (this === other) {
            return VO.true;
        }
        return VO.false;
    };
    self.isNull = function isNull() {
        return VO.false;
    };
    self.isBoolean = function isBoolean() {
        return VO.false;
    };
    self.isNumber = function isNumber() {
        return VO.false;
    };
    self.isString = function isString() {
        return VO.false;
    };
    self.isArray = function isArray() {
        return VO.false;
    };
    self.isObject = function isObject() {
        return VO.false;
    };
    self.throw = function error(value) {
        throw new Error(value);
    };
    VO.throw = self.throw;  // allow top-level exceptions
    self.ensure = function ensure(predicate) {
        if (predicate !== VO.true) {
            VO.throw(predicate);
        }
    };
    VO.ensure = self.ensure;  // allow top-level assertions
    var constructor = function Value() {};
    self.constructor = constructor;
    constructor.prototype = self;
    return constructor;
})();

VO.Null = (function (self) {
    self = self || new VO.Value();
    self.isNull = function isNull() {
        return VO.true;
    };
    var constructor = function Null() {
        this._value = null;
    };
    self.constructor = constructor;
    constructor.prototype = self;
    VO.null = new constructor();
    return function Null() {
        return VO.null;
    };
})();

VO.Boolean = (function (self) {
    self = self || new VO.Value();
    self.isBoolean = function isBoolean() {
        return VO.true;
    };
    self.not = function not() {
        this.ensure(this.isBoolean());
        if (this === VO.false) {
            return VO.true;
        }
        return VO.false;
    };
    self.and = function and(boolean) {
        this.ensure(this.isBoolean());
        if (this === VO.false) {
            return VO.false;
        }
        this.ensure(boolean.isBoolean());
        return boolean;
    };
    self.or = function or(boolean) {
        this.ensure(this.isBoolean());
        if (this === VO.true) {
            return VO.true;
        }
        this.ensure(boolean.isBoolean());
        return boolean;
    };
    var constructor = function Boolean(value) {
        this._value = value;
    };
    self.constructor = constructor;
    constructor.prototype = self;
    VO.true = new constructor(true);
    VO.false = new constructor(false);
    return function Boolean(value) {
        return (value ? VO.true : VO.false);
    };
})();

VO.Number = (function (self) {
    self = self || new VO.Value();
    self.equals = function equals(other) {
        if (this === other) {
            return VO.true;
        }
        if ((this.isNumber() === VO.true) && (other.isNumber() === VO.true)) {
            if (this._value === other._value) {
                return VO.true;
            }
        }
        return VO.false;
    };
    self.isNumber = function isNumber() {
        return VO.true;
    };
    self.lessThan = function lessThan(number) {
        this.ensure(this.isNumber());
        this.ensure(number.isNumber());
        return VO.Boolean(this._value < number._value);
    };
    self.lessEqual = function lessEqual(number) {
        this.ensure(this.isNumber());
        this.ensure(number.isNumber());
        return VO.Boolean(this._value <= number._value);
    };
    self.greaterEqual = function greaterEqual(number) {
        this.ensure(this.isNumber());
        this.ensure(number.isNumber());
        return VO.Boolean(this._value >= number._value);
    };
    self.greaterThan = function greaterThan(number) {
        this.ensure(this.isNumber());
        this.ensure(number.isNumber());
        return VO.Boolean(this._value > number._value);
    };
    self.plus = function plus(number) {
        this.ensure(this.isNumber());
        this.ensure(number.isNumber());
        return new VO.Number(this._value + number._value);
    };
    self.times = function times(number) {
        this.ensure(this.isNumber());
        this.ensure(number.isNumber());
        return new VO.Number(this._value * number._value);
    };
    var constructor = function Number(value) {
        VO.ensure(VO.Boolean(typeof value === "number"));
        this._value = value;
    };
    self.constructor = constructor;
    constructor.prototype = self;
    VO.minusOne = new constructor(-1);
    VO.zero = new constructor(0);
    VO.one = new constructor(1);
    VO.two = new constructor(2);
    return constructor;
})();

VO.String = (function (self) {
    self = self || new VO.Value();
    self.equals = function equals(other) {
        if (this === other) {
            return VO.true;
        }
        if ((this.isString() === VO.true) && (other.isString() === VO.true)) {
            if (this._value === other._value) {
                return VO.true;
            }
        }
        return VO.false;
    };
    self.isString = function isString() {
        return VO.true;
    };
    self.length = function length() {
        this.ensure(this.isString());
        return new VO.Number(this._value.length);
    };
    self.value = function value(offset) {
        this.ensure(this.isString());
        this.ensure(offset.isNumber());
        this.ensure(VO.zero.lessEqual(offset));  // 0 <= offset
        this.ensure(offset.lessThan(this.length()));  // offset < length
        return new VO.Number(this._value.charCodeAt(offset._value));  // FIXME: use .codePointAt() when available
    };
    self.concatenate = function concatenate(string) {
        this.ensure(this.isString());
        this.ensure(string.isString());
        return new VO.String(this._value + string._value);
    };
    self.extract = function extract(from, upto) {
        this.ensure(this.isString());
        this.ensure(from.isNumber());
        this.ensure(upto.isNumber());
        this.ensure(VO.zero.lessEqual(from));  // 0 <= from
        this.ensure(from.lessEqual(upto));  // from <= upto
        this.ensure(upto.lessEqual(this.length()));  // upto <= length
        return new VO.String(this._value.slice(from._value, upto._value));
    };
    var constructor = function String(value) {
        VO.ensure(VO.Boolean(typeof value === "string"));
        this._value = value;
    };
    self.constructor = constructor;
    constructor.prototype = self;
    VO.emptyString = new constructor("");
    return constructor;
})();

VO.Array = (function (self) {
    self = self || new VO.Value();
    self.equals = function equals(other) {
        if (this === other) {
            return VO.true;
        }
        if ((this.isArray() === VO.true) && (other.isArray() === VO.true)) {
            var a = this._value;
            var b = other._value;
            if (a.length === b.length) {
                for (var i = 0; i < a.length; ++i) {
                    if (a[i].equals(b[i]) === VO.false) {
                        return VO.false;
                    }
                }
                return VO.true;
            }
        }
        return VO.false;
    };
    self.isArray = function isArray() {
        return VO.true;
    };
    self.length = function length() {
        this.ensure(this.isArray());
        return new VO.Number(this._value.length);
    };
    self.value = function value(offset) {
        this.ensure(this.isArray());
        this.ensure(offset.isNumber());
        this.ensure(VO.zero.lessEqual(offset));  // 0 <= offset
        this.ensure(offset.lessThan(this.length()));  // offset < length
        return this._value[offset._value];
    };
    self.concatenate = function concatenate(array) {
        this.ensure(this.isArray());
        this.ensure(array.isArray());
        return new VO.Array(this._value.concat(array._value));
    };
    self.extract = function extract(from, upto) {
        this.ensure(this.isArray());
        this.ensure(from.isNumber());
        this.ensure(upto.isNumber());
        this.ensure(VO.zero.lessEqual(from));  // 0 <= from
        this.ensure(from.lessEqual(upto));  // from <= upto
        this.ensure(upto.lessEqual(this.length()));  // upto <= length
        return new VO.Array(this._value.slice(from._value, upto._value));
    };
    var isArray = Array.isArray || function isArray(object) {
        return (Object.prototype.toString.call(object) === '[object Array]');
    };
    var constructor = function Array(value) {
        VO.ensure(VO.Boolean(isArray(value)));
        this._value = value;
    };
    self.constructor = constructor;
    constructor.prototype = self;
    VO.emptyArray = new constructor([]);
    return constructor;
})();

VO.Object = (function (self) {
    self = self || new VO.Value();
    self.equals = function equals(other) {
        if (this === other) {
            return VO.true;
        }
        if ((this.isObject() === VO.true) && (other.isObject() === VO.true)) {
            var a = this._value;
            var b = other._value;
            var keys = Object.keys(a);
            if (keys.length === Object.keys(b).length) {
                for (var i = 0; i < keys.length; ++i) {
                    var key = keys[i];
                    if (a[key].equals(b[key]) === VO.false) {
                        return VO.false;
                    }
                }
                return VO.true;
            }
        }
        return VO.false;
    };
    self.isObject = function isObject() {
        return VO.true;
    };
    self.hasProperty = function hasProperty(name) {
        this.ensure(this.isObject());
        this.ensure(name.isString());
        return VO.Boolean(this._value.hasOwnProperty(name._value));
    };
    self.value = function value(name) {
        this.ensure(this.isObject());
        this.ensure(name.isString());
        return this._value[name._value];
    };
    self.concatenate = function concatenate(object) {
        this.ensure(this.isObject());
        this.ensure(object.isObject());
        var result = {};
        Object.keys(this._value).forEach(function (key) {
            result[key] = this._value[key];  // copy properties from this
        });
        Object.keys(object._value).forEach(function (key) {
            result[key] = object._value[key];  // argument/replace with properties from object
        });
        return new VO.Object(result);
    };
    self.extract = function extract(/* ...arguments */) {
        this.ensure(this.isObject());
        VO.ensure(VO.Boolean(arguments.length > 0));
        var result = {};
        for (var i = 0; i < arguments.length; ++i) {
            var name = arguments[i];
            VO.ensure(name.isString());
            result[name._value] = this._value[name._value];
        }
        return new VO.Object(result);
    };
    self.names = function names() {
        this.ensure(this.isObject());
        var keys = Object.keys(this._value);
        for (var i = 0; i < keys.length; ++i) {
            keys[i] = new VO.String(keys[i]);  // convert to VO.String
        }
        return new VO.Array(keys);
    };
    var isObject = function isObject(object) {
        return (Object.prototype.toString.call(object) === '[object Object]');
    };
    var constructor = function Object(value) {
        VO.ensure(VO.Boolean(isObject(value)));
        this._value = value;
    };
    self.constructor = constructor;
    constructor.prototype = self;
    VO.emptyObject = new constructor({});
    return constructor;
})();

VO.selfTest = (function () {
    var sampleString = new VO.String("Hello, World!");
    var sampleArray = new VO.Array([
        VO.null,
        VO.true,
        VO.false,
        VO.zero,
        VO.one,
        VO.emptyString,
        VO.emptyArray,
        VO.emptyObject
    ]);
    var sampleObject = new VO.Object({
        "null": VO.null,
        "true": VO.true,
        "false": VO.false,
        "zero": VO.zero,
        "one": VO.one,
        "emptyString": VO.emptyString,
        "emptyArray": VO.emptyArray,
        "emptyObject": VO.emptyObject
    });

    return function selfTest() {
        // Null
        VO.ensure(VO.null.equals(VO.null));
        VO.ensure(VO.null.equals(VO.true).not());
        VO.ensure(VO.null.equals(VO.false).not());
        VO.ensure(VO.null.equals(VO.zero).not());
        VO.ensure(VO.null.isNull());
        VO.ensure(VO.null.isBoolean().not());
        VO.ensure(VO.null.isNumber().not());
        VO.ensure(VO.null.isString().not());
        VO.ensure(VO.null.isArray().not());
        VO.ensure(VO.null.isObject().not());

        VO.ensure(VO.null.equals(VO.Null()));
        VO.ensure(VO.null.equals(new VO.Null()));
        VO.ensure(VO.Boolean(VO.null === new VO.Null()));

        // Boolean
        VO.ensure(VO.true.equals(VO.null).not());
        VO.ensure(VO.true.equals(VO.true));
        VO.ensure(VO.true.equals(VO.false).not());
        VO.ensure(VO.true.equals(VO.zero).not());
        VO.ensure(VO.true.isNull().not());
        VO.ensure(VO.true.isBoolean());
        VO.ensure(VO.true.isNumber().not());
        VO.ensure(VO.true.isString().not());
        VO.ensure(VO.true.isArray().not());
        VO.ensure(VO.true.isObject().not());

        VO.ensure(VO.false.equals(VO.null).not());
        VO.ensure(VO.false.equals(VO.true).not());
        VO.ensure(VO.false.equals(VO.false));
        VO.ensure(VO.false.equals(VO.zero).not());
        VO.ensure(VO.false.isNull().not());
        VO.ensure(VO.false.isBoolean());
        VO.ensure(VO.false.isNumber().not());
        VO.ensure(VO.false.isString().not());
        VO.ensure(VO.false.isArray().not());
        VO.ensure(VO.false.isObject().not());

        VO.ensure(VO.false.and(VO.false).not());
        VO.ensure(VO.false.and(VO.true).not());
        VO.ensure(VO.true.and(VO.false).not());
        VO.ensure(VO.true.and(VO.true));
        VO.ensure(VO.false.or(VO.false).not());
        VO.ensure(VO.false.or(VO.true));
        VO.ensure(VO.true.or(VO.false));
        VO.ensure(VO.true.or(VO.true));

        VO.ensure(VO.true.equals(VO.Boolean(true)));
        VO.ensure(VO.true.equals(new VO.Boolean(true)));
        VO.ensure(VO.Boolean(VO.true === new VO.Boolean(true)));
        VO.ensure(VO.false.equals(VO.Boolean(false)));
        VO.ensure(VO.false.equals(new VO.Boolean(false)));
        VO.ensure(VO.Boolean(VO.false === new VO.Boolean(false)));

        // Number
        VO.ensure(VO.zero.equals(VO.zero));
        VO.ensure(VO.zero.equals(VO.emptyString).not());
        VO.ensure(VO.zero.equals(VO.emptyArray).not());
        VO.ensure(VO.zero.equals(VO.emptyObject).not());
        VO.ensure(VO.zero.isNull().not());
        VO.ensure(VO.zero.isBoolean().not());
        VO.ensure(VO.zero.isNumber());
        VO.ensure(VO.zero.isString().not());
        VO.ensure(VO.zero.isArray().not());
        VO.ensure(VO.zero.isObject().not());

        VO.ensure(VO.zero.lessThan(VO.minusOne).not());
        VO.ensure(VO.zero.lessThan(VO.zero).not());
        VO.ensure(VO.zero.lessThan(VO.one));
        VO.ensure(VO.zero.lessEqual(VO.minusOne).not());
        VO.ensure(VO.zero.lessEqual(VO.zero));
        VO.ensure(VO.zero.lessEqual(VO.one));
        VO.ensure(VO.zero.greaterEqual(VO.minusOne));
        VO.ensure(VO.zero.greaterEqual(VO.zero));
        VO.ensure(VO.zero.greaterEqual(VO.one).not());
        VO.ensure(VO.zero.greaterThan(VO.minusOne));
        VO.ensure(VO.zero.greaterThan(VO.zero).not());
        VO.ensure(VO.zero.greaterThan(VO.one).not());
        VO.ensure(VO.one.lessThan(VO.zero).not());
        VO.ensure(VO.one.lessThan(VO.two));
        VO.ensure(VO.zero.lessThan(VO.two));
        VO.ensure(VO.zero.plus(VO.zero).equals(VO.zero));
        VO.ensure(VO.zero.plus(VO.one).equals(VO.one));
        VO.ensure(VO.one.plus(VO.zero).equals(VO.one));
        VO.ensure(VO.one.plus(VO.one).equals(VO.two));
        VO.ensure(VO.one.plus(VO.minusOne).equals(VO.zero));
        VO.ensure(VO.zero.times(VO.zero).equals(VO.zero));
        VO.ensure(VO.zero.times(VO.one).equals(VO.zero));
        VO.ensure(VO.one.times(VO.zero).equals(VO.zero));
        VO.ensure(VO.one.times(VO.one).equals(VO.one));
        VO.ensure(VO.one.times(VO.minusOne).equals(VO.minusOne));
        VO.ensure(VO.two.times(VO.minusOne).equals(new VO.Number(-2)));

//        VO.ensure(VO.zero.equals(VO.Number(0)));  // ERROR: VO.Number(0) === undefined
        VO.ensure(VO.zero.equals(new VO.Number(0)));
        VO.ensure(VO.Boolean(VO.zero !== new VO.Number(0)));
        VO.ensure(VO.one.equals(new VO.Number(1)));
        VO.ensure(VO.Boolean(VO.one !== new VO.Number(1)));

        // String
        VO.ensure(VO.emptyString.equals(VO.zero).not());
        VO.ensure(VO.emptyString.equals(VO.emptyString));
        VO.ensure(VO.emptyString.equals(VO.emptyArray).not());
        VO.ensure(VO.emptyString.equals(VO.emptyObject).not());
        VO.ensure(VO.emptyString.isNull().not());
        VO.ensure(VO.emptyString.isBoolean().not());
        VO.ensure(VO.emptyString.isNumber().not());
        VO.ensure(VO.emptyString.isString());
        VO.ensure(VO.emptyString.isArray().not());
        VO.ensure(VO.emptyString.isObject().not());

        // Array
        VO.ensure(VO.emptyArray.equals(VO.zero).not());
        VO.ensure(VO.emptyArray.equals(VO.emptyString).not());
        VO.ensure(VO.emptyArray.equals(VO.emptyArray));
        VO.ensure(VO.emptyArray.equals(VO.emptyObject).not());
        VO.ensure(VO.emptyArray.isNull().not());
        VO.ensure(VO.emptyArray.isBoolean().not());
        VO.ensure(VO.emptyArray.isNumber().not());
        VO.ensure(VO.emptyArray.isString().not());
        VO.ensure(VO.emptyArray.isArray());
        VO.ensure(VO.emptyArray.isObject().not());

        // Object
        VO.ensure(VO.emptyObject.equals(VO.zero).not());
        VO.ensure(VO.emptyObject.equals(VO.emptyString).not());
        VO.ensure(VO.emptyObject.equals(VO.emptyArray).not());
        VO.ensure(VO.emptyObject.equals(VO.emptyObject));
        VO.ensure(VO.emptyObject.isNull().not());
        VO.ensure(VO.emptyObject.isBoolean().not());
        VO.ensure(VO.emptyObject.isNumber().not());
        VO.ensure(VO.emptyObject.isString().not());
        VO.ensure(VO.emptyObject.isArray().not());
        VO.ensure(VO.emptyObject.isObject());
    };
})();
