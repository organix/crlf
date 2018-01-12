/*

VO.js - "crlf/VO" JSON Value Objects

The MIT License (MIT)

Copyright (c) 2017-2018 Dale Schumacher

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

var deepFreeze = (typeof Object.freeze !== "function")
    ? function deepFreeze(obj) { return obj; }  // freeze not supported
    : function deepFreeze(obj) {
        Object.getOwnPropertyNames(obj).forEach(function(name) {
            var value = obj[name];
            if ((value !== null)
            &&  (typeof value === 'object')
            &&  (!Object.isFrozen(value))) {
                deepFreeze(value);
            }
        });
        return Object.freeze(obj);
    };

var isNativeArray = Array.isArray || function isArray(object) {
    return (Object.prototype.toString.call(object) === '[object Array]');
};

var isNativeObject = function isObject(object) {
    return (Object.prototype.toString.call(object) === '[object Object]');
};

VO.throw = function error(value) {  // signal an error
    throw new Error(value);
};

VO.ensure = function ensure(predicate) {  // express an invariant condition
    if (predicate !== VO.true) {
        VO.throw(predicate);
    }
};

VO.fromNative = function fromNative(native) {  // convert native JS value to VO equivalent
    if (native === null) {
        return VO.null;
    } else if (native === true) {
        return VO.true;
    } else if (native === false) {
        return VO.false;
    } else if (typeof native === "number") {
        return new VO.Number(native);
    } else if (typeof native === "string") {
        return new VO.String(native);
    } else if (isNativeArray(native)) {
        var a = [];
        native.forEach(function (v, i) {
            a[i] = VO.fromNative(v);
        });
        return new VO.Array(a);
    } else if (isNativeObject(native)) {
        var o = {};
        Object.keys(native).forEach(function (k) {
            o[k] = VO.fromNative(native[k]);
        });
        return new VO.Object(o);
    }
    VO.throw(native);  // no VO equivalent
};

VO.Value = (function (self) {
    self = self || {};
    self.equals = function equals(that) {
        if (this === that) {
            return VO.true;
        }
        return VO.false;
    };
    self.hasType = function hasType(type) {
        return VO.Boolean(this instanceof type);
    };
    self.combine = function combine(name) {  // data-value combiner is abstract field accessor
        VO.ensure(this.hasType(VO.Value));
        VO.ensure(name.hasType(VO.String));
        var member = this[name._value];
        VO.ensure(VO.Boolean(member !== undefined));
        if (typeof member === 'function') {
            member = new VO.Combiner(member.bind(this));  // wrap native function
        }
        VO.ensure(member.hasType(VO.Value));
        return member;
    };
    self.method = function method(name) {  // create adapter for native method
        VO.ensure(this.hasType(VO.Value));
        var target = this;  // this is the target object
        VO.ensure(name.hasType(VO.String));
        var method = target[name._value];  // extract native method from target
        VO.ensure(VO.Boolean(typeof method === "function"));
        var operation = new VO.Operation(function methodCall(args/*, context*/) {
            VO.ensure(args.hasType(VO.Array));
//            VO.ensure(context.hasType(VO.Object));
            return method.apply(target, args._value);  // call native method on target object
        });
        return operation;  // return operation bound to target object method
    };
    var constructor = function Value() {
//        deepFreeze(this);  // can't freeze Values because we need mutable prototypes
    };
    self.constructor = constructor;
    constructor.prototype = self;
    return constructor;
})();

VO.Combiner = (function (self) {
    self = self || new VO.Value();
    self.combine = function combine(value) {  // delegate to native operative function
        VO.ensure(this.hasType(VO.Combiner));
        VO.ensure(value.hasType(VO.Value));
        return this._oper(value);
    };
    self.concatenate = function concatenate(that) {
        VO.ensure(this.hasType(VO.Combiner));
        var first = this._oper;
        VO.ensure(this.hasType(VO.Combiner));
        var second = that._oper;
        var composition = function composition(value) {
            value = first(value);  // apply this operative first
            value = second(value);  // apply that operative second
            return value;  // return final value
        }
        return new VO.Combiner(composition);
    };
    var constructor = function Combiner(operative) {
        VO.ensure(VO.Boolean(typeof operative === "function"));
        this._oper = operative;
        deepFreeze(this);
    };
    self.constructor = constructor;
    constructor.prototype = self;
    return constructor;
})();

VO.Data = (function (self) {
//    self = self || Object.create(VO.emptyObject);
    self = self || new VO.Value();
    self.asJSON = function asJSON() {
        VO.throw("Not Implemented");  // FIXME!
    };
    var constructor = function Data() {
//        deepFreeze(this);  // can't freeze because we need mutable prototypes
    };
    self.constructor = constructor;
    constructor.prototype = self;
    return constructor;
})();

VO.Null = (function (self) {
    self = self || new VO.Data();
    var constructor = function Null() {
        if (VO.null === undefined) {
            this._value = null;
            VO.null = deepFreeze(this);
        }
        return VO.null;
    };
    self.constructor = constructor;
    constructor.prototype = self;
    VO.null = new constructor();
    return constructor;
})();

