/*

term.js - "crlf/term" Term Rewriting Algebra/Calculus

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

var VO = require("VO.js");

var term = module.exports;
term.version = "0.0.1";

term.log = console.log;
//term.log = function () {};

var s_kind = VO.String("kind");
var s_sort = VO.String("sort");
var s_name = VO.String("name");

term.kind = {};  // term compiler namespace

term.factory = function compile_term(source) {  // { "kind": <string>, ... }
    term.log('term.factory:', source);
    VO.ensure(source.hasType(VO.Object));
    VO.ensure(source.hasProperty(s_kind));
    var compiler = term.kind[source.value(s_kind)._value];
    var value = compiler(source);
    term.log('term.factory -> value:', value);
    VO.ensure(value.hasType(term.Term));
    return value;
};

term.kind["operator"] = (function (factory) {
    factory = factory || function term_terminal(ast) {
        // { "kind":"operator", "sort":<string>, "name":<string> }
        // { "kind":"operator", "sort":<string>, "name":<string>, "index":<value> }
        // { "kind":"operator", "sort":<string>, "name":<string>, "arguments":<array> }
        VO.ensure(ast.hasType(VO.Object));
        VO.ensure(ast.hasProperty(s_kind));
        VO.ensure(ast.value(s_kind).equals(VO.String("operator")));
        VO.ensure(ast.hasProperty(s_sort));
        VO.ensure(ast.value(s_sort).hasType(VO.String));
        VO.ensure(ast.hasProperty(s_name));
        VO.ensure(ast.value(s_name).hasType(VO.String));
        var _index = undefined;
        if (ast.hasProperty(VO.String("index")) === VO.true) {
            _index = ast.value(VO.String("index"));
            VO.ensure(_index.hasType(VO.Value));
        }
        var _args = VO.emptyArray;
        if (ast.hasProperty(VO.String("arguments")) === VO.true) {
            _args = ast.value(VO.String("arguments"));
            VO.ensure(_args.hasType(VO.Array));
            _args = _args.reduce(function (v, x) {
                var t = term.factory(v);  // compile argument terms
                return x.append(t);
            }, VO.emptyArray);
        }
        return term.Operator(ast.value(s_sort), ast.value(s_name), _args, _index);
    };
    return factory;
})();

term.kind["variable"] = (function (factory) {
    factory = factory || function term_terminal(ast) {
        // { "kind":"variable", "sort":<string>, "name":<string> }
        VO.ensure(ast.hasType(VO.Object));
        VO.ensure(ast.hasProperty(s_kind));
        VO.ensure(ast.value(s_kind).equals(VO.String("variable")));
        VO.ensure(ast.hasProperty(s_sort));
        VO.ensure(ast.value(s_sort).hasType(VO.String));
        VO.ensure(ast.hasProperty(s_name));
        VO.ensure(ast.value(s_name).hasType(VO.String));
        return term.Variable(ast.value(s_sort), ast.value(s_name));
    };
    return factory;
})();

term.Term = (function (proto) {  // abstract base-class
    proto = proto || VO.Value();
    proto.replace = function replace(name, value) {
        // return a term created by substituting the value for the named variable in this term
        VO.ensure(name.hasType(VO.String));
        VO.ensure(value.hasType(term.Term));
        VO.throw("Not Implemented");  // FIXME!
    };
    proto.FV = function FV() {  // free variables
        // return an object describing the free variables in this term (name -> sort)
        VO.throw("Not Implemented");  // FIXME!
    };
    var constructor = function Term() {
        if (!(this instanceof Term)) { return new Term(); }  // if called without "new" keyword...
    };
    proto.constructor = constructor;
    constructor.prototype = proto;
    return constructor;
})();

term.Operator = (function (proto) {
    proto = proto || term.Term();
    proto.FV = function FV() {  // free variables
        let fv = this._args.reduce(function (v, x) {
            VO.ensure(v.hasType(term.Term));
            return x.concatenate(v.FV());
        }, VO.emptyObject);
        return fv;
    };
    var constructor = function Operator(sort, name, args, index) {
        if (!(this instanceof Operator)) { return new Operator(sort, name, args, index); }  // if called without "new" keyword...
        VO.ensure(sort.hasType(VO.String));
        VO.ensure(name.hasType(VO.String));
        VO.ensure(args.hasType(VO.Array));
        this._sort = sort;
        this._name = name;
        this._args = args;
        if (index !== undefined) {
            VO.ensure(index.hasType(VO.Value));
            this._index = index;
        }
    };
    proto.constructor = constructor;
    constructor.prototype = proto;
    return constructor;
})();

term.Variable = (function (proto) {
    proto = proto || term.Term();
    proto.FV = function FV() {  // free variables
        let fv = this._name.bind(this._sort);
        return fv;
    };
    var constructor = function Variable(sort, name) {
        if (!(this instanceof Variable)) { return new Variable(sort, name); }  // if called without "new" keyword...
        VO.ensure(sort.hasType(VO.String));
        VO.ensure(name.hasType(VO.String));
        this._sort = sort;
        this._name = name;
    };
    proto.constructor = constructor;
    constructor.prototype = proto;
    return constructor;
})();

term.Binder = (function (proto) {
    proto = proto || term.Term();
    proto.FV = function FV() {  // free variables
        var fv = this._scope.FV();
        let bv = this._bindings;
        fv = fv.reduce(function (n, v, x) {
            if (bv.hasProperty(n)) {
                return x;  // skip bound variable
            }
            return x.concatenate(n.bind(v));  // copy free variable
        }, VO.emptyObject);
        return fv;
    };
    var constructor = function Binder(bindings, scope) {
        if (!(this instanceof Binder)) { return new Binder(bindings, scope); }  // if called without "new" keyword...
        VO.ensure(bindings.hasType(VO.Object));  // (name -> sort)
        VO.ensure(scope.hasType(term.Term));
        this._bindings = bindings;
        this._scope = scope;
    };
    proto.constructor = constructor;
    constructor.prototype = proto;
    return constructor;
})();

term.selfTest = (function () {

    var test_term = function test_term() {
        var expr;
        var expect;
        var actual;

        // times(num[2];plus(num[3];x))
        expr = term.factory(VO.fromNative(
            { "kind":"operator", "sort":"Exp", "name":"times", "arguments":[
                { "kind":"operator", "sort":"Exp", "name":"num", "index":2 },
                { "kind":"operator", "sort":"Exp", "name":"plus", "arguments":[
                    { "kind":"operator", "sort":"Exp", "name":"num", "index":3 },
                    { "kind":"variable", "sort":"Exp", "name":"x" }
                ]}
            ]}
        ));
        term.log('expr:', expr);
        actual = expr.FV();
        expect = VO.fromNative({"x":"Exp"});
        VO.ensure(actual.equals(expect));
    };

    return function selfTest() {
        VO.selfTest();  // test imported package first
        test_term();
        return true;  // all tests passed
    };

})();
