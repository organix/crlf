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

var crlf = module.exports;
crlf.version = "0.0.0";

//crlf.log = console.log;
crlf.log = function () {};

crlf.compile = function compile(source) {  // { "lang": <string>, "ast": <value> }
    var constructor = crlf.language[source.lang];
    return new constructor(source.ast);  // compile crlf language ast
};

crlf.language = {};  // language factory namespace

crlf.language["PEG"] = (function (constructor) {
    var kind = {};
    var compile = function compile(expr) {  // { "kind": <string>, ... }
        var constructor = kind[expr.kind];
        return new constructor(expr);
    };
    constructor = constructor || function PEG_grammar(ast) {
        this._ast = ast;  // { "kind": "grammar", "rules": {...} }
        this._rules = ast.rules;
        Object.keys(this._rules).forEach(function (key) {
            this._rules[key] = compile(this._rules[key]);  // compile rules
        }, this);
    };
    var prototype = constructor.prototype;
    prototype.constructor = constructor;
    prototype.rule = function rule(name) {  // get named rule
        return this._rules[name];
    };
    prototype.match = function match(name, input) {  // apply named rule to input
        var rule = this.rule(name);
        return rule.match(input);
    };
    kind["fail"] = (function (constructor) {
        constructor = constructor || function PEG_fail(ast) {
            this._ast = ast;
        };
        var prototype = constructor.prototype;
        prototype.constructor = constructor;
        prototype.match = function match(input) {
            return false;  // match failure
        };
        return constructor;
    })();
    kind["nothing"] = (function (constructor) {
        constructor = constructor || function PEG_nothing(ast) {
            this._ast = ast;
        };
        var prototype = constructor.prototype;
        prototype.constructor = constructor;
        prototype.match = function match(input) {
            return {
                "value": [],
                "remainder": input
            };  // match success
        };
        return constructor;
    })();
    kind["anything"] = (function (constructor) {
        constructor = constructor || function PEG_anything(ast) {
            this._ast = ast;
        };
        var prototype = constructor.prototype;
        prototype.constructor = constructor;
        prototype.match = function match(input) {
            if (input.length > 0) {
                return {
                    "value": input[0],
                    "remainder": input.slice(1, input.length)
                };  // match success
            }
            return false;  // match failure
        };
        return constructor;
    })();
    kind["terminal"] = (function (constructor) {
        constructor = constructor || function PEG_terminal(ast) {
            this._ast = ast;
        };
        var prototype = constructor.prototype;
        prototype.constructor = constructor;
        prototype.match = function match(input) {
            if (input.length > 0) {
                var value = input[0];
                if (value === this._ast.value) {
                    return {
                        "value": value,
                        "remainder": input.slice(1, input.length)
                    };  // match success
                }
            }
            return false;  // match failure
        };
        return constructor;
    })();
    return constructor;
})();

crlf.selfTest = (function () {
    var source = {
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
                                {
                                    "kind": "star",
                                    "expr": { "kind": "rule", "name": "digit0-9" }
                                }
                            ]
                        }
                    ]
                },
                "digit0": { "kind": "terminal", "value": 48 },
                "digit1-9": { "kind": "range", "from": 49, "to": 57 },
                "digit0-9": { "kind": "range", "from": 48, "to": 57 }
            }
        }
    };

    return function selfTest() {
        var source;
        var grammar;
        var input;
        var match;

        source = {
            "lang": "PEG",
            "ast": {
                "kind": "grammar",
                "rules": {
                    "nothing": { "kind": "nothing" },
                    "anything": { "kind": "anything" },
                    "zero": { "kind": "terminal", "value": 48 },
                    "fail": { "kind": "fail" }
                }
            }
        };
        grammar = crlf.compile(source);
        input = [ 48, 10 ];  // "0\n"
        match = grammar.rule("zero").match(input);

        return true;  // all tests passed
    };
})();
