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

//term.log = console.log;
term.log = function () {};

var s_kind = VO.String("kind");
var s_name = VO.String("name");

term.kind = {};  // term compiler namespace

term.factory = function compile_term(source) {  // { "kind": <string>, ... }
    VO.ensure(source.hasType(VO.Object));
    VO.ensure(source.hasProperty(s_kind));
    var compiler = term.kind[source.value(s_kind)._value];
    var value = compiler(source);
    return value;
};

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
    proto = proto || term.Term;
    proto.FV = function FV() {  // free variables
        let fv = this._args.reduce(function (v, x) {
            VO.ensure(v.hasType(term.Term));
            return x.concatenate(v.FV());
        }, VO.emptyObject);
        return fv;
    };
    var constructor = function Operator(sort, name, args, index) {
        if (!(this instanceof Operator)) { return new Operator(sort, name, args); }  // if called without "new" keyword...
        VO.ensure(sort.hasType(VO.String));
        VO.ensure(name.hasType(VO.String));
        VO.ensure(args.hasType(VO.Array));
        this._sort = sort;
        this._name = name;
        this._args = args;
        if (index !== undefined) {
            VO.ensure(args.hasType(VO.Value));
            this._index = index;
        }
    };
    proto.constructor = constructor;
    constructor.prototype = proto;
    return constructor;
})();

