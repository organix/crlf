/*

PEG.js - "crlf/PEG" Parsing Expression Grammars

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

var PEG = module.exports;
PEG.version = "0.0.1";

PEG.log = console.log;
//PEG.log = function () {};

var s_kind = VO.String("kind");
var s_value = VO.String("value");
var s_remainder = VO.String("remainder");

PEG.kind = {};  // matching-operator factory namespace

PEG.factory = function compile_PEG(source) {  // { "kind": "grammar", "rules": <object> }
    VO.ensure(source.hasType(VO.Object));
    VO.ensure(source.hasProperty(s_kind));
    VO.ensure(source.value(s_kind).equals(VO.String("grammar")));
    VO.ensure(source.hasProperty(VO.String("rules")));
    VO.ensure(source.value(VO.String("rules")).hasType(VO.Object));
    var grammar = source.value(VO.String("rules")).reduce(function (n, v, x) {
        return x.compile_rule(n, v);
    }, PEG.Grammar());
    return grammar;
};

PEG.Grammar = (function (proto) {
    proto = proto || VO.Value();
    proto.compile = function compile(ast) {  // { "kind": <string>, ... }
        VO.ensure(ast.hasType(VO.Object));
        VO.ensure(ast.hasProperty(s_kind));
        VO.ensure(ast.value(s_kind).hasType(VO.String));
        var _kind = ast.value(s_kind)._value;
        var factory = PEG.kind[_kind];
        var expr = factory(ast, this);
        return expr;
    };
    proto.compile_rule = function compile_rule(name, ast) {  // { "kind": <string>, ... }
        VO.ensure(name.hasType(VO.String));
        var expr = this.compile(ast);
        this._rules[name._value] = expr;
        PEG.log('rule:', name._value, ':=', expr);
        return this;  // return "updated" grammar
    };
    proto.rule = function rule(name) {  // get named rule
        VO.ensure(name.hasType(VO.String));
        var expr = this._rules[name._value];
        PEG.log('rule:', name._value, '->', expr);
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

PEG.Pattern = (function (proto) {
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

PEG.kind["fail"] = (function (factory) {
    factory = factory || function PEG_fail(ast, grammar) {  // { "kind": "fail" }
        VO.ensure(ast.hasType(VO.Object));
        VO.ensure(ast.hasProperty(s_kind));
        VO.ensure(ast.value(s_kind).equals(VO.String("fail")));
        return PEG.Fail();
    };
    return factory;
})();

PEG.Fail = (function (proto) {
    proto = proto || PEG.Pattern();
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

PEG.kind["nothing"] = (function (factory) {
    factory = factory || function PEG_nothing(ast, grammar) {  // { "kind": "nothing" }
        VO.ensure(ast.hasType(VO.Object));
        VO.ensure(ast.hasProperty(s_kind));
        VO.ensure(ast.value(s_kind).equals(VO.String("nothing")));
        return PEG.Nothing();
    };
    return factory;
})();

PEG.Nothing = (function (proto) {
    proto = proto || PEG.Pattern();
    proto.match = function match(input) {  // { value:, remainder: } | false
        VO.ensure(input.hasType(VO.String).or(input.hasType(VO.Array)));
        return (VO.emptyObject  // match success
            .concatenate(s_value.bind(VO.emptyArray))
            .concatenate(s_remainder.bind(input)));
    };
    var constructor = function Nothing() {
        if (instance) { return instance; }  // return cached singleton
        if (!(this instanceof Nothing)) { return new Nothing(); }  // if called without "new" keyword...
    };
    proto.constructor = constructor;
    constructor.prototype = proto;
    var instance = new constructor();  // cache singleton instance
    return constructor;
})();

PEG.kind["anything"] = (function (factory) {
    factory = factory || function PEG_anything(ast, grammar) {  // { "kind": "anything" }
        VO.ensure(ast.hasType(VO.Object));
        VO.ensure(ast.hasProperty(s_kind));
        VO.ensure(ast.value(s_kind).equals(VO.String("anything")));
        return PEG.Anything();
    };
    return factory;
})();

PEG.Anything = (function (proto) {
    proto = proto || PEG.Pattern();
    proto.match = function match(input) {  // { value:, remainder: } | false
        VO.ensure(input.hasType(VO.String).or(input.hasType(VO.Array)));
        if (input.length.greaterThan(VO.zero) === VO.true) {
            return (VO.emptyObject  // match success
                .concatenate(s_value.bind(input.value(VO.zero)))
                .concatenate(s_remainder.bind(input.skip(VO.one))));
        }
        return VO.false;  // match failure
    };
    var constructor = function Anything() {
        if (instance) { return instance; }  // return cached singleton
        if (!(this instanceof Anything)) { return new Anything(); }  // if called without "new" keyword...
    };
    proto.constructor = constructor;
    constructor.prototype = proto;
    var instance = new constructor();  // cache singleton instance
    return constructor;
})();

PEG.kind["terminal"] = (function (factory) {
    factory = factory || function PEG_terminal(ast, grammar) {  // { "kind": "terminal", "value": <value> }
        VO.ensure(ast.hasType(VO.Object));
        VO.ensure(ast.hasProperty(s_kind));
        VO.ensure(ast.value(s_kind).equals(VO.String("terminal")));
        VO.ensure(ast.hasProperty(s_value));
        return PEG.Terminal(ast.value(s_value));
    };
    return factory;
})();

PEG.Terminal = (function (proto) {
    proto = proto || PEG.Pattern();
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

PEG.kind["range"] = (function (factory) {
    factory = factory || function PEG_range(ast, grammar) {  // { "kind": "range", "from": <number>, "to": <number> }
        VO.ensure(ast.hasType(VO.Object));
        VO.ensure(ast.hasProperty(s_kind));
        VO.ensure(ast.value(s_kind).equals(VO.String("range")));
        VO.ensure(ast.hasProperty(VO.String("from")));
        VO.ensure(ast.hasProperty(VO.String("to")));
        return PEG.Range(ast.value(VO.String("from")), ast.value(VO.String("to")));
    };
    return factory;
})();

PEG.Range = (function (proto) {
    proto = proto || PEG.Pattern();
    proto.match = function match(input) {  // { value:, remainder: } | false
        VO.ensure(input.hasType(VO.String).or(input.hasType(VO.Array)));
        if (input.length.greaterThan(VO.zero) === VO.true) {
            var value = input.value(VO.zero);
            if (this._from.lessEqual(value)
                .and(this._to.greaterEqual(value)) === VO.true) {
                return (VO.emptyObject  // match success
                    .concatenate(s_value.bind(value))
                    .concatenate(s_remainder.bind(input.skip(VO.one))));
            }
        }
        return VO.false;  // match failure
    };
    var constructor = function Range(from, to) {
        if (!(this instanceof Range)) { return new Range(from, to); }  // if called without "new" keyword...
        VO.ensure(from.hasType(VO.Number));
        VO.ensure(to.hasType(VO.Number));
        VO.ensure(from.lessEqual(to));
        this._from = from;
        this._to = to;
    };
    proto.constructor = constructor;
    constructor.prototype = proto;
    return constructor;
})();

PEG.kind["rule"] = (function (factory) {
    factory = factory || function PEG_rule(ast, grammar) {  // { "kind": "rule", "name": <string> }
        VO.ensure(ast.hasType(VO.Object));
        VO.ensure(ast.hasProperty(s_kind));
        VO.ensure(ast.value(s_kind).equals(VO.String("rule")));
        VO.ensure(ast.hasProperty(VO.String("name")));
        VO.ensure(ast.value(VO.String("name")).hasType(VO.String));
        return PEG.Rule(ast.value(VO.String("name")), grammar);
    };
    return factory;
})();

PEG.Rule = (function (proto) {
    proto = proto || PEG.Pattern();
    proto.match = function match(input) {  // { value:, remainder: } | false
        VO.ensure(input.hasType(VO.String).or(input.hasType(VO.Array)));
        var rule = this._resolve();  // lookup is delayed to allow mutally recursive references
        return rule.match(input);
    };
    var constructor = function Rule(name, grammar) {
        if (!(this instanceof Rule)) { return new Rule(name, grammar); }  // if called without "new" keyword...
        VO.ensure(name.hasType(VO.String));
        this._name = name;
        this._resolve = function resolve() {
            return grammar.rule(name);  // avoid deepFreeze of grammar reference
        };
    };
    proto.constructor = constructor;
    constructor.prototype = proto;
    return constructor;
})();

PEG.kind["sequence"] = (function (factory) {
    factory = factory || function PEG_sequence(ast, grammar) {  // { "kind": "sequence", "of": <array> }
        VO.ensure(ast.hasType(VO.Object));
        VO.ensure(ast.hasProperty(s_kind));
        VO.ensure(ast.value(s_kind).equals(VO.String("sequence")));
        VO.ensure(ast.hasProperty(VO.String("of")));
        VO.ensure(ast.value(VO.String("of")).hasType(VO.Array));
        var _of = ast.value(VO.String("of")).reduce(function (v, x) {
            var expr = grammar.compile(v);  // compile expression
            return x.append(expr);
        }, VO.emptyArray);
        return PEG.Sequence(_of);
    };
    return factory;
})();

PEG.Sequence = (function (proto) {
    proto = proto || PEG.Pattern();
    proto.match = function match(input) {  // { value:, remainder: } | false
        VO.ensure(input.hasType(VO.String).or(input.hasType(VO.Array)));
        var match = this._of.reduce(function (v, x) {
            if (x === VO.false) {
                return VO.false;  // propagate failure
            }
            var _match = v.match(x.value(s_remainder));
            if (_match === VO.false) {
                return VO.false;  // match failure
            }
            var _value = x.value(s_value).append(_match.value(s_value));
            return (_match.concatenate(s_value.bind(_value)));  // update successful match
        }, (VO.emptyObject  // match success (default)
            .concatenate(s_value.bind(VO.emptyArray))
            .concatenate(s_remainder.bind(input)))
        );
        return match;
    };
    var constructor = function Sequence(_of) {
        if (!(this instanceof Sequence)) { return new Sequence(_of); }  // if called without "new" keyword...
        VO.ensure(_of.hasType(VO.Array));
        this._of = _of;
    };
    proto.constructor = constructor;
    constructor.prototype = proto;
    return constructor;
})();

PEG.kind["alternative"] = (function (factory) {
    factory = factory || function PEG_alternative(ast, grammar) {  // { "kind": "alternative", "of": <array> }
        VO.ensure(ast.hasType(VO.Object));
        VO.ensure(ast.hasProperty(s_kind));
        VO.ensure(ast.value(s_kind).equals(VO.String("alternative")));
        VO.ensure(ast.hasProperty(VO.String("of")));
        VO.ensure(ast.value(VO.String("of")).hasType(VO.Array));
        var _of = ast.value(VO.String("of")).reduce(function (v, x) {
            var expr = grammar.compile(v);  // compile expression
            return x.append(expr);
        }, VO.emptyArray);
        return PEG.Alternative(_of);
    };
    return factory;
})();

PEG.Alternative = (function (proto) {
    proto = proto || PEG.Pattern();
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

PEG.kind["star"] = (function (factory) {
    factory = factory || function PEG_star(ast, grammar) {  // { "kind": "star", "expr": <object> }
        VO.ensure(ast.hasType(VO.Object));
        VO.ensure(ast.hasProperty(s_kind));
        VO.ensure(ast.value(s_kind).equals(VO.String("star")));
        VO.ensure(ast.hasProperty(VO.String("expr")));
        VO.ensure(ast.value(VO.String("expr")).hasType(VO.Object));
        var expr = grammar.compile(ast.value(VO.String("expr")));  // compile expression
        return PEG.Star(expr);
    };
    return factory;
})();

PEG.Star = (function (proto) {  // star(expr) = alternative(sequence(expr, star(expr)), nothing)
    proto = proto || PEG.Pattern();
    proto.match = function match(input) {  // { value:, remainder: } | false
        VO.ensure(input.hasType(VO.String).or(input.hasType(VO.Array)));
        return match_repeat(input, this._expr, 0);
    };
    var constructor = function Star(expr) {
        if (!(this instanceof Star)) { return new Star(expr); }  // if called without "new" keyword...
        VO.ensure(expr.hasType(PEG.Pattern));
        this._expr = expr;
    };
    proto.constructor = constructor;
    constructor.prototype = proto;
    return constructor;
})();

PEG.kind["plus"] = (function (factory) {
    factory = factory || function PEG_plus(ast, grammar) {  // { "kind": "plus", "expr": <object> }
        VO.ensure(ast.hasType(VO.Object));
        VO.ensure(ast.hasProperty(s_kind));
        VO.ensure(ast.value(s_kind).equals(VO.String("plus")));
        VO.ensure(ast.hasProperty(VO.String("expr")));
        VO.ensure(ast.value(VO.String("expr")).hasType(VO.Object));
        var expr = grammar.compile(ast.value(VO.String("expr")));  // compile expression
        return PEG.Plus(expr);
    };
    return factory;
})();

PEG.Plus = (function (proto) {  // plus(expr) = sequence(expr, star(expr))
    proto = proto || PEG.Pattern();
    proto.match = function match(input) {  // { value:, remainder: } | false
        VO.ensure(input.hasType(VO.String).or(input.hasType(VO.Array)));
        return match_repeat(input, this._expr, 1);
    };
    var constructor = function Plus(expr) {
        if (!(this instanceof Plus)) { return new Plus(expr); }  // if called without "new" keyword...
        VO.ensure(expr.hasType(PEG.Pattern));
        this._expr = expr;
    };
    proto.constructor = constructor;
    constructor.prototype = proto;
    return constructor;
})();

PEG.kind["optional"] = (function (factory) {
    factory = factory || function PEG_optional(ast, grammar) {  // { "kind": "optional", "expr": <object> }
        VO.ensure(ast.hasType(VO.Object));
        VO.ensure(ast.hasProperty(s_kind));
        VO.ensure(ast.value(s_kind).equals(VO.String("optional")));
        VO.ensure(ast.hasProperty(VO.String("expr")));
        VO.ensure(ast.value(VO.String("expr")).hasType(VO.Object));
        var expr = grammar.compile(ast.value(VO.String("expr")));  // compile expression
        return PEG.Optional(expr);
    };
    return factory;
})();

PEG.Optional = (function (proto) {  // optional(expr) = alternative(expr, nothing)
    proto = proto || PEG.Pattern();
    proto.match = function match(input) {  // { value:, remainder: } | false
        VO.ensure(input.hasType(VO.String).or(input.hasType(VO.Array)));
        return match_repeat(input, this._expr, 0, 1);
    };
    var constructor = function Optional(expr) {
        if (!(this instanceof Optional)) { return new Optional(expr); }  // if called without "new" keyword...
        VO.ensure(expr.hasType(PEG.Pattern));
        this._expr = expr;
    };
    proto.constructor = constructor;
    constructor.prototype = proto;
    return constructor;
})();

/*
 * <FOR-REFERENCE-ONLY>
 */
