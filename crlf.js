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
crlf.version = "0.0.0";

//crlf.log = console.log;
crlf.log = function () {};

crlf.compile = function compile(source) {  // { "lang": <string>, "ast": <value> }
    VO.ensure(source.hasType(VO.Object));
    var constructor = crlf.language[source.value(new VO.String("lang"))._value];
    return new constructor(source.value(new VO.String("ast")));  // compile crlf language ast
};

crlf.language = {};  // language factory namespace

crlf.language["PEG"] = (function (constructor) {
    var kind = {};
    var compile = function compile(expr, grammar) {  // { "kind": <string>, ... }
        VO.ensure(expr.hasType(VO.Object));
        VO.ensure(expr.hasProperty(new VO.String("kind")));
        var constructor = kind[expr.value(new VO.String("kind"))._value];
        return new constructor(expr, grammar);
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
            rules[n._value] = compile(v, grammar);  // compile rules
            return x;
        }, VO.null);
        this._rules = rules;
    };
    var prototype = constructor.prototype;
    prototype.constructor = constructor;
    prototype.rule = function rule(name) {  // get named rule
        VO.ensure(name.hasType(VO.String));
        return this._rules[name._value];
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
        prototype.match = function match(input) {
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
        prototype.match = function match(input) {
            VO.ensure(input.hasType(VO.String).or(input.hasType(VO.Array)));
            return (VO.emptyObject  // match success
                .append(new VO.String("value"), VO.emptyArray)
                .append(new VO.String("remainder"), input));
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
        prototype.match = function match(input) {
            VO.ensure(input.hasType(VO.String).or(input.hasType(VO.Array)));
            if (input.length().greaterThan(VO.zero) === VO.true) {
                return (VO.emptyObject  // match success
                    .append(new VO.String("value"), input.value(VO.zero))
                    .append(new VO.String("remainder"), input.extract(VO.one, input.length())));
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
        prototype.match = function match(input) {
            VO.ensure(input.hasType(VO.String).or(input.hasType(VO.Array)));
            if (input.length().greaterThan(VO.zero) === VO.true) {
                var value = input.value(VO.zero);
                if (value.equals(this._ast.value(new VO.String("value"))) === VO.true) {
                    return (VO.emptyObject  // match success
                        .append(new VO.String("value"), value)
                        .append(new VO.String("remainder"), input.extract(VO.one, input.length())));
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
        prototype.match = function match(input) {
            VO.ensure(input.hasType(VO.String).or(input.hasType(VO.Array)));
            if (input.length().greaterThan(VO.zero) === VO.true) {
                var value = input.value(VO.zero);
                if (value.greaterEqual(this._ast.value(new VO.String("from")))
                .and(value.lessEqual(this._ast.value(new VO.String("to")))) === VO.true) {
                    return (VO.emptyObject  // match success
                        .append(new VO.String("value"), value)
                        .append(new VO.String("remainder"), input.extract(VO.one, input.length())));
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
        prototype.match = function match(input) {
            VO.ensure(input.hasType(VO.String).or(input.hasType(VO.Array)));
            var name = this._ast.value(new VO.String("name"));
            return this._grammar.rule(name).match(input);
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
                rules.push(compile(v, g));  // compile rules
                return x;
            }, VO.null);
            this._of = rules;
        };
        var prototype = constructor.prototype;
        prototype.constructor = constructor;
        prototype.match = function match(input) {
            VO.ensure(input.hasType(VO.String).or(input.hasType(VO.Array)));
            var match = (VO.emptyObject  // match success (default)
                .append(new VO.String("value"), VO.emptyArray)
                .append(new VO.String("remainder"), input));
            this._of.forEach(function (rule) {
                var _match = rule.match(input);
                if (_match === VO.false) {
                    return VO.false;  // match failure
                }
                input = _match.value(new VO.String("remainder"));  // update input position
                match = (VO.emptyObject  // update successful match
                    .append(new VO.String("value"),
                            match.value(new VO.String("value"))
                                .append(_match.value(new VO.String("value"))))
                    .append(new VO.String("remainder"), input));
            });
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
                rules.push(compile(v, g));  // compile rules
                return x;
            }, VO.null);
            this._of = rules;
        };
        var prototype = constructor.prototype;
        prototype.constructor = constructor;
        prototype.match = function match(input) {
            VO.ensure(input.hasType(VO.String).or(input.hasType(VO.Array)));
            this._of.forEach(function (rule) {
                var match = rule.match(input);
                if (match !== VO.false) {
                    return match;  // match success
                }
            });
            return VO.false;  // match failure
        };
        return constructor;
    })();
    return constructor;
})();

crlf.selfTest = (function () {
    var source = VO.fromNative({
        "lang": "PEG",
        "ast": {
            "kind": "grammar",
            "rules": {
                "integer": {
                    "kind": "alternative",
                    "of": [
                        { "kind": "rule", "name": "digit0" },
                        {
                            "kind": "sequence",
                            "of": [
                                { "kind": "rule", "name": "digit1-9" },
/*
                                {
                                    "kind": "star",
                                    "expr": { "kind": "rule", "name": "digit0-9" }
                                }
*/
                                { "kind": "rule", "name": "digit0-9" }
                            ]
                        }
                    ]
                },
                "digit0": { "kind": "terminal", "value": 48 },
                "digit1-9": { "kind": "range", "from": 49, "to": 57 },
                "digit0-9": { "kind": "range", "from": 48, "to": 57 },
                "anything": { "kind": "anything" },
                "nothing": { "kind": "nothing" },
                "fail": { "kind": "fail" }
            }
        }
    });

    return function selfTest() {
        var grammar;
        var input;
        var match;

        grammar = crlf.compile(source);

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
        VO.ensure(match.value(new VO.String("remainder")).length().equals(new VO.Number(1)));

        match = grammar.rule(new VO.String("digit0-9")).match(input);
        VO.ensure(match.hasType(VO.Object));
        VO.ensure(match.value(new VO.String("value")).equals(new VO.Number(48)));  // "0"
        VO.ensure(match.value(new VO.String("remainder")).length().equals(new VO.Number(1)));

        match = grammar.rule(new VO.String("digit1-9")).match(input);
        VO.ensure(match.equals(VO.false));

        input = VO.fromNative("\r\n");
        match = grammar.rule(new VO.String("anything")).match(input);
        VO.ensure(match.hasType(VO.Object));
        VO.ensure(match.value(new VO.String("value")).equals(new VO.Number(13)));  // "\r"
        VO.ensure(match.value(new VO.String("remainder")).length().equals(new VO.Number(1)));

        match = grammar.rule(new VO.String("integer")).match(new VO.String("0"));
        VO.ensure(match.equals(VO.false).not());
//        VO.ensure(match.value(new VO.String("value")).equals(new VO.String("0")));
        VO.ensure(match.value(new VO.String("remainder")).length().equals(VO.zero));

        match = grammar.rule(new VO.String("integer")).match(new VO.String("00"));
        VO.ensure(match.equals(VO.false));

        match = grammar.rule(new VO.String("integer")).match(new VO.String("1"));
        VO.ensure(match.equals(VO.false).not());
//        VO.ensure(match.value(new VO.String("value")).equals(new VO.String("1")));
        VO.ensure(match.value(new VO.String("remainder")).length().equals(VO.zero));

        match = grammar.rule(new VO.String("integer")).match(new VO.String("12"));
        VO.ensure(match.equals(VO.false).not());
//        VO.ensure(match.value(new VO.String("value")).equals(new VO.String("12")));
        VO.ensure(match.value(new VO.String("remainder")).length().equals(VO.zero));

        match = grammar.rule(new VO.String("integer")).match(new VO.String("123"));
        VO.ensure(match.equals(VO.false));

        return true;  // all tests passed
    };
})();