term.Variable = (function (proto) {
    proto = proto || term.Term;
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
    proto = proto || term.Term;
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

term.Grammar = (function (proto) {
    proto = proto || VO.Value();
    proto.compile = function compile(ast) {  // { "kind": <string>, ... }
        VO.ensure(ast.hasType(VO.Object));
        VO.ensure(ast.hasProperty(s_kind));
        VO.ensure(ast.value(s_kind).hasType(VO.String));
        var _kind = ast.value(s_kind)._value;
        var factory = term.kind[_kind];
        var expr = factory(ast, this);
        return expr;
    };
    proto.compile_rule = function compile_rule(name, ast) {  // { "kind": <string>, ... }
        VO.ensure(name.hasType(VO.String));
        var expr = this.compile(ast);
        this._rules[name._value] = expr;
        term.log('rule:', name._value, ':=', expr);
        return this;  // return "updated" grammar
    };
    proto.rule = function rule(name) {  // get named rule
        VO.ensure(name.hasType(VO.String));
        var expr = this._rules[name._value];
        term.log('rule:', name._value, '->', expr);
        return expr;
    };
    var constructor = function Grammar(rules) {
        if (!(this instanceof Grammar)) { return new Grammar(rules); }  // if called without "new" keyword...
        this._rules = rules || {};
    };
    proto.constructor = constructor;
    constructor.prototype = proto;
    return constructor;
})();

term.Pattern = (function (proto) {
    proto = proto || VO.Value();
    proto.match = function match(input) {  // { value:, remainder: } | false
        VO.ensure(input.hasType(VO.String).or(input.hasType(VO.Array)));
        VO.throw("Not Implemented");  // FIXME!
    };
    var constructor = function Pattern() {
        if (!(this instanceof Pattern)) { return new Pattern(); }  // if called without "new" keyword...
    };
    proto.constructor = constructor;
    constructor.prototype = proto;
    return constructor;
})();

term.kind["fail"] = (function (factory) {
    factory = factory || function term_fail(ast, grammar) {  // { "kind": "fail" }
        VO.ensure(ast.hasType(VO.Object));
        VO.ensure(ast.hasProperty(s_kind));
        VO.ensure(ast.value(s_kind).equals(VO.String("fail")));
        return term.Fail();
    };
    return factory;
})();

term.Fail = (function (proto) {
    proto = proto || term.Pattern();
    proto.match = function match(input) {  // { value:, remainder: } | false
        VO.ensure(input.hasType(VO.String).or(input.hasType(VO.Array)));
        return VO.false;  // match failure
    };
    var constructor = function Fail() {
        if (instance) { return instance; }  // return cached singleton
        if (!(this instanceof Fail)) { return new Fail(); }  // if called without "new" keyword...
    };
    proto.constructor = constructor;
    constructor.prototype = proto;
    var instance = new constructor();  // cache singleton instance
    return constructor;
})();

term.kind["terminal"] = (function (factory) {
    factory = factory || function term_terminal(ast, grammar) {  // { "kind": "terminal", "value": <value> }
        VO.ensure(ast.hasType(VO.Object));
        VO.ensure(ast.hasProperty(s_kind));
        VO.ensure(ast.value(s_kind).equals(VO.String("terminal")));
        VO.ensure(ast.hasProperty(s_value));
        return term.Terminal(ast.value(s_value));
    };
    return factory;
})();

term.Terminal = (function (proto) {
    proto = proto || term.Pattern();
    proto.match = function match(input) {  // { value:, remainder: } | false
        VO.ensure(input.hasType(VO.String).or(input.hasType(VO.Array)));
        if (input.length.greaterThan(VO.zero) === VO.true) {
            var value = input.value(VO.zero);
            if (this._value.equals(value) === VO.true) {
                return (VO.emptyObject  // match success
                    .concatenate(s_value.bind(value))
                    .concatenate(s_remainder.bind(input.skip(VO.one))));
            }
        }
        return VO.false;  // match failure
    };
    var constructor = function Terminal(value) {
        if (!(this instanceof Terminal)) { return new Terminal(value); }  // if called without "new" keyword...
        this._value = value;
    };
    proto.constructor = constructor;
    constructor.prototype = proto;
    return constructor;
})();

term.kind["alternative"] = (function (factory) {
    factory = factory || function term_alternative(ast, grammar) {  // { "kind": "alternative", "of": <array> }
        VO.ensure(ast.hasType(VO.Object));
        VO.ensure(ast.hasProperty(s_kind));
        VO.ensure(ast.value(s_kind).equals(VO.String("alternative")));
        VO.ensure(ast.hasProperty(VO.String("of")));
        VO.ensure(ast.value(VO.String("of")).hasType(VO.Array));
        var _of = ast.value(VO.String("of")).reduce(function (v, x) {
            var expr = grammar.compile(v);  // compile expression
            return x.append(expr);
        }, VO.emptyArray);
        return term.Alternative(_of);
    };
    return factory;
})();

term.Alternative = (function (proto) {
    proto = proto || term.Pattern();
    proto.match = function match(input) {  // { value:, remainder: } | false
        VO.ensure(input.hasType(VO.String).or(input.hasType(VO.Array)));
        var match = this._of.reduce(function (v, x) {
            if (x !== VO.false) {
                return x;  // propagate success
            }
            return v.match(input);
        }, VO.false);  // default match failure
        return match;
    };
    var constructor = function Alternative(_of) {
        if (!(this instanceof Alternative)) { return new Alternative(_of); }  // if called without "new" keyword...
        VO.ensure(_of.hasType(VO.Array));
        this._of = _of;
    };
    proto.constructor = constructor;
    constructor.prototype = proto;
    return constructor;
})();

var match_repeat = function match_repeat(input, expr, min, max) {
    var count = 0;
    var match = (VO.emptyObject  // match success (default)
        .concatenate(s_value.bind(VO.emptyArray))
        .concatenate(s_remainder.bind(input)));
    while ((max === undefined) || (count < max)) {
        var _match = expr.match(match.value(s_remainder));  // match at current position
        if (_match === VO.false) {
            break;  // match failure
        }
        count += 1;  // update match count
        var _value = match.value(s_value).append(_match.value(s_value));  // update value
        match = _match.concatenate(s_value.bind(_value));  // update successful match
    }
    if (count < min) {
        match = VO.false;  // match failure
    }
    return match;
};

term.kind["star"] = (function (factory) {
    factory = factory || function term_star(ast, grammar) {  // { "kind": "star", "expr": <object> }
        VO.ensure(ast.hasType(VO.Object));
        VO.ensure(ast.hasProperty(s_kind));
        VO.ensure(ast.value(s_kind).equals(VO.String("star")));
        VO.ensure(ast.hasProperty(VO.String("expr")));
        VO.ensure(ast.value(VO.String("expr")).hasType(VO.Object));
        var expr = grammar.compile(ast.value(VO.String("expr")));  // compile expression
        return term.Star(expr);
    };
    return factory;
})();

term.Star = (function (proto) {  // star(expr) = alternative(sequence(expr, star(expr)), nothing)
    proto = proto || term.Pattern();
    proto.match = function match(input) {  // { value:, remainder: } | false
        VO.ensure(input.hasType(VO.String).or(input.hasType(VO.Array)));
        return match_repeat(input, this._expr, 0);
    };
    var constructor = function Star(expr) {
        if (!(this instanceof Star)) { return new Star(expr); }  // if called without "new" keyword...
        VO.ensure(expr.hasType(term.Pattern));
        this._expr = expr;
    };
    proto.constructor = constructor;
    constructor.prototype = proto;
    return constructor;
})();

term.selfTest = (function () {

    var test_term = function test_term() {
        var input;
        var match;

        var grammar = term.factory(VO.fromNative({ //"lang": "term", "ast": {
            "kind": "grammar",
            "rules": {
                "number": {
                    "kind": "sequence",
                    "of": [
                        {
                            "kind": "optional",
                            "expr": { "kind": "rule", "name": "minus" }
                        },
                        { "kind": "rule", "name": "integer" },
                        {
                            "kind": "optional",
                            "expr": { "kind": "rule", "name": "frac" }
                        },
                        {
                            "kind": "optional",
                            "expr": { "kind": "rule", "name": "exp" }
                        }
                    ]
                },
                "integer": {
                    "kind": "alternative",
                    "of": [
                        { "kind": "rule", "name": "digit0" },
                        {
                            "kind": "sequence",
                            "of": [
                                { "kind": "rule", "name": "digit1-9" },
                                {
                                    "kind": "star",
                                    "expr": { "kind": "rule", "name": "digit0-9" }
                                }
                            ]
                        }
                    ]
                },
                "frac": {
                    "kind": "sequence",
                    "of": [
                        { "kind": "rule", "name": "decimal-point" },
                        {
                            "kind": "plus",
                            "expr": { "kind": "rule", "name": "digit0-9" }
                        }
                    ]
                },
                "exp": {
                    "kind": "sequence",
                    "of": [
                        { "kind": "rule", "name": "e" },
                        {
                            "kind": "optional",
                            "expr": {
                                "kind": "alternative",
                                "of": [
                                    { "kind": "rule", "name": "minus" },
                                    { "kind": "rule", "name": "plus" }
                                ]
                            }
                        },
                        {
                            "kind": "plus",
                            "expr": { "kind": "rule", "name": "digit0-9" }
                        }
                    ]
                },
                "e": {
                    "kind": "alternative",
                    "of": [
                        { "kind": "terminal", "value": 101 },
                        { "kind": "terminal", "value": 69 }
                    ]
                },
                "decimal-point": { "kind": "terminal", "value": 46 },
                "plus": { "kind": "terminal", "value": 43 },
                "minus": { "kind": "terminal", "value": 45 },
                "digit0": { "kind": "terminal", "value": 48 },
                "digit1-9": { "kind": "range", "from": 49, "to": 57 },
                "digit0-9": { "kind": "range", "from": 48, "to": 57 },
                "anything": { "kind": "anything" },
                "nothing": { "kind": "nothing" },
                "fail": { "kind": "fail" }
            }//}
        }));
        term.log('grammar:', grammar);

        match = grammar.rule(VO.String("digit0")).match(VO.emptyString);
        VO.ensure(match.equals(VO.false));

        input = VO.fromNative("0\n");
        match = grammar.rule(VO.String("fail")).match(input);
        VO.ensure(match.equals(VO.false));

        match = grammar.rule(VO.String("digit0")).match(input);
        VO.ensure(match.hasType(VO.Object));
        VO.ensure(match.value(s_value).equals(VO.Number(48)));  // "0"
        VO.ensure(match.value(s_remainder).length.equals(VO.Number(1)));

        match = grammar.rule(VO.String("number")).match(VO.String("3.14e+0"));
        VO.ensure(match.equals(VO.false).not);
        VO.ensure(match.value(s_value)
                  .equals(VO.fromNative([
                        [], 
                        [51, []], 
                        [[46, [49, 52]]], 
                        [[101, [43], [48]]]
                    ])));
        VO.ensure(match.value(s_remainder).length.equals(VO.zero));
    };

    return function selfTest() {
        VO.selfTest();  // test imported package first
        test_term();
        return true;  // all tests passed
    };

})();