var REMOVE_THIS_JUNK = (function () {
    var kind = {};

    var match_repeat = function match_repeat(input, expr, min, max) {
        var count = 0;
        var match = (VO.emptyObject  // match success (default)
            .concatenate(s_value.bind(VO.emptyArray))
            .concatenate(s_remainder.bind(input)));
        while ((max === undefined) || (count < max)) {
            var _match = expr.match(input);
            if (_match === VO.false) {
                break;  // match failure
            }
            count += 1;  // update match count
            input = _match.value(s_remainder);  // update input position
            let s_value = s_value;
            match = (VO.emptyObject  // update successful match
                .concatenate(s_value.bind(
                        match.value(s_value).append(_match.value(s_value))))
                .concatenate(s_remainder.bind(input)));
        }
        if (count < min) {
            match = VO.false;  // match failure
        }
        return match;
    };
    kind["optional"] = (function (constructor) {  // optional(expr) = alternative(expr, nothing)
        constructor = constructor || function PEG_optional(ast, g) {  // { "kind": "optional", "expr": <object> }
            VO.ensure(ast.hasType(VO.Object));
            VO.ensure(ast.hasProperty(s_kind));
            VO.ensure(ast.value(s_kind).equals(VO.String("optional")));
            VO.ensure(ast.hasProperty(VO.String("expr")));
            VO.ensure(ast.value(VO.String("expr")).hasType(VO.Object));
            this._ast = ast;
            this._expr = compile_expr(ast.value(VO.String("expr")), g);  // compile expression
        };
        var prototype = constructor.prototype;
        prototype.constructor = constructor;
        prototype.match = function match_optional(input) {
            VO.ensure(input.hasType(VO.String).or(input.hasType(VO.Array)));
            return match_repeat(input, this._expr, 0, 1);
        };
        return constructor;
    })();
});
/*
 * </FOR-REFERENCE-ONLY>
 */