VO.Boolean = (function (self) {
    self = self || new VO.Data();
    self.not = function not() {
        VO.ensure(this.hasType(VO.Boolean));
        if (this === VO.false) {
            return VO.true;
        }
        return VO.false;
    };
    self.and = function and(that) {
        VO.ensure(this.hasType(VO.Boolean));
        if (this === VO.false) {
            return VO.false;
        }
        VO.ensure(that.hasType(VO.Boolean));
        return that;
    };
    self.or = function or(that) {
        VO.ensure(this.hasType(VO.Boolean));
        if (this === VO.true) {
            return VO.true;
        }
        VO.ensure(that.hasType(VO.Boolean));
        return that;
    };
    var constructor = function Boolean(value) {
        if (value) {
            if (VO.true === undefined) {
                this._value = true;
                VO.true = deepFreeze(this);
            }
            return VO.true;
        } else {
            if (VO.false === undefined) {
                this._value = false;
                VO.false = deepFreeze(this);
            }
            return VO.false;
        }
    };
    self.constructor = constructor;
    constructor.prototype = self;
    VO.true = new constructor(true);
    VO.false = new constructor(false);
    return constructor;
})();

VO.Number = (function (self) {
    self = self || new VO.Data();
    self.equals = function equals(that) {
        if (this === that) {
            return VO.true;
        }
        if ((this.hasType(VO.Number) === VO.true)
        &&  (that.hasType(VO.Number) === VO.true)) {
            if (this._value === that._value) {
                return VO.true;
            }
        }
        return VO.false;
    };
    self.lessThan = function lessThan(that) {
        VO.ensure(this.hasType(VO.Number));
        VO.ensure(that.hasType(VO.Number));
        return VO.Boolean(this._value < that._value);
    };
    self.lessEqual = function lessEqual(that) {
        VO.ensure(this.hasType(VO.Number));
        VO.ensure(that.hasType(VO.Number));
        return VO.Boolean(this._value <= that._value);
    };
    self.greaterEqual = function greaterEqual(that) {
        VO.ensure(this.hasType(VO.Number));
        VO.ensure(that.hasType(VO.Number));
        return VO.Boolean(this._value >= that._value);
    };
    self.greaterThan = function greaterThan(that) {
        VO.ensure(this.hasType(VO.Number));
        VO.ensure(that.hasType(VO.Number));
        return VO.Boolean(this._value > that._value);
    };
    self.plus = function plus(that) {
        VO.ensure(this.hasType(VO.Number));
        VO.ensure(that.hasType(VO.Number));
        return new VO.Number(this._value + that._value);
    };
    self.times = function times(that) {
        VO.ensure(this.hasType(VO.Number));
        VO.ensure(that.hasType(VO.Number));
        return new VO.Number(this._value * that._value);
    };
    var constructor = function Number(value) {
        VO.ensure(VO.Boolean(typeof value === "number"));
        this._value = value;
        deepFreeze(this);
    };
    self.constructor = constructor;
    constructor.prototype = self;
    VO.minusOne = new constructor(-1);
    VO.zero = new constructor(0);
    VO.one = new constructor(1);
    VO.two = new constructor(2);
    return constructor;
})();

VO.Composite = (function (self) {
//    self = self || Object.create(VO.emptyObject);
    self = self || new VO.Data();
    self.value = function value(selector) {  // extract a component of the composite
        VO.throw("Not Implemented");  // FIXME!
    };
    var constructor = function Composite() {
//        deepFreeze(this);  // can't freeze because we need mutable prototypes
    };
    self.constructor = constructor;
    constructor.prototype = self;
    return constructor;
})();

