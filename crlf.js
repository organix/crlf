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
    var factory = crlf.language[name];  // find compiler for specified language
    var ast = source.value(VO.String("ast"));
    crlf.log('language:', name, '->', factory, ast);
    return factory(ast);  // compile crlf language ast
};

crlf.language = {};  // language factory namespace

crlf.language["PEG"] = (function (constructor) {
    var kind = {};
    var factory = function compile_PEG(ast) {  // { "kind": <string>, ... }
        return new constructor(ast);  // delegate to top-level constructor
    };
    var compile_expr = function compile_expr(ast, grammar) {  // { "kind": <string>, ... }
        VO.ensure(ast.hasType(VO.Object));
        VO.ensure(ast.hasProperty(new VO.String("kind")));
        VO.ensure(ast.value(new VO.String("kind")).hasType(VO.String));
        var constructor = kind[ast.value(new VO.String("kind"))._value];
        return new constructor(ast, grammar);
    };
    constructor = constructor || function PEG_grammar(ast) {  // { "kind": "grammar", "rules": <object> }
        var grammar = this;  // capture this
        VO.ensure(ast.hasType(VO.Object));
        VO.ensure(ast.hasProperty(new VO.String("kind")));
        VO.ensure(ast.value(new VO.String("kind")).equals(new VO.String("grammar")));
        VO.ensure(ast.hasProperty(new VO.String("rules")));
        VO.ensure(ast.value(new VO.String("rules")).hasType(VO.Object));
        this._ast = ast;
        var rules = {};
        ast.value(new VO.String("rules")).reduce(function (n, v, x) {
            rules[n._value] = compile_expr(v, grammar);  // compile rules
            return x;
        }, VO.null);
        this._rules = rules;
    };
    var prototype = constructor.prototype;
    prototype.constructor = constructor;
    prototype.rule = function rule(name) {  // get named rule
        VO.ensure(name.hasType(VO.String));
        var _name = name._value;
        var _rule = this._rules[_name];
        crlf.log('rule:', _name, '->', _rule);
        return _rule;
    };
    kind["fail"] = (function (constructor) {
        constructor = constructor || function PEG_fail(ast, g) {  // { "kind": "fail" }
            VO.ensure(ast.hasType(VO.Object));
            VO.ensure(ast.hasProperty(new VO.String("kind")));
            VO.ensure(ast.value(new VO.String("kind")).equals(new VO.String("fail")));
            this._ast = ast;
        };
        var prototype = constructor.prototype;
        prototype.constructor = constructor;
        prototype.match = function match_fail(input) {
            VO.ensure(input.hasType(VO.String).or(input.hasType(VO.Array)));
            return VO.false;  // match failure
        };
        return constructor;
    })();
    kind["nothing"] = (function (constructor) {
        constructor = constructor || function PEG_nothing(ast, g) {  // { "kind": "nothing" }
            VO.ensure(ast.hasType(VO.Object));
            VO.ensure(ast.hasProperty(new VO.String("kind")));
            VO.ensure(ast.value(new VO.String("kind")).equals(new VO.String("nothing")));
            this._ast = ast;
        };
        var prototype = constructor.prototype;
        prototype.constructor = constructor;
        prototype.match = function match_nothing(input) {
            VO.ensure(input.hasType(VO.String).or(input.hasType(VO.Array)));
            return (VO.emptyObject  // match success
                .concatenate((new VO.String("value")).bind(VO.emptyArray))
                .concatenate((new VO.String("remainder")).bind(input)));
        };
        return constructor;
    })();
    kind["anything"] = (function (constructor) {
        constructor = constructor || function PEG_anything(ast, g) {  // { "kind": "anything" }
            VO.ensure(ast.hasType(VO.Object));
            VO.ensure(ast.hasProperty(new VO.String("kind")));
            VO.ensure(ast.value(new VO.String("kind")).equals(new VO.String("anything")));
            this._ast = ast;
        };
        var prototype = constructor.prototype;
        prototype.constructor = constructor;
        prototype.match = function match_anything(input) {
            VO.ensure(input.hasType(VO.String).or(input.hasType(VO.Array)));
            if (input.length.greaterThan(VO.zero) === VO.true) {
                return (VO.emptyObject  // match success
                    .concatenate((new VO.String("value")).bind(input.value(VO.zero)))
                    .concatenate((new VO.String("remainder")).bind(input.skip(VO.one))));
            }
            return VO.false;  // match failure
        };
        return constructor;
    })();
    kind["terminal"] = (function (constructor) {
        constructor = constructor || function PEG_terminal(ast, g) {  // { "kind": "terminal", "value": <value> }
            VO.ensure(ast.hasType(VO.Object));
            VO.ensure(ast.hasProperty(new VO.String("kind")));
            VO.ensure(ast.value(new VO.String("kind")).equals(new VO.String("terminal")));
            VO.ensure(ast.hasProperty(new VO.String("value")));
            this._ast = ast;
        };
        var prototype = constructor.prototype;
        prototype.constructor = constructor;
        prototype.match = function match_terminal(input) {
            VO.ensure(input.hasType(VO.String).or(input.hasType(VO.Array)));
            if (input.length.greaterThan(VO.zero) === VO.true) {
                var value = input.value(VO.zero);
                if (value.equals(this._ast.value(new VO.String("value"))) === VO.true) {
                    return (VO.emptyObject  // match success
                        .concatenate((new VO.String("value")).bind(value))
                        .concatenate((new VO.String("remainder")).bind(input.skip(VO.one))));
                }
            }
            return VO.false;  // match failure
        };
        return constructor;
    })();
    kind["range"] = (function (constructor) {
        constructor = constructor || function PEG_range(ast, g) {  // { "kind": "range", "from": <number>, "to": <number> }
            VO.ensure(ast.hasType(VO.Object));
            VO.ensure(ast.hasProperty(new VO.String("kind")));
            VO.ensure(ast.value(new VO.String("kind")).equals(new VO.String("range")));
            VO.ensure(ast.hasProperty(new VO.String("from")));
            VO.ensure(ast.value(new VO.String("from")).hasType(VO.Number));
            VO.ensure(ast.hasProperty(new VO.String("to")));
            VO.ensure(ast.value(new VO.String("to")).hasType(VO.Number));
            VO.ensure(ast.value(new VO.String("from")).lessEqual(ast.value(new VO.String("to"))));
            this._ast = ast;
        };
        var prototype = constructor.prototype;
        prototype.constructor = constructor;
        prototype.match = function match_range(input) {
            VO.ensure(input.hasType(VO.String).or(input.hasType(VO.Array)));
            if (input.length.greaterThan(VO.zero) === VO.true) {
                var value = input.value(VO.zero);
                if (value.greaterEqual(this._ast.value(new VO.String("from")))
                .and(value.lessEqual(this._ast.value(new VO.String("to")))) === VO.true) {
                    return (VO.emptyObject  // match success
                        .concatenate((new VO.String("value")).bind(value))
                        .concatenate((new VO.String("remainder")).bind(input.skip(VO.one))));
                }
            }
            return VO.false;  // match failure
        };
        return constructor;
    })();
    kind["rule"] = (function (constructor) {  // apply named rule to input
        constructor = constructor || function PEG_rule(ast, g) {  // { "kind": "rule", "name": <string> }
            VO.ensure(ast.hasType(VO.Object));
            VO.ensure(ast.hasProperty(new VO.String("kind")));
            VO.ensure(ast.value(new VO.String("kind")).equals(new VO.String("rule")));
            VO.ensure(ast.hasProperty(new VO.String("name")));
            VO.ensure(ast.value(new VO.String("name")).hasType(VO.String));
            this._ast = ast;
            this._grammar = g;
        };
        var prototype = constructor.prototype;
        prototype.constructor = constructor;
        prototype.match = function match_rule(input) {
            VO.ensure(input.hasType(VO.String).or(input.hasType(VO.Array)));
            var name = this._ast.value(new VO.String("name"));
            var rule = this._grammar.rule(name);
            return rule.match(input);
        };
        return constructor;
    })();
    kind["sequence"] = (function (constructor) {
        constructor = constructor || function PEG_sequence(ast, g) {  // { "kind": "sequence", "of": <array> }
            VO.ensure(ast.hasType(VO.Object));
            VO.ensure(ast.hasProperty(new VO.String("kind")));
            VO.ensure(ast.value(new VO.String("kind")).equals(new VO.String("sequence")));
            VO.ensure(ast.hasProperty(new VO.String("of")));
            VO.ensure(ast.value(new VO.String("of")).hasType(VO.Array));
            this._ast = ast;
            var rules = [];
            ast.value(new VO.String("of")).reduce(function (v, x) {
                rules.push(compile_expr(v, g));  // compile rules
                return x;
            }, VO.null);
            this._of = rules;
        };
        var prototype = constructor.prototype;
        prototype.constructor = constructor;
        prototype.match = function match_sequence(input) {
            VO.ensure(input.hasType(VO.String).or(input.hasType(VO.Array)));
            var match = (VO.emptyObject  // match success (default)
                .concatenate((new VO.String("value")).bind(VO.emptyArray))
                .concatenate((new VO.String("remainder")).bind(input)));
            for (var i = 0; i < this._of.length; ++i) {
                var rule = this._of[i];
                var _match = rule.match(input);
                if (_match === VO.false) {
                    return VO.false;  // match failure
                }
                input = _match.value(new VO.String("remainder"));  // update input position
                let s_value = new VO.String("value");
                match = (VO.emptyObject  // update successful match
                    .concatenate(s_value.bind(
                            match.value(s_value).append(_match.value(s_value))))
                    .concatenate((new VO.String("remainder")).bind(input)));
            }
            return match;
        };
        return constructor;
    })();
    kind["alternative"] = (function (constructor) {
        constructor = constructor || function PEG_alternative(ast, g) {  // { "kind": "alternative", "of": <array> }
            VO.ensure(ast.hasType(VO.Object));
            VO.ensure(ast.hasProperty(new VO.String("kind")));
            VO.ensure(ast.value(new VO.String("kind")).equals(new VO.String("alternative")));
            VO.ensure(ast.hasProperty(new VO.String("of")));
            VO.ensure(ast.value(new VO.String("of")).hasType(VO.Array));
            this._ast = ast;
            var rules = [];
            ast.value(new VO.String("of")).reduce(function (v, x) {
                rules.push(compile_expr(v, g));  // compile rules
                return x;
            }, VO.null);
            this._of = rules;
        };
        var prototype = constructor.prototype;
        prototype.constructor = constructor;
        prototype.match = function match_alternative(input) {
            VO.ensure(input.hasType(VO.String).or(input.hasType(VO.Array)));
            for (var i = 0; i < this._of.length; ++i) {
                var rule = this._of[i];
                var match = rule.match(input);
                if (match !== VO.false) {
                    return match;  // match success
                }
            }
            return VO.false;  // match failure
        };
        return constructor;
    })();
    var match_repeat = function match_repeat(input, expr, min, max) {
        var count = 0;
        var match = (VO.emptyObject  // match success (default)
            .concatenate((new VO.String("value")).bind(VO.emptyArray))
            .concatenate((new VO.String("remainder")).bind(input)));
        while ((max === undefined) || (count < max)) {
            var _match = expr.match(input);
            if (_match === VO.false) {
                break;  // match failure
            }
            count += 1;  // update match count
            input = _match.value(new VO.String("remainder"));  // update input position
            let s_value = new VO.String("value");
            match = (VO.emptyObject  // update successful match
                .concatenate(s_value.bind(
                        match.value(s_value).append(_match.value(s_value))))
                .concatenate((new VO.String("remainder")).bind(input)));
        }
        if (count < min) {
            match = VO.false;  // match failure
        }
        return match;
    };
    kind["star"] = (function (constructor) {  // star(expr) = alternative(sequence(expr, star(expr)), nothing)
        constructor = constructor || function PEG_star(ast, g) {  // { "kind": "star", "expr": <object> }
            VO.ensure(ast.hasType(VO.Object));
            VO.ensure(ast.hasProperty(new VO.String("kind")));
            VO.ensure(ast.value(new VO.String("kind")).equals(new VO.String("star")));
            VO.ensure(ast.hasProperty(new VO.String("expr")));
            VO.ensure(ast.value(new VO.String("expr")).hasType(VO.Object));
            this._ast = ast;
            this._expr = compile_expr(ast.value(new VO.String("expr")), g);  // compile expression
        };
        var prototype = constructor.prototype;
        prototype.constructor = constructor;
        prototype.match = function match_star(input) {
            VO.ensure(input.hasType(VO.String).or(input.hasType(VO.Array)));
            return match_repeat(input, this._expr, 0);
        };
        return constructor;
    })();
    kind["plus"] = (function (constructor) {  // plus(expr) = sequence(expr, star(expr))
        constructor = constructor || function PEG_plus(ast, g) {  // { "kind": "plus", "expr": <object> }
            VO.ensure(ast.hasType(VO.Object));
            VO.ensure(ast.hasProperty(new VO.String("kind")));
            VO.ensure(ast.value(new VO.String("kind")).equals(new VO.String("plus")));
            VO.ensure(ast.hasProperty(new VO.String("expr")));
            VO.ensure(ast.value(new VO.String("expr")).hasType(VO.Object));
            this._ast = ast;
            this._expr = compile_expr(ast.value(new VO.String("expr")), g);  // compile expression
        };
        var prototype = constructor.prototype;
        prototype.constructor = constructor;
        prototype.match = function match_plus(input) {
            VO.ensure(input.hasType(VO.String).or(input.hasType(VO.Array)));
            return match_repeat(input, this._expr, 1);
        };
        return constructor;
    })();
    kind["optional"] = (function (constructor) {  // optional(expr) = alternative(expr, nothing)
        constructor = constructor || function PEG_optional(ast, g) {  // { "kind": "optional", "expr": <object> }
            VO.ensure(ast.hasType(VO.Object));
            VO.ensure(ast.hasProperty(new VO.String("kind")));
            VO.ensure(ast.value(new VO.String("kind")).equals(new VO.String("optional")));
            VO.ensure(ast.hasProperty(new VO.String("expr")));
            VO.ensure(ast.value(new VO.String("expr")).hasType(VO.Object));
            this._ast = ast;
            this._expr = compile_expr(ast.value(new VO.String("expr")), g);  // compile expression
        };
        var prototype = constructor.prototype;
        prototype.constructor = constructor;
        prototype.match = function match_optional(input) {
            VO.ensure(input.hasType(VO.String).or(input.hasType(VO.Array)));
            return match_repeat(input, this._expr, 0, 1);
        };
        return constructor;
    })();
    return factory;
})();

