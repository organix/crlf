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

crlf.language = {};  // language factory namespace

crlf.language["PEG"] = (function (PEG) {
    PEG = PEG || {};
    PEG["grammar"] = (function (constructor) {
        constructor = constructor || function PEG_grammar(ast) {
            this._ast = ast;
            this._rules = {};
            Object.keys(ast.rules).forEach(function (key) {
                var expr = ast.rules[key];
                this._rules[key] = new (PEG[expr.kind])(expr);  // compile rules
            }, this);
        };
        var prototype = constructor.prototype;
        prototype.constructor = constructor;
        prototype.rule = function rule(name) {  // get named rule
            return this._rules[name];
        };
        prototype.match = function match(name, input) {  // apply named rule to input
            return this.rule(name).match(input);
        };
        return constructor;
    })();
    PEG["fail"] = (function (constructor) {
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
    PEG["empty"] = (function (constructor) {
        constructor = constructor || function PEG_empty(ast) {
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
    PEG["anything"] = (function (constructor) {
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
    PEG["terminal"] = (function (constructor) {
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
    return PEG;
})();

crlf.compile = function compile(source) {
    return new (crlf.language[source.lang])(source.ast);  // compile crlf language
};

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
        var grammar = crlf.compile(source);

        return true;  // all tests passed
    };
})();
