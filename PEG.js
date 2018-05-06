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

PEG.kind = {};  // matching operator factory namespace

PEG.factory = function compile_PEG(source) {  // { "kind": "grammar", "rules": <object> }
    VO.ensure(source.hasType(VO.Object));
    VO.ensure(source.hasProperty(VO.String("kind")));
    VO.ensure(source.value(VO.String("kind")).equals(VO.String("grammar")));
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
        VO.ensure(ast.hasProperty(VO.String("kind")));
        VO.ensure(ast.value(VO.String("kind")).hasType(VO.String));
        var _kind = ast.value(VO.String("kind"))._value;
        var factory = PEG.kind[_kind];
        var expr = factory(ast, this);
        return expr;
    };
    proto.compile_rule = function compile_rule(name, ast) {  // { "kind": <string>, ... }
        VO.ensure(name.hasType(VO.String));
        var expr = this.compile(ast);
//        this._rules[name._value] = expr;
        this._rules = this._rules.concatenate(name.bind(expr));
        PEG.log('rule:', name._value, ':=', expr);
        return this;
    };
    proto.rule = function rule(name) {  // get named rule
        VO.ensure(name.hasType(VO.String));
//        var expr = this._rules[name._value];
        var expr = this._rules.value(name);
        PEG.log('rule:', name._value, '->', expr);
        return expr;
    };
    var constructor = function Grammar(rules) {
        if (!(this instanceof Grammar)) { return new Grammar(rules); }  // if called without "new" keyword...
        rules = rules || VO.emptyObject;
        VO.ensure(rules.hasType(VO.Object));
        this._rules = rules;
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

PEG.Fail = (function (proto) {
    proto = proto || PEG.Pattern();
    proto.match = function match(input) {  // { value:, remainder: } | false
        VO.ensure(input.hasType(VO.String).or(input.hasType(VO.Array)));
        return VO.false;  // match failure
    };
    var constructor = function Fail() {
        if (!(this instanceof Fail)) { return new Fail(); }  // if called without "new" keyword...
    };
    proto.constructor = constructor;
    constructor.prototype = proto;
    return constructor;
})();

PEG.kind["fail"] = (function (factory) {
    factory = factory || function PEG_fail(ast, grammar) {  // { "kind": "fail" }
        VO.ensure(ast.hasType(VO.Object));
        VO.ensure(ast.hasProperty(VO.String("kind")));
        VO.ensure(ast.value(VO.String("kind")).equals(VO.String("fail")));
        return PEG.Fail();
    };
    return factory;
})();

PEG.Nothing = (function (proto) {
    proto = proto || PEG.Pattern();
    proto.match = function match(input) {  // { value:, remainder: } | false
        VO.ensure(input.hasType(VO.String).or(input.hasType(VO.Array)));
        return (VO.emptyObject  // match success
            .concatenate((VO.String("value")).bind(VO.emptyArray))
            .concatenate((VO.String("remainder")).bind(input)));
    };
    var constructor = function Nothing() {
        if (!(this instanceof Nothing)) { return new Nothing(); }  // if called without "new" keyword...
    };
    proto.constructor = constructor;
    constructor.prototype = proto;
    return constructor;
})();

PEG.kind["nothing"] = (function (factory) {
    factory = factory || function PEG_nothing(ast, grammar) {  // { "kind": "nothing" }
        VO.ensure(ast.hasType(VO.Object));
        VO.ensure(ast.hasProperty(VO.String("kind")));
        VO.ensure(ast.value(VO.String("kind")).equals(VO.String("nothing")));
        return PEG.Nothing();
    };
    return factory;
})();

PEG.Anything = (function (proto) {
    proto = proto || PEG.Pattern();
    proto.match = function match(input) {  // { value:, remainder: } | false
        VO.ensure(input.hasType(VO.String).or(input.hasType(VO.Array)));
        if (input.length.greaterThan(VO.zero) === VO.true) {
            return (VO.emptyObject  // match success
                .concatenate((VO.String("value")).bind(input.value(VO.zero)))
                .concatenate((VO.String("remainder")).bind(input.skip(VO.one))));
        }
        return VO.false;  // match failure
    };
    var constructor = function Anything() {
        if (!(this instanceof Anything)) { return new Anything(); }  // if called without "new" keyword...
    };
    proto.constructor = constructor;
    constructor.prototype = proto;
    return constructor;
})();

PEG.kind["anything"] = (function (factory) {
    factory = factory || function PEG_anything(ast, grammar) {  // { "kind": "anything" }
        VO.ensure(ast.hasType(VO.Object));
        VO.ensure(ast.hasProperty(VO.String("kind")));
        VO.ensure(ast.value(VO.String("kind")).equals(VO.String("anything")));
        return PEG.Anything();
    };
    return factory;
})();

PEG.Rule = (function (proto) {
    proto = proto || PEG.Pattern();
    proto.match = function match(input) {  // { value:, remainder: } | false
        VO.ensure(input.hasType(VO.String).or(input.hasType(VO.Array)));
        var rule = this._grammar.rule(this._name);
        return rule.match(input);
    };
    var constructor = function Rule(name, grammar) {
        if (!(this instanceof Rule)) { return new Rule(); }  // if called without "new" keyword...
        VO.ensure(name.hasType(VO.String));
        VO.ensure(grammar.hasType(PEG.Grammar));
        this._name = name;
        this._grammar = grammar;
    };
    proto.constructor = constructor;
    constructor.prototype = proto;
    return constructor;
})();

PEG.kind["rule"] = (function (factory) {
    factory = factory || function PEG_rule(ast, grammar) {  // { "kind": "rule", "name": <string> }
        VO.ensure(ast.hasType(VO.Object));
        VO.ensure(ast.hasProperty(VO.String("kind")));
        VO.ensure(ast.value(VO.String("kind")).equals(VO.String("rule")));
        VO.ensure(ast.hasProperty(VO.String("name")));
        VO.ensure(ast.value(VO.String("name")).hasType(VO.String));
        return PEG.Rule(ast.value(VO.String("name")), grammar);
    };
    return factory;
})();

PEG.Optional = (function (proto) {  // optional(expr) = alternative(expr, nothing)
    proto = proto || PEG.Pattern();
    proto.match = function match(input) {  // { value:, remainder: } | false
        VO.ensure(input.hasType(VO.String).or(input.hasType(VO.Array)));
        return match_repeat(input, this._expr, 0, 1);  // [FIXME: match_repeat NOT IMPLEMENTED!]
    };
    var constructor = function Optional(expr) {
        if (!(this instanceof Optional)) { return new Optional(); }  // if called without "new" keyword...
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
        VO.ensure(ast.hasProperty(VO.String("kind")));
        VO.ensure(ast.value(VO.String("kind")).equals(VO.String("optional")));
        VO.ensure(ast.hasProperty(VO.String("expr")));
        VO.ensure(ast.value(VO.String("expr")).hasType(VO.Object));
        var expr = grammar.compile(ast.value(VO.String("expr")));  // compile expression
        return PEG.Optional(expr);
    };
    return factory;
})();

/*
 * <FOR-REFERENCE-ONLY>
 */
var REMOVE_THIS_JUNK = (function () {
    var kind = {};

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
});
/*
 * </FOR-REFERENCE-ONLY>
 */

PEG.selfTest = (function () {

    var test_PEG = function test_PEG() {
        var input;
        var match;

        var grammar = PEG.factory(VO.fromNative({
/*
            "lang": "PEG",
            "ast": {
*/
                "kind": "grammar",
                "rules": {
/*
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
*/
                    "anything": { "kind": "anything" },
                    "nothing": { "kind": "nothing" },
                    "fail": { "kind": "fail" }
                }
/*
            }
*/
        }));

        match = grammar.rule(VO.String("nothing")).match(VO.emptyString);
        VO.ensure(match.hasType(VO.Object));
        VO.ensure(match.value(VO.String("remainder")).equals(VO.emptyString));

        match = grammar.rule(VO.String("nothing")).match(VO.emptyArray);
        VO.ensure(match.hasType(VO.Object));
        VO.ensure(match.value(VO.String("remainder")).equals(VO.emptyArray));

        match = grammar.rule(VO.String("anything")).match(VO.emptyArray);
        VO.ensure(match.equals(VO.false));

/*
        match = grammar.rule(new VO.String("digit0")).match(VO.emptyString);
        VO.ensure(match.equals(VO.false));
*/

        input = VO.fromNative("0\n");
        match = grammar.rule(VO.String("fail")).match(input);
        VO.ensure(match.equals(VO.false));

/*
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
*/

        input = VO.fromNative("\r\n");
        match = grammar.rule(VO.String("anything")).match(input);
        VO.ensure(match.hasType(VO.Object));
        VO.ensure(match.value(VO.String("value")).equals(VO.Number(13)));  // "\r"
        VO.ensure(match.value(VO.String("remainder")).length.equals(VO.Number(1)));
        VO.ensure(match.value(VO.String("remainder")).equals(VO.String("\n")));

/*
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
*/
    };

    return function selfTest() {
        VO.selfTest();  // test imported package first
        test_PEG();
        return true;  // all tests passed
    };

})();