VO.String = (function (self) {
    self = self || new VO.Composite();
    self.equals = function equals(that) {
        if (this === that) {
            return VO.true;
        }
        if ((this.hasType(VO.String) === VO.true) 
        &&  (that.hasType(VO.String) === VO.true)) {
            if (this._value === that._value) {
                return VO.true;
            }
        }
        return VO.false;
    };
    Object.defineProperty(self, 'length', {
        enumerable: true,
        get: function () {
            VO.ensure(this.hasType(VO.String));
            return new VO.Number(this._value.length);
        }
    });
    self.value = function value(offset) {
        VO.ensure(this.hasType(VO.String));
        VO.ensure(offset.hasType(VO.Number));
        VO.ensure(VO.zero.lessEqual(offset));  // 0 <= offset
        VO.ensure(offset.lessThan(this.length));  // offset < length
        return new VO.Number(this._value.codePointAt(offset._value));
    };
    self.concatenate = function concatenate(that) {
        VO.ensure(this.hasType(VO.String));
        VO.ensure(that.hasType(VO.String));
        return new VO.String(this._value + that._value);
    };
    self.extract = function extract(from, upto) {
        VO.ensure(this.hasType(VO.String));
        VO.ensure(from.hasType(VO.Number));
        VO.ensure(upto.hasType(VO.Number));
        VO.ensure(VO.zero.lessEqual(from));  // 0 <= from
        VO.ensure(from.lessEqual(upto));  // from <= upto
        VO.ensure(upto.lessEqual(this.length));  // upto <= length
        return new VO.String(this._value.slice(from._value, upto._value));
    };
    self.append = function append(value) {
        VO.ensure(this.hasType(VO.String));
        VO.ensure(value.hasType(VO.Number));
        return new VO.String(this._value + String.fromCodePoint(value._value));
    };
    self.bind = function bind(value) {
        VO.ensure(this.hasType(VO.String));
        VO.ensure(value.hasType(VO.Value));
        var obj = {};
        obj[this._value] = value;
        return new VO.Object(obj);
    };
    self.reduce = function reduce(func, value) {
        VO.ensure(this.hasType(VO.String));
        if (typeof func === "function") {
            for (var i = 0; i < this._value.length; ++i) {
                var c = new VO.Number(this._value.codePointAt(i));
                value = func(c, value);
            }
            return value;
        }
        VO.throw("Not Implemented");  // FIXME!
    };
    self.asArray = function asArray() {
        VO.ensure(this.hasType(VO.String));
        return this.reduce(function (c, x) {
            return x.append(c);
        }, VO.emptyArray);
    };
    var constructor = function String(value) {
        if (value === undefined) {
            return VO.emptyString;
        }
        VO.ensure(VO.Boolean(typeof value === "string"));
        this._value = value;
        deepFreeze(this);
    };
    self.constructor = constructor;
    constructor.prototype = self;
    VO.emptyString = new constructor("");
    return constructor;
})();