crlf.language["VO"] = (function (constructor) {
    var kind = {};
    var factory = function compile_VO(ast) {  // { "kind": <string>, ... }
        if (ast.hasType(VO.Array) === VO.true) {
            return ast.reduce(function (v, x) {
                return x.append(factory(v));  // compile expressions
            }, VO.emptyArray);
        }
        VO.ensure(ast.hasType(VO.Object));
        VO.ensure(ast.hasProperty(new VO.String("kind")));
        VO.ensure(ast.value(new VO.String("kind")).hasType(VO.String));
        var constructor = kind[ast.value(new VO.String("kind"))._value];
        return new constructor(ast);
    };
//    constructor = constructor || function VO_expression(ast) {
//        return compile(ast);
//    };
//    var prototype = constructor.prototype;
//    prototype.constructor = constructor;
    kind["value"] = (function (constructor) {
        constructor = constructor || function VO_value(ast) {  // { "kind": "value", "value": <value> }
            VO.ensure(ast.hasType(VO.Object));
            VO.ensure(ast.hasProperty(new VO.String("kind")));
            VO.ensure(ast.value(new VO.String("kind")).equals(new VO.String("value")));
            VO.ensure(ast.hasProperty(new VO.String("value")));
            VO.ensure(ast.value(new VO.String("value")).hasType(VO.Value));
            return new VO.ValueExpr(ast.value(new VO.String("value")));
        };
        return constructor;
    })();
    kind["variable"] = (function (constructor) {
        constructor = constructor || function VO_variable(ast) {  // { "kind": "variable", "name": <string> }
            VO.ensure(ast.hasType(VO.Object));
            VO.ensure(ast.hasProperty(new VO.String("kind")));
            VO.ensure(ast.value(new VO.String("kind")).equals(new VO.String("variable")));
            VO.ensure(ast.hasProperty(new VO.String("name")));
            VO.ensure(ast.value(new VO.String("name")).hasType(VO.String));
            return new VO.VariableExpr(ast.value(new VO.String("name")));
        };
        return constructor;
    })();
    kind["combination"] = (function (constructor) {
        constructor = constructor || function VO_combination(ast) {  // { "kind": "combination", "operative": <expression>, "parameter": <value> }
            VO.ensure(ast.hasType(VO.Object));
            VO.ensure(ast.hasProperty(new VO.String("kind")));
            VO.ensure(ast.value(new VO.String("kind")).equals(new VO.String("combination")));
            VO.ensure(ast.hasProperty(new VO.String("operative")));
            var operative = factory(ast.value(new VO.String("operative")));
            VO.ensure(ast.hasProperty(new VO.String("parameter")));
            var parameter = factory(ast.value(new VO.String("parameter")));
            return new VO.CombineExpr(operative, parameter);
        };
        return constructor;
    })();
    kind["method"] = (function (constructor) {
        constructor = constructor || function VO_method(ast) {  // { "kind": "method", "target": <expression>, "selector": <expression> }
            VO.ensure(ast.hasType(VO.Object));
            VO.ensure(ast.hasProperty(new VO.String("kind")));
            VO.ensure(ast.value(new VO.String("kind")).equals(new VO.String("method")));
            VO.ensure(ast.hasProperty(new VO.String("target")));
            var target = factory(ast.value(new VO.String("target")));
            VO.ensure(ast.hasProperty(new VO.String("selector")));
            var selector = factory(ast.value(new VO.String("selector")));
            return new VO.MethodExpr(target, selector);
        };
        return constructor;
    })();
    return factory;
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
    let factory = function compile_proof(ast) {  // { "kind":<string>, ... }
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
                return x.append(factory(v));  // compile abt's
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
                this._scope = factory(ast.value(s_scope));
            }
        };
        prototype.constructor = constructor;
        constructor.prototype = prototype;
        return constructor;
    })(new VO.Value());
    return factory;
})();

