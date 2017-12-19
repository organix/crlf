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
    self.ensure = function ensure(predicate) {
        if (predicate !== VO.true) {
            throw new Error("VO.ensure FAILED!");
        }
    };
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
    var constructor = function Number(value) {
        this.ensure(VO.Boolean(typeof value === "number"));
        this._value = value;
    };
    constructor.prototype = self;
    return constructor;
})();
