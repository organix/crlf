/*

crlf.js - Language factory foundation

The MIT License (MIT)

Copyright (c) 2018 Dale Schumacher

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

var VO = require("VO.js");
var PEG = require("PEG.js");

var crlf = module.exports;
crlf.version = "0.0.1";

//crlf.log = console.log;
crlf.log = function () {};

crlf.factory = function compile_crlf(source) {  // { "lang": <string>, "ast": <value> }
    VO.ensure(source.hasType(VO.Object));
    VO.ensure(source.hasProperty(VO.String("lang")));
    VO.ensure(source.value(VO.String("lang")).hasType(VO.String));
    VO.ensure(source.hasProperty(VO.String("ast")));
    VO.ensure(source.value(VO.String("ast")).hasType(VO.Value));
    var name = source.value(VO.String("lang"))._value;
    var compiler = crlf.language[name];  // find compiler for specified language
    var ast = source.value(VO.String("ast"));
    crlf.log('language:', name, '->', compiler, ast);
    return compiler(ast);  // compile crlf language ast
};

crlf.language = {};  // language compiler namespace

crlf.language["PEG"] = PEG.factory;

crlf.language["VO"] = (function (compiler) {
    var kind = {};
    compiler = compiler || function compile_VO(ast) {  // { "kind": <string>, ... } | [ ...<expr> ]
        if (ast.hasType(VO.Array) === VO.true) {
            return ast.reduce(function (v, x) {
                return x.append(compiler(v));  // compile expressions
            }, VO.emptyArray);
        }
        VO.ensure(ast.hasType(VO.Object));
        VO.ensure(ast.hasProperty(VO.String("kind")));
        VO.ensure(ast.value(VO.String("kind")).hasType(VO.String));
        var factory = kind[ast.value(VO.String("kind"))._value];
        return factory(ast);
    };
    kind["value"] = (function (factory) {
        factory = factory || function VO_value(ast) {  // { "kind": "value", "value": <value> }
            VO.ensure(ast.hasType(VO.Object));
            VO.ensure(ast.hasProperty(VO.String("kind")));
            VO.ensure(ast.value(VO.String("kind")).equals(VO.String("value")));
            VO.ensure(ast.hasProperty(VO.String("value")));
            VO.ensure(ast.value(VO.String("value")).hasType(VO.Value));
            return VO.ValueExpr(ast.value(VO.String("value")));
        };
        return factory;
    })();
    kind["variable"] = (function (factory) {
        factory = factory || function VO_variable(ast) {  // { "kind": "variable", "name": <string> }
            VO.ensure(ast.hasType(VO.Object));
            VO.ensure(ast.hasProperty(VO.String("kind")));
            VO.ensure(ast.value(VO.String("kind")).equals(VO.String("variable")));
            VO.ensure(ast.hasProperty(VO.String("name")));
            VO.ensure(ast.value(VO.String("name")).hasType(VO.String));
            return VO.VariableExpr(ast.value(VO.String("name")));
        };
        return factory;
    })();
    kind["combination"] = (function (factory) {
        factory = factory || function VO_combination(ast) {  // { "kind": "combination", "operative": <expression>, "parameter": <value> }
            VO.ensure(ast.hasType(VO.Object));
            VO.ensure(ast.hasProperty(VO.String("kind")));
            VO.ensure(ast.value(VO.String("kind")).equals(VO.String("combination")));
            VO.ensure(ast.hasProperty(VO.String("operative")));
            var operative = compiler(ast.value(VO.String("operative")));
            VO.ensure(ast.hasProperty(VO.String("parameter")));
            var parameter = compiler(ast.value(VO.String("parameter")));
            return VO.CombineExpr(operative, parameter);
        };
        return factory;
    })();
    kind["method"] = (function (factory) {
        factory = factory || function VO_method(ast) {  // { "kind": "method", "target": <expression>, "selector": <expression> }
            VO.ensure(ast.hasType(VO.Object));
            VO.ensure(ast.hasProperty(VO.String("kind")));
            VO.ensure(ast.value(VO.String("kind")).equals(VO.String("method")));
            VO.ensure(ast.hasProperty(VO.String("target")));
            var target = compiler(ast.value(VO.String("target")));
            VO.ensure(ast.hasProperty(VO.String("selector")));
            var selector = compiler(ast.value(VO.String("selector")));
            return VO.MethodExpr(target, selector);
        };
        return factory;
    })();
    return compiler;
})();

crlf.language["proof"] = (function (constructor) {
    var kind = {};
    let s_kind = new VO.String("kind");
    let s_sort = new VO.String("sort");
    let s_name = new VO.String("name");
    let s_arguments = new VO.String("arguments");
    let s_index = new VO.String("index");
    let s_bindings = new VO.String("bindings");
    let s_scope = new VO.String("scope");
    let compiler = function compile_proof(ast) {  // { "kind":<string>, ... }
        VO.ensure(ast.hasType(VO.Object));
        VO.ensure(ast.hasProperty(s_kind));
        VO.ensure(ast.value(s_kind).hasType(VO.String));
        var constructor = kind[ast.value(s_kind)._value];
        return new constructor(ast);
    };
//    constructor = constructor || function proof_abt(ast) {
//        return compile(ast);
//    };
//    let prototype = constructor.prototype;
//    prototype.constructor = constructor;
    let Variable = kind["variable"] = (function (prototype) {
        prototype.equals = function equals(that) {
            if (this === that) {
                return VO.true;
            }
            if ((this.hasType(Variable) === VO.true) 
            &&  (that.hasType(Variable) === VO.true)) {
                if (this._sort.equals(that._sort) === VO.false) {
                    return VO.false;
                }
                return this._name.equals(that._name);
            }
            return VO.false;
        };
        prototype.substitute = function substitute(name, abt) {
            VO.ensure(name.hasType(VO.String));
            VO.ensure(abt.hasType(VO.Value));  // [FIXME]? VO.ensure(abt.hasType(VO.Object));
            // replace all occurances of variable `name` with `abt` in `this` target
            if (this._name.equals(name)) {
                VO.ensure(this._sort.equals(abt._sort));  // check type (sort) of substitution
                return abt;
            }
            return this;
        };
        prototype.free_variables = function FV() {
            // return an Object mapping names to free variables in `this` target
            let fv = this._name.bind(this);
            return fv;
        };
        let constructor = function proof_variable(ast) {
            // { "kind":"variable", "sort":<string>, "name":<string> }
            VO.ensure(ast.hasType(VO.Object));
            VO.ensure(ast.hasProperty(s_kind));
            VO.ensure(ast.value(s_kind).equals(new VO.String("variable")));
            VO.ensure(ast.hasProperty(s_sort));
            VO.ensure(ast.value(s_sort).hasType(VO.String));
            this._sort = ast.value(s_sort);
            VO.ensure(ast.hasProperty(s_name));
            VO.ensure(ast.value(s_name).hasType(VO.String));
            this._name = ast.value(s_name);
        };
        prototype.constructor = constructor;
        constructor.prototype = prototype;
        return constructor;
    })(new VO.Value());
    let Operator = kind["operator"] = (function (prototype) {
        prototype.equals = function equals(that) {
            if (this === that) {
                return VO.true;
            }
            if ((this.hasType(Operator) === VO.true) 
            &&  (that.hasType(Operator) === VO.true)) {
                if (this._sort.equals(that._sort) === VO.false) {
                    return VO.false;
                }
                if (this._name.equals(that._name) === VO.false) {
                    return VO.false;
                }
                if (this._arguments.equals(that._arguments) === VO.false) {
                    return VO.false;
                }
                if ((this._index === undefined) && (that._index === undefined)) {
                    return VO.true;
                }
                return this._index.equals(that._index);
            }
            return VO.false;
        };
        prototype.copy = function copy() {  // duplicate object, but with EMPTY argument list
            var ast = VO.emptyObject
                .concatenate(s_kind.bind(new VO.String("operator")))
                .concatenate(s_sort.bind(this._sort))
                .concatenate(s_name.bind(this._name));
            ast = ast.concatenate(s_arguments.bind(VO.emptyArray));  // empty argument list!
            if (this._index !== undefined) {
                ast = ast.concatenate(s_index.bind(this._index));
            }
            return new Operator(ast);
        };
        prototype.substitute = function substitute(name, abt) {
            VO.ensure(name.hasType(VO.String));
            VO.ensure(abt.hasType(VO.Value));  // [FIXME]? VO.ensure(abt.hasType(VO.Object));
            // replace all occurances of variable `name` with `abt` in `this` target
            let value = this.copy();  // make a shallow copy...
            let args = this._arguments.reduce(function (v, x) {
                return x.append(v.substitute(name, abt));
            }, VO.emptyArray);
            value._arguments = args;  // replace arguments
            return value;
        };
        prototype.free_variables = function FV() {
            // return an Object mapping names to free variables in `this` target
            let fv = this._arguments.reduce(function (v, x) {
                return x.concatenate(v.free_variables());
            }, VO.emptyObject);
            return fv;
        };
        let constructor = function proof_operator(ast) {
            // { "kind":"operator", "sort":<string>, "name":<string>, "arguments":<array>[, "index":<value> ]}
            VO.ensure(ast.hasType(VO.Object));
            VO.ensure(ast.hasProperty(s_kind));
            VO.ensure(ast.value(s_kind).equals(new VO.String("operator")));
            VO.ensure(ast.hasProperty(s_sort));
            VO.ensure(ast.value(s_sort).hasType(VO.String));
            this._sort = ast.value(s_sort);
            VO.ensure(ast.hasProperty(s_name));
            VO.ensure(ast.value(s_name).hasType(VO.String));
            this._name = ast.value(s_name);
            VO.ensure(ast.hasProperty(s_arguments));
            VO.ensure(ast.value(s_arguments).hasType(VO.Array));
            this._arguments = ast.value(s_arguments).reduce(function (v, x) {
                return x.append(compiler(v));  // compile abt's
            }, VO.emptyArray);
            if (ast.hasProperty(s_index) === VO.true) {
                VO.ensure(ast.value(s_index).hasType(VO.Value));
                this._index = ast.value(s_index);
            }
        };
        prototype.constructor = constructor;
        constructor.prototype = prototype;
        return constructor;
    })(new VO.Value());
    let Binder = kind["binder"] = (function (prototype) {
        prototype.equals = function equals(that) {
            if (this === that) {
                return VO.true;
            }
            if ((this.hasType(Binder) === VO.true) 
            &&  (that.hasType(Binder) === VO.true)) {
                if (this._bindings.equals(that._bindings) === VO.false) {
                    return VO.false;
                }
                return this._scope.equals(that._scope);
            }
            return VO.false;
        };
        prototype.copy = function copy() {  // duplicate object, but with EMPTY argument list
            var ast = VO.emptyObject
                .concatenate(s_kind.bind(new VO.String("binder")))
                .concatenate(s_bindings.bind(this._sort));
            return new Binder(ast);
        };
        prototype.substitute = function substitute(name, abt) {
            VO.ensure(name.hasType(VO.String));
            VO.ensure(abt.hasType(VO.Value));  // [FIXME]? VO.ensure(abt.hasType(VO.Object));
            // replace all occurances of variable `name` with `abt` in `this` target
            let value = this.copy();  // make a shallow copy...
            if (this._sort.hasProperty(name)) {
                value._scope = this._scope;  // copy scope without substitutions
            } else {
                value._scope = this._scope.substitute(name, abt);  // replace scope
            }
            return value;
        };
        prototype.free_variables = function FV() {
            // return an Object mapping names to free variables in `this` target
            let bv = this._bindings;
            let fv = this._scope.free_variables().reduce(function (n, v, x) {
                if (bv.hasProperty(n)) {
                    return x;  // skip bound variable
                }
                return x.concatenate(n.bind(v));  // copy free variable
            }, VO.emptyObject);
            return fv;
        };
        let constructor = function proof_binder(ast) {
            // { "kind":"binder", "bindings":{..., <string>:<string>}, "scope":<abt> }
            VO.ensure(ast.hasType(VO.Object));
            VO.ensure(ast.hasProperty(s_kind));
            VO.ensure(ast.value(s_kind).equals(new VO.String("binder")));
            VO.ensure(ast.hasProperty(s_bindings));
            VO.ensure(ast.value(s_bindings).hasType(VO.Object));
            this._bindings = ast.value(s_bindings);
            if (ast.hasProperty(s_scope) === VO.true) {
                VO.ensure(ast.value(s_scope).hasType(VO.Object));
                this._scope = compiler(ast.value(s_scope));
            }
        };
        prototype.constructor = constructor;
        constructor.prototype = prototype;
        return constructor;
    })(new VO.Value());
    return compiler;
})();

crlf.selfTest = (function () {

    var test_VO = function test_VO() {
        var context = VO.fromNative({
            "zero": 0,
            "one": 1,
            "two": 2,
            "null": null
        });
        var expr;
        var value;

        expr = crlf.factory(VO.fromNative({
            "lang": "VO",
            "ast": {
                "kind": "value",
                "value": []
            }
        }));
        value = expr.evaluate(context);
        VO.ensure(value.equals(VO.emptyArray));

        expr = crlf.factory(VO.fromNative({
            "lang": "VO",
            "ast": {
                "kind": "combination",
                "operative": {
                    "kind": "method",
                    "target": { "kind": "variable", "name": "one" },
                    "selector": { "kind": "value", "value": "plus" }
                },
                "parameter": [
                    { "kind": "value", "value": 1 }
                ]
            }
        }));
        value = expr.evaluate(context);
        VO.ensure(value.equals(new VO.Number(2)));
    };
    
    var test_proof = function test_proof() {
        var expr;
        var value;
        var arg;
        var expect;

        expr = crlf.factory(VO.fromNative({ "lang":"proof", "ast":  // times(num[2];plus(num[3];x))
            { "kind":"operator", "sort":"Exp", "name":"times", "arguments":[
                { "kind":"operator", "sort":"Exp", "name":"num", "index":2, "arguments":[] },
                { "kind":"operator", "sort":"Exp", "name":"plus", "arguments":[
                    { "kind":"operator", "sort":"Exp", "name":"num", "index":3, "arguments":[] },
                    { "kind":"variable", "sort":"Exp", "name":"x" }
                ]}
            ]}
        }));
        value = expr.free_variables();
        VO.ensure(value.names.equals(VO.fromNative(["x"])));

        expr = crlf.factory(VO.fromNative({ "lang":"proof", "ast":  // ap(lam{τ}(x.x);y)
            { "kind":"operator", "sort":"Term", "name":"ap", "arguments":[
                { "kind":"operator", "sort":"Term", "name":"lam", "arguments":[
                    { "kind":"variable", "sort":"Type", "name":"τ" },
                    { "kind":"binder", "bindings":{"x":"τ"}, "scope":
                        { "kind":"variable", "sort":"Term", "name":"x" }
                    }
                ]},
                { "kind":"variable", "sort":"Term", "name":"y" }
            ]}
        }));
        value = expr.free_variables();
        VO.ensure(value.names.equals(VO.fromNative(["τ", "y"])));

        expr = crlf.factory(VO.fromNative({ "lang":"proof", "ast":  // plus(x;num[1])
            { "kind":"operator", "sort":"Exp", "name":"plus", "arguments":[
                { "kind":"variable", "sort":"Exp", "name":"x" },
                { "kind":"operator", "sort":"Exp", "name":"num", "index":1, "arguments":[] }
            ]}
        }));
        arg = crlf.factory(VO.fromNative({ "lang":"proof", "ast":  // num[0]
            { "kind":"operator", "sort":"Exp", "name":"num", "index":0, "arguments":[] }
        }));
        expect = crlf.factory(VO.fromNative({ "lang":"proof", "ast":  // plus(num[0];num[1])
            { "kind":"operator", "sort":"Exp", "name":"plus", "arguments":[
                { "kind":"operator", "sort":"Exp", "name":"num", "index":0, "arguments":[] },
                { "kind":"operator", "sort":"Exp", "name":"num", "index":1, "arguments":[] }
            ]}
        }));
        value = expr.substitute(new VO.String("x"), arg);
        VO.ensure(value.equals(expect));
    };

    return function selfTest() {
        // test imported packages first
        VO.selfTest();
        PEG.selfTest();
        // then run suite of local tests
        test_VO();
        test_proof();
        return true;  // all tests passed
    };

})();