crlf.selfTest = (function () {

    var test_PEG = function test_PEG() {
        var input;
        var match;

        var grammar = crlf.factory(VO.fromNative({
            "lang": "PEG",
            "ast": {
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
                }
            }
        }));

        match = grammar.rule(new VO.String("nothing")).match(VO.emptyString);
        VO.ensure(match.hasType(VO.Object));
        VO.ensure(match.value(new VO.String("remainder")).equals(VO.emptyString));

        match = grammar.rule(new VO.String("nothing")).match(VO.emptyArray);
        VO.ensure(match.hasType(VO.Object));
        VO.ensure(match.value(new VO.String("remainder")).equals(VO.emptyArray));

        match = grammar.rule(new VO.String("anything")).match(VO.emptyArray);
        VO.ensure(match.equals(VO.false));

        match = grammar.rule(new VO.String("digit0")).match(VO.emptyString);
        VO.ensure(match.equals(VO.false));

        input = VO.fromNative("0\n");
        match = grammar.rule(new VO.String("fail")).match(input);
        VO.ensure(match.equals(VO.false));

        match = grammar.rule(new VO.String("digit0")).match(input);
        VO.ensure(match.hasType(VO.Object));
        VO.ensure(match.value(new VO.String("value")).equals(new VO.Number(48)));  // "0"
        VO.ensure(match.value(new VO.String("remainder")).length.equals(new VO.Number(1)));

        match = grammar.rule(new VO.String("digit0-9")).match(input);
        VO.ensure(match.hasType(VO.Object));
        VO.ensure(match.value(new VO.String("value")).equals(new VO.Number(48)));  // "0"
        VO.ensure(match.value(new VO.String("remainder")).length.equals(new VO.Number(1)));

        match = grammar.rule(new VO.String("digit1-9")).match(input);
        VO.ensure(match.equals(VO.false));

        input = VO.fromNative("\r\n");
        match = grammar.rule(new VO.String("anything")).match(input);
        VO.ensure(match.hasType(VO.Object));
        VO.ensure(match.value(new VO.String("value")).equals(new VO.Number(13)));  // "\r"
        VO.ensure(match.value(new VO.String("remainder")).length.equals(new VO.Number(1)));

        match = grammar.rule(new VO.String("integer")).match(new VO.String("0"));
        VO.ensure(match.equals(VO.false).not);
        VO.ensure(match.value(new VO.String("value")).equals(VO.fromNative(48)));
        VO.ensure(match.value(new VO.String("remainder")).length.equals(VO.zero));

        match = grammar.rule(new VO.String("integer")).match(new VO.String("00"));
        VO.ensure(match.equals(VO.false).not);
        VO.ensure(match.value(new VO.String("value")).equals(VO.fromNative(48)));
        VO.ensure(match.value(new VO.String("remainder")).equals(VO.fromNative("0")));

        match = grammar.rule(new VO.String("integer")).match(new VO.String("1"));
        VO.ensure(match.equals(VO.false).not);
        VO.ensure(match.value(new VO.String("value")).equals(VO.fromNative([49, []])));
        VO.ensure(match.value(new VO.String("remainder")).length.equals(VO.zero));

        match = grammar.rule(new VO.String("integer")).match(new VO.String("12"));
        VO.ensure(match.equals(VO.false).not);
        VO.ensure(match.value(new VO.String("value")).equals(VO.fromNative([49, [50]])));
        VO.ensure(match.value(new VO.String("remainder")).length.equals(VO.zero));

        match = grammar.rule(new VO.String("integer")).match(new VO.String("123"));
        VO.ensure(match.equals(VO.false).not);
        VO.ensure(match.value(new VO.String("value")).equals(VO.fromNative([49, [50, 51]])));
        VO.ensure(match.value(new VO.String("remainder")).length.equals(VO.zero));

        match = grammar.rule(new VO.String("integer")).match(new VO.String("3.14e+0"));
        VO.ensure(match.equals(VO.false).not);
        VO.ensure(match.value(new VO.String("value")).equals(VO.fromNative([51, []])));
        VO.ensure(match.value(new VO.String("remainder")).equals(VO.fromNative(".14e+0")));

        match = grammar.rule(new VO.String("number")).match(new VO.String("3.14e+0"));
        VO.ensure(match.equals(VO.false).not);
        VO.ensure(match.value(new VO.String("value"))
                  .equals(VO.fromNative([
                        [], 
                        [51, []], 
                        [[46, [49, 52]]], 
                        [[101, [43], [48]]]
                    ])));
        VO.ensure(match.value(new VO.String("remainder")).length.equals(VO.zero));
    };

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
        VO.selfTest();  // test imported package first
        test_PEG();
        test_VO();
        test_proof();
        return true;  // all tests passed
    };

})();