PEG.selfTest = (function () {

    var test_PEG = function test_PEG() {
        var input;
        var match;

        var grammar = PEG.factory(VO.fromNative({ //"lang": "PEG", "ast": {
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
        PEG.log('grammar:', grammar);

        match = grammar.rule(VO.String("nothing")).match(VO.emptyString);
        VO.ensure(match.hasType(VO.Object));
        VO.ensure(match.value(s_remainder).equals(VO.emptyString));

        match = grammar.rule(VO.String("nothing")).match(VO.emptyArray);
        VO.ensure(match.hasType(VO.Object));
        VO.ensure(match.value(s_remainder).equals(VO.emptyArray));

        match = grammar.rule(VO.String("anything")).match(VO.emptyArray);
        VO.ensure(match.equals(VO.false));

        match = grammar.rule(VO.String("digit0")).match(VO.emptyString);
        VO.ensure(match.equals(VO.false));

        input = VO.fromNative("0\n");
        match = grammar.rule(VO.String("fail")).match(input);
        VO.ensure(match.equals(VO.false));

        match = grammar.rule(VO.String("digit0")).match(input);
        VO.ensure(match.hasType(VO.Object));
        VO.ensure(match.value(s_value).equals(VO.Number(48)));  // "0"
        VO.ensure(match.value(s_remainder).length.equals(VO.Number(1)));

        match = grammar.rule(VO.String("digit0-9")).match(input);
        VO.ensure(match.hasType(VO.Object));
        VO.ensure(match.value(s_value).equals(VO.Number(48)));  // "0"
        VO.ensure(match.value(s_remainder).length.equals(VO.Number(1)));

        match = grammar.rule(VO.String("digit1-9")).match(input);
        VO.ensure(match.equals(VO.false));

        input = VO.fromNative("\r\n");
        match = grammar.rule(VO.String("anything")).match(input);
        VO.ensure(match.hasType(VO.Object));
        VO.ensure(match.value(s_value).equals(VO.Number(13)));  // "\r"
        VO.ensure(match.value(s_remainder).length.equals(VO.Number(1)));
        VO.ensure(match.value(s_remainder).equals(VO.String("\n")));

        match = grammar.rule(VO.String("e")).match(VO.String("E"));
        VO.ensure(match.equals(VO.false).not);
        VO.ensure(match.value(s_value).equals(VO.fromNative(69)));
        VO.ensure(match.value(s_remainder).length.equals(VO.zero));

        match = grammar.rule(VO.String("integer")).match(VO.String("0"));
        VO.ensure(match.equals(VO.false).not);
        VO.ensure(match.value(s_value).equals(VO.fromNative(48)));
        VO.ensure(match.value(s_remainder).length.equals(VO.zero));

        match = grammar.rule(VO.String("integer")).match(VO.String("00"));
        VO.ensure(match.equals(VO.false).not);
        VO.ensure(match.value(s_value).equals(VO.fromNative(48)));
        VO.ensure(match.value(s_remainder).equals(VO.fromNative("0")));

        match = grammar.rule(VO.String("integer")).match(VO.String("1"));
        VO.ensure(match.equals(VO.false).not);
        VO.ensure(match.value(s_value).equals(VO.fromNative([49, []])));
        VO.ensure(match.value(s_remainder).length.equals(VO.zero));

        match = grammar.rule(VO.String("integer")).match(VO.String("12"));
        VO.ensure(match.equals(VO.false).not);
        VO.ensure(match.value(s_value).equals(VO.fromNative([49, [50]])));
        VO.ensure(match.value(s_remainder).length.equals(VO.zero));

        match = grammar.rule(VO.String("integer")).match(VO.String("123"));
        VO.ensure(match.equals(VO.false).not);
        VO.ensure(match.value(s_value).equals(VO.fromNative([49, [50, 51]])));
        VO.ensure(match.value(s_remainder).length.equals(VO.zero));

        match = grammar.rule(VO.String("integer")).match(VO.String("3.14e+0"));
        VO.ensure(match.equals(VO.false).not);
        VO.ensure(match.value(s_value).equals(VO.fromNative([51, []])));
        VO.ensure(match.value(s_remainder).equals(VO.fromNative(".14e+0")));

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
        test_PEG();
        return true;  // all tests passed
    };

})();
