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
    constructor.prototype = self;
    VO.null = new constructor();
    return function Null() {
        return VO.Null;
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
        this.ensure(from.lessThan(VO.zero).not());  // 0 <= from
        this.ensure(upto.lessThan(from).not());  // from <= upto
        this.ensure(this.length().lessThan(upto).not());  // upto <= length
        return new VO.String(this._value.substring(from._value, upto._value));
    };
    var constructor = function String(value) {
        VO.ensure(VO.Boolean(typeof value === "string"));
        this._value = value;
    };
    constructor.prototype = self;
    return constructor;
})();