VO.Array = (function (self) {
    self = self || new VO.Composite();
    self.equals = function equals(that) {
        if (this === that) {
            return VO.true;
        }
        if ((this.hasType(VO.Array) === VO.true)
        &&  (that.hasType(VO.Array) === VO.true)) {
            var a = this._value;
            var b = that._value;
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
    Object.defineProperty(self, 'length', {
        enumerable: true,
        get: function () {
            VO.ensure(this.hasType(VO.Array));
            return new VO.Number(this._value.length);
        }
    });
    self.value = function value(offset) {
        VO.ensure(this.hasType(VO.Array));
        VO.ensure(offset.hasType(VO.Number));
        VO.ensure(VO.zero.lessEqual(offset));  // 0 <= offset
        VO.ensure(offset.lessThan(this.length));  // offset < length
        return this._value[offset._value];
    };
    self.concatenate = function concatenate(that) {
        VO.ensure(this.hasType(VO.Array));
        VO.ensure(that.hasType(VO.Array));
        return new VO.Array(this._value.concat(that._value));
    };
    self.extract = function extract(from, upto) {
        VO.ensure(this.hasType(VO.Array));
        VO.ensure(from.hasType(VO.Number));
        VO.ensure(upto.hasType(VO.Number));
        VO.ensure(VO.zero.lessEqual(from));  // 0 <= from
        VO.ensure(from.lessEqual(upto));  // from <= upto
        VO.ensure(upto.lessEqual(this.length));  // upto <= length
        return new VO.Array(this._value.slice(from._value, upto._value));
    };
    self.append = function append(value) {
        VO.ensure(this.hasType(VO.Array));
        VO.ensure(value.hasType(VO.Value));
        return new VO.Array(this._value.concat(value));
    };
    self.reduce = function reduce(func, value) {
        VO.ensure(this.hasType(VO.Array));
        if (typeof func === "function") {
            for (var i = 0; i < this._value.length; ++i) {
                value = func(this._value[i], value);
            }
            return value;
        }
        VO.throw("Not Implemented");  // FIXME!
    };
    self.asString = function asString() {
        VO.ensure(this.hasType(VO.Array));
        return this.reduce(function (v, x) {
            VO.ensure(v.hasType(VO.Number));
            return x.append(v);
        }, VO.emptyString);
    };
    var constructor = function Array(value) {
        if (value === undefined) {
            return VO.emptyArray;
        }
        VO.ensure(VO.Boolean(isNativeArray(value)));
        this._value = value;
        deepFreeze(this);
    };
    self.constructor = constructor;
    constructor.prototype = self;
    VO.emptyArray = new constructor([]);
    return constructor;
})();

VO.Object = (function (self) {
    self = self || new VO.Composite();
    self.equals = function equals(that) {
        if (this === that) {
            return VO.true;
        }
        if ((this.hasType(VO.Object) === VO.true) 
        &&  (that.hasType(VO.Object) === VO.true)) {
            var a = this._value;
            var b = that._value;
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
    self.hasProperty = function hasProperty(name) {
        VO.ensure(this.hasType(VO.Object));
        VO.ensure(name.hasType(VO.String));
        return VO.Boolean(this._value.hasOwnProperty(name._value));
    };
    self.value = function value(name) {
        VO.ensure(this.hasType(VO.Object));
        VO.ensure(name.hasType(VO.String));
        return this._value[name._value];
    };
    self.concatenate = function concatenate(that) {
        VO.ensure(this.hasType(VO.Object));
        VO.ensure(that.hasType(VO.Object));
        var result = {};
        Object.keys(this._value).forEach(function (key) {
            result[key] = this._value[key];  // copy properties from this
        }, this);
        Object.keys(that._value).forEach(function (key) {
            result[key] = that._value[key];  // argument/replace with properties from that
        });
        return new VO.Object(result);
    };
    self.extract = function extract(/* ...arguments */) {
        VO.ensure(this.hasType(VO.Object));
        VO.ensure(VO.Boolean(arguments.length >= 0));
        var result = {};
        for (var i = 0; i < arguments.length; ++i) {
            var name = arguments[i];
            VO.ensure(name.hasType(VO.String));
            result[name._value] = this._value[name._value];
        }
        return new VO.Object(result);
    };
    self.names = function names() {
        VO.ensure(this.hasType(VO.Object));
        var keys = Object.keys(this._value);
        for (var i = 0; i < keys.length; ++i) {
            keys[i] = new VO.String(keys[i]);  // convert to VO.String
        }
        return new VO.Array(keys);
    };
    self.append = function append(name, value) {
        VO.ensure(this.hasType(VO.Object));
        VO.ensure(name.hasType(VO.String));
        VO.ensure(value.hasType(VO.Value));
        var obj = {};
        obj[name._value] = value;
        return this.concatenate(new VO.Object(obj));
    };
    self.reduce = function reduce(func, value) {
        VO.ensure(this.hasType(VO.Object));
        if (typeof func === "function") {
            var keys = Object.keys(this._value);
            for (var i = 0; i < keys.length; ++i) {
                var key = keys[i];
                value = func(new VO.String(key), this._value[key], value);
            }
            return value;
        }
        VO.throw("Not Implemented");  // FIXME!
    };
/*
    self.names = function names() {
        VO.ensure(this.hasType(VO.Object));
        return this.reduce(function (n, v, x) {
            return x.append(n);
        }, VO.emptyArray);
    };
*/
    var constructor = function Object(value) {
        if (value === undefined) {
            return VO.emptyObject;
        }
        VO.ensure(VO.Boolean(isNativeObject(value)));
        this._value = value;
        deepFreeze(this);
    };
    self.constructor = constructor;
    constructor.prototype = self;
    VO.emptyObject = new constructor({});
    return constructor;
})();

VO.Expression = (function (self) {
//    self = self || Object.create(VO.emptyObject);
    self = self || new VO.Value();
    self.evaluate = function evaluate(/* context */) {
        VO.throw("Not Implemented");
    };
    var constructor = function Expression() {
//        deepFreeze(this);  // can't freeze Expressions because we need mutable prototypes
    };
    self.constructor = constructor;
    constructor.prototype = self;
    return constructor;
})();

VO.ValueExpr = (function (self) {
    self = self || new VO.Expression();
    self.evaluate = function evaluate(/* context */) {
        return this._value;
    };
    var constructor = function ValueExpr(value) {
        VO.ensure(value.hasType(VO.Value));
        this._value = value;
    };
    self.constructor = constructor;
    constructor.prototype = self;
    return constructor;
})();

VO.VariableExpr = (function (self) {
    self = self || new VO.Expression();
    self.evaluate = function evaluate(context) {
//        VO.ensure(context.hasType(VO.Object));
        return context.value(this._name);
    };
    var constructor = function VariableExpr(name) {
        VO.ensure(name.hasType(VO.String));
        this._name = name;
    };
    self.constructor = constructor;
    constructor.prototype = self;
    return constructor;
})();

VO.CombineExpr = (function (self) {
    self = self || new VO.Expression();
    self.evaluate = function evaluate(context) {
//        VO.ensure(context.hasType(VO.Object));
        var operation = this._expr.evaluate(context);
//        VO.ensure(VO.Boolean(typeof operation.operate === "function"));
        VO.ensure(operation.hasType(VO.Operation));
        return operation.operate(this._data, context);
    };
    var constructor = function CombineExpr(expr, data) {
        VO.ensure(expr.hasType(VO.Expression));
        VO.ensure(data.hasType(VO.Value));
        this._expr = expr;
        this._data = data;
    };
    self.constructor = constructor;
    constructor.prototype = self;
    return constructor;
})();

VO.Operation = (function (self) {
    self = self || new VO.Expression();
    self.evaluate = function evaluate(context) {
//        VO.ensure(context.hasType(VO.Object));
        return this;  // operations evaluate to themselves
    };
    self.operate = function operate(value, context) {
        VO.ensure(value.hasType(VO.Value));
//        VO.ensure(context.hasType(VO.Object));
        var operative = this._oper;
        return operative(value, context);
    };
    self.concatenate = function concatenate(that) {
        VO.ensure(this.hasType(VO.Operation));
        var first = this._oper;
        VO.ensure(that.hasType(VO.Operation));
        var second = that._oper;
        var composition = function composition(value, context) {
            value = first(value, context);  // apply this operative first
            value = second(value, context);  // apply that operative second
            return value;  // return final value
        }
        return new VO.Operation(composition);
    };
    var constructor = function Operation(operative) {
        VO.ensure(VO.Boolean(typeof operative === "function"));
        this._oper = operative;
        deepFreeze(this);
    };
    self.constructor = constructor;
    constructor.prototype = self;
    /* return unevaluated argument */
    VO.quoteOper = new constructor(function quoteOper(value/*, context*/) {
        VO.ensure(value.hasType(VO.Value));
//        VO.ensure(context.hasType(VO.Object));
        return value;
    });
    /* return array of evaluated arguments */
    VO.arrayOper = new constructor(function arrayOper(array, context) {
        VO.ensure(array.hasType(VO.Array));
//        VO.ensure(context.hasType(VO.Object));
        return array.reduce(function (v, x) {
            VO.ensure(v.hasType(VO.Expression));
            return x.append(v.evaluate(context));
        }, VO.emptyArray);
    });
    /* evaluate expressions sequentially, returning the last value */
    VO.sequentialOper = new constructor(function sequentialOper(array, context) {
        VO.ensure(array.hasType(VO.Array));
//        VO.ensure(context.hasType(VO.Object));
        return array.reduce(function (v, x) {
            VO.ensure(v.hasType(VO.Expression));
            return v.evaluate(context);
        }, VO.null);
    });
    return constructor;
})();

VO.MethodExpr = (function (self) {
    self = self || new VO.Expression();
    self.evaluate = function evaluate(context) {
        VO.ensure(context.hasType(VO.Object));
        var selector = this._name.evaluate(context);  // evaluate name expression to get selector
        VO.ensure(selector.hasType(VO.String));
        var target = this._this.evaluate(context);  // evaluate this expression to get target
        VO.ensure(target.hasType(VO.Value));
        var operation = target.method(selector);  // adapter for native method
        operation = VO.arrayOper.concatenate(operation);  // evaluate arguments before calling method
        return operation;  // return applicative bound to target object
    };
    var constructor = function MethodExpr(target, selector) {
        VO.ensure(target.hasType(VO.Expression));
        VO.ensure(selector.hasType(VO.Expression));
        this._this = target;
        this._name = selector;
    };
    self.constructor = constructor;
    constructor.prototype = self;
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
    var sampleContext = sampleObject.concatenate(new VO.Object({
        "quote": VO.quoteOper,
        "arrayOf": VO.arrayOper
    }));
    var tmp;

    return function selfTest() {
        // Null
        VO.ensure(VO.null.equals(VO.null));
        VO.ensure(VO.null.equals(VO.true).not());
        VO.ensure(VO.null.equals(VO.false).not());
        VO.ensure(VO.null.equals(VO.zero).not());
        VO.ensure(VO.null.hasType(VO.Value));
        VO.ensure(VO.null.hasType(VO.Data));
        VO.ensure(VO.null.hasType(VO.Null));
        VO.ensure(VO.null.hasType(VO.Boolean).not());
        VO.ensure(VO.null.hasType(VO.Number).not());
        VO.ensure(VO.null.hasType(VO.String).not());
        VO.ensure(VO.null.hasType(VO.Array).not());
        VO.ensure(VO.null.hasType(VO.Object).not());

        VO.ensure(VO.null.equals(VO.Null()));
        VO.ensure(VO.null.equals(new VO.Null()));
        VO.ensure(VO.Boolean(VO.null === new VO.Null()));
        VO.ensure(VO.Boolean(new VO.Null() === new VO.Null()));

        // Boolean
        VO.ensure(VO.true.equals(VO.null).not());
        VO.ensure(VO.true.equals(VO.true));
        VO.ensure(VO.true.equals(VO.false).not());
        VO.ensure(VO.true.equals(VO.zero).not());
        VO.ensure(VO.true.hasType(VO.Value));
        VO.ensure(VO.true.hasType(VO.Data));
        VO.ensure(VO.true.hasType(VO.Null).not());
        VO.ensure(VO.true.hasType(VO.Boolean));
        VO.ensure(VO.true.hasType(VO.Number).not());
        VO.ensure(VO.true.hasType(VO.String).not());
        VO.ensure(VO.true.hasType(VO.Array).not());
        VO.ensure(VO.true.hasType(VO.Object).not());

        VO.ensure(VO.false.equals(VO.null).not());
        VO.ensure(VO.false.equals(VO.true).not());
        VO.ensure(VO.false.equals(VO.false));
        VO.ensure(VO.false.equals(VO.zero).not());
        VO.ensure(VO.false.hasType(VO.Value));
        VO.ensure(VO.false.hasType(VO.Data));
        VO.ensure(VO.false.hasType(VO.Null).not());
        VO.ensure(VO.false.hasType(VO.Boolean));
        VO.ensure(VO.false.hasType(VO.Number).not());
        VO.ensure(VO.false.hasType(VO.String).not());
        VO.ensure(VO.false.hasType(VO.Array).not());
        VO.ensure(VO.false.hasType(VO.Object).not());

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
        VO.ensure(VO.Boolean(new VO.Boolean(true) === new VO.Boolean(true)));
        VO.ensure(VO.false.equals(VO.Boolean(false)));
        VO.ensure(VO.false.equals(new VO.Boolean(false)));
        VO.ensure(VO.Boolean(VO.false === new VO.Boolean(false)));
        VO.ensure(VO.Boolean(new VO.Boolean(false) === new VO.Boolean(false)));

        // Number
        VO.ensure(VO.zero.equals(VO.zero));
        VO.ensure(VO.zero.equals(VO.emptyString).not());
        VO.ensure(VO.zero.equals(VO.emptyArray).not());
        VO.ensure(VO.zero.equals(VO.emptyObject).not());
        VO.ensure(VO.zero.hasType(VO.Value));
        VO.ensure(VO.zero.hasType(VO.Data));
        VO.ensure(VO.zero.hasType(VO.Null).not());
        VO.ensure(VO.zero.hasType(VO.Boolean).not());
        VO.ensure(VO.zero.hasType(VO.Number));
        VO.ensure(VO.zero.hasType(VO.String).not());
        VO.ensure(VO.zero.hasType(VO.Array).not());
        VO.ensure(VO.zero.hasType(VO.Object).not());

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
        VO.ensure(VO.emptyString.hasType(VO.Value));
        VO.ensure(VO.emptyString.hasType(VO.Data));
        VO.ensure(VO.emptyString.hasType(VO.Null).not());
        VO.ensure(VO.emptyString.hasType(VO.Boolean).not());
        VO.ensure(VO.emptyString.hasType(VO.Number).not());
        VO.ensure(VO.emptyString.hasType(VO.Composite));
        VO.ensure(VO.emptyString.hasType(VO.String));
        VO.ensure(VO.emptyString.hasType(VO.Array).not());
        VO.ensure(VO.emptyString.hasType(VO.Object).not());

        VO.ensure(VO.emptyString.length.equals(VO.zero));
        VO.ensure(sampleString.length.equals(new VO.Number(13)));
        VO.ensure(sampleString.value(VO.zero).equals(new VO.Number(72)));  // "H"
        VO.ensure(sampleString.value(new VO.Number(6)).equals(new VO.Number(32)));  // " "
        VO.ensure(sampleString.value(sampleString.length.plus(VO.minusOne)).equals(new VO.Number(33)));  // "!"
        VO.ensure(sampleString.extract(VO.zero, VO.zero).equals(VO.emptyString));
        VO.ensure(sampleString.extract(VO.zero, VO.one).length.equals(VO.one));
        VO.ensure(sampleString.extract(VO.one, VO.one).length.equals(VO.zero));
        VO.ensure(sampleString.extract(VO.zero, sampleString.length).equals(sampleString));
        VO.ensure(sampleString.extract(VO.zero, new VO.Number(5))
                  .equals(new VO.String("Hello")));
        VO.ensure(sampleString.extract(new VO.Number(7), sampleString.length.plus(VO.minusOne))
                  .equals(new VO.String("World")));
        VO.ensure(VO.emptyString.concatenate(VO.emptyString).equals(VO.emptyString));
        VO.ensure(sampleString.concatenate(VO.emptyString).equals(sampleString));
        VO.ensure(VO.emptyString.concatenate(sampleString).equals(sampleString));
        VO.ensure(sampleString.extract(VO.zero, new VO.Number(6))
                  .concatenate(sampleString.extract(new VO.Number(6), sampleString.length))
                  .equals(sampleString));
        VO.ensure(new VO.String("foo").bind(new VO.Number(42)).equals(VO.fromNative({ "foo": 42 })));

        VO.ensure(VO.emptyString.append(new VO.Number(72)).append(new VO.Number(105))
                  .equals(new VO.String("Hi")));
        VO.ensure(sampleString.reduce(
                      function (c, x) {
                          return x.plus(VO.one);
                      }, VO.zero)
                  .equals(sampleString.length));

//        VO.ensure(VO.emptyString.equals(VO.String("")));  // ERROR: VO.String("") === undefined
        VO.ensure(VO.emptyString.equals(new VO.String("")));
        VO.ensure(VO.Boolean(VO.emptyString !== new VO.String("")));
        VO.ensure(VO.Boolean(VO.emptyString === new VO.String()));

        // Array
        VO.ensure(VO.emptyArray.equals(VO.zero).not());
        VO.ensure(VO.emptyArray.equals(VO.emptyString).not());
        VO.ensure(VO.emptyArray.equals(VO.emptyArray));
        VO.ensure(VO.emptyArray.equals(VO.emptyObject).not());
        VO.ensure(VO.emptyArray.hasType(VO.Value));
        VO.ensure(VO.emptyArray.hasType(VO.Data));
        VO.ensure(VO.emptyArray.hasType(VO.Null).not());
        VO.ensure(VO.emptyArray.hasType(VO.Boolean).not());
        VO.ensure(VO.emptyArray.hasType(VO.Number).not());
        VO.ensure(VO.emptyArray.hasType(VO.Composite));
        VO.ensure(VO.emptyArray.hasType(VO.String).not());
        VO.ensure(VO.emptyArray.hasType(VO.Array));
        VO.ensure(VO.emptyArray.hasType(VO.Object).not());

        VO.ensure(VO.emptyArray.length.equals(VO.zero));
        VO.ensure(sampleArray.length.equals(new VO.Number(8)));
        VO.ensure(sampleArray.value(VO.zero).equals(VO.null));
        VO.ensure(sampleArray.value(new VO.Number(4)).equals(new VO.Number(1)));
        VO.ensure(sampleArray.value(sampleArray.length.plus(VO.minusOne)).equals(VO.emptyObject));
        VO.ensure(sampleArray.extract(VO.zero, VO.zero).equals(VO.emptyArray));
        VO.ensure(sampleArray.extract(VO.zero, VO.one).length.equals(VO.one));
        VO.ensure(sampleArray.extract(VO.one, VO.one).length.equals(VO.zero));
        VO.ensure(sampleArray.extract(VO.zero, sampleArray.length).equals(sampleArray));
        VO.ensure(VO.emptyArray.concatenate(VO.emptyArray).equals(VO.emptyArray));
        VO.ensure(sampleArray.concatenate(VO.emptyArray).equals(sampleArray));
        VO.ensure(VO.emptyArray.concatenate(sampleArray).equals(sampleArray));
        VO.ensure(sampleArray.extract(VO.zero, new VO.Number(4))
                  .concatenate(sampleArray.extract(new VO.Number(4), sampleArray.length))
                  .equals(sampleArray));

        VO.ensure(sampleArray.reduce(
                      function (v, x) {
                          return x.plus(VO.one);
                      }, VO.zero)
                  .equals(sampleArray.length));
        VO.ensure(VO.emptyArray.append(new VO.Number(72)).append(new VO.Number(105))
                  .reduce(
                      function (v, x) {
                          return x.append(v);
                      }, VO.emptyString)
                  .equals(new VO.String("Hi")));
        VO.ensure(sampleString.asArray().asString().equals(sampleString));

//        VO.ensure(VO.emptyArray.equals(VO.Array([])));  // ERROR: VO.Array([]) === undefined
        VO.ensure(VO.emptyArray.equals(new VO.Array([])));
        VO.ensure(VO.Boolean(VO.emptyArray !== new VO.Array([])));
        VO.ensure(VO.Boolean(VO.emptyArray === new VO.Array()));

        // Object
        VO.ensure(VO.emptyObject.equals(VO.zero).not());
        VO.ensure(VO.emptyObject.equals(VO.emptyString).not());
        VO.ensure(VO.emptyObject.equals(VO.emptyArray).not());
        VO.ensure(VO.emptyObject.equals(VO.emptyObject));
        VO.ensure(VO.emptyObject.hasType(VO.Value));
        VO.ensure(VO.emptyObject.hasType(VO.Data));
        VO.ensure(VO.emptyObject.hasType(VO.Null).not());
        VO.ensure(VO.emptyObject.hasType(VO.Boolean).not());
        VO.ensure(VO.emptyObject.hasType(VO.Number).not());
        VO.ensure(VO.emptyObject.hasType(VO.Composite));
        VO.ensure(VO.emptyObject.hasType(VO.String).not());
        VO.ensure(VO.emptyObject.hasType(VO.Array).not());
        VO.ensure(VO.emptyObject.hasType(VO.Object));

        VO.ensure(VO.emptyObject.names().equals(VO.emptyArray));
        VO.ensure(sampleObject.names().length.equals(new VO.Number(8)));
        VO.ensure(VO.emptyObject.hasProperty(new VO.String("zero")).not());
        VO.ensure(sampleObject.hasProperty(new VO.String("zero")));
        VO.ensure(sampleObject.hasProperty(new VO.String("none")).not());
        VO.ensure(sampleObject.value(new VO.String("zero")).equals(VO.zero));
        VO.ensure(sampleObject.value(new VO.String("one")).equals(new VO.Number(1)));
        VO.ensure(sampleObject.value(new VO.String("emptyObject")).equals(VO.emptyObject));
        VO.ensure(sampleObject.extract().equals(VO.emptyObject));
        VO.ensure(sampleObject.extract(new VO.String("zero")).names().length.equals(VO.one));
        VO.ensure(sampleObject.extract(new VO.String("zero"), new VO.String("one"))
                  .names().length.equals(VO.two));
        VO.ensure(sampleObject.extract(new VO.String("zero"), new VO.String("one"))
                  .value(new VO.String("zero")).equals(VO.zero));
        VO.ensure(sampleObject.extract(new VO.String("zero"), new VO.String("one"))
                  .value(new VO.String("one")).equals(VO.one));
        VO.ensure(sampleObject.extract.apply(sampleObject, sampleObject.names()._value)
                  .equals(sampleObject));
        VO.ensure(VO.emptyObject.concatenate(VO.emptyObject).equals(VO.emptyObject));
        VO.ensure(sampleObject.concatenate(VO.emptyObject).equals(sampleObject));
        VO.ensure(VO.emptyObject.concatenate(sampleObject).equals(sampleObject));
        VO.ensure(sampleObject.extract(new VO.String("zero"))
                  .concatenate(sampleObject.extract(new VO.String("one")))
                  .equals(sampleObject.extract(new VO.String("one"), new VO.String("zero"))));
        tmp = new VO.Object({ a: VO.true, b: VO.false })
            .concatenate(new VO.Object({ b: VO.true, c: VO.true }));
        VO.ensure(tmp.value(new VO.String("a"))
                  .and(tmp.value(new VO.String("b")))
                  .and(tmp.value(new VO.String("c"))));

        VO.ensure(sampleObject.reduce(
                      function (n, v, x) {
                          return x.plus(VO.one);
                      }, VO.zero)
                  .equals(sampleObject.names().length));
        VO.ensure(sampleObject.reduce(
                      function (n, v, x) {
                          return x.append(n);
                      }, VO.emptyArray)
                  .equals(sampleObject.names()));
        VO.ensure(VO.emptyObject
                  .append(new VO.String("space"), new VO.Number(33))
                  .append(new VO.String("bang"), new VO.Number(34))
                  .reduce(
                      function (n, v, x) {
                          return x.times(v);
                      }, VO.one)
                  .equals(new VO.Number(33 * 34)));

//        VO.ensure(VO.emptyObject.equals(VO.Object({})));  // ERROR: VO.Object({}) === undefined
        VO.ensure(VO.emptyObject.equals(new VO.Object({})));
        VO.ensure(VO.Boolean(VO.emptyObject !== new VO.Object({})));
        VO.ensure(VO.Boolean(VO.emptyObject === new VO.Object()));

        VO.ensure(VO.fromNative(
            {
                "null": null,
                true: true,
                false: false,
                zero: 0,
                one: 1,
                emptyString: "",
                emptyArray: [],
                emptyObject: {}
            }).equals(sampleObject));
        VO.ensure(VO.fromNative(
            JSON.parse(
                '{"null":null, "true":true, "false":false, "zero":0, "one":1, "emptyString":"", "emptyArray":[], "emptyObject":{}}'
            )).equals(sampleObject));

        // Expression
        VO.ensure(new VO.ValueExpr(new VO.Null()).evaluate(VO.emptyObject).equals(VO.null));
        VO.ensure(new VO.ValueExpr(new VO.Number(2)).evaluate(sampleObject).equals(VO.two));
        VO.ensure(new VO.ValueExpr(sampleArray).evaluate(sampleObject).equals(sampleArray));
        VO.ensure(new VO.ValueExpr(sampleObject).evaluate(sampleObject).equals(sampleObject));

        VO.ensure(new VO.VariableExpr(new VO.String("one")).evaluate(sampleObject).equals(VO.one));
        VO.ensure(new VO.VariableExpr(new VO.String("emptyObject"))
                  .evaluate(sampleContext).equals(VO.emptyObject));
        VO.ensure(new VO.VariableExpr(new VO.String("arrayOf"))
                  .evaluate(sampleContext).equals(VO.arrayOper));

        VO.ensure(new VO.CombineExpr(VO.quoteOper, sampleArray)
                  .evaluate(VO.emptyObject).equals(sampleArray));
        VO.ensure(new VO.CombineExpr(VO.arrayOper, 
                                     sampleObject.reduce(function (n, v, x) {
                                         return x.append(new VO.VariableExpr(n));
                                     }, VO.emptyArray))
                  .evaluate(sampleObject).equals(sampleArray));
        VO.ensure(new VO.CombineExpr(new VO.VariableExpr(new VO.String("arrayOf")), 
                                     sampleObject.reduce(function (n, v, x) {
                                         return x.append(new VO.VariableExpr(n));
                                     }, VO.emptyArray))
                  .evaluate(sampleContext).equals(sampleArray));
        VO.ensure(new VO.CombineExpr(
                        new VO.MethodExpr(
                            new VO.VariableExpr(new VO.String("one")), 
                            new VO.ValueExpr(new VO.String("plus"))),
                        VO.emptyArray.append(
                            new VO.ValueExpr(VO.one)))
                  .evaluate(sampleContext).equals(new VO.Number(2)));

        return true;  // all tests passed
    };
})();

deepFreeze(VO);  // LOCK DOWN EVERYTHING!
