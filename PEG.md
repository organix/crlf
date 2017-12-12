# Parsing Expression Grammars

A [Parsing Expression Grammar](https://en.wikipedia.org/wiki/Parsing_expression_grammar) (PEG) is a pattern matching language.

## Primitive Expressions

```javascript
{
    "lang": "PEG",
    "ast": <grammar>
}
```

The `ast` represents a _grammar_, which contains a set of named _rules_:

```javascript
{
    "kind": "grammar",
    "rules": {
        <name>: <expression>,
        ...
    }
}
```

A parsing _expression_ is applied to an _source_ array value. If the expression fails to match, it produces a `false` _result_ value. If the expression matches, it produces a match _result_ object with an arbitrary _value_ and a _remainder_ array containing the unmatched portion of the source array:

```
{
    value: <any>,
    remainder: <array>
}
```

A primitive parsing expression may be any of the following:

  * Empty string
  * Terminal symbol
  * Rule reference
  * Sequence
  * Alternative (ordered choice)
  * Negation (look-ahead)

### Empty string

```javascript
{
    "kind": "empty"
}
```

Matches without consuming any input. The result _value_ is `[]`.

### Terminal symbol

```javascript
{
    "kind": "terminal",
    "value": <any>
}
```

If _source_ is empty, fail. Otherwise consume the first element of _source_ and match if it equals _value_. The result _value_ is the _value_ matched.

### Rule reference

```javascript
{
    "kind": "rule",
    "name": <string>
}
```

Apply the named rule to _source_. If matched, the result _value_ is the _value_ matched by the rule.

### Sequence

```javascript
{
    "kind": "sequence",
    "of": [
        <expression>,
        ...
    ]
}
```

Match all of the _expressions_ in _of_ to successive portions of the _source_. If matched, the result _value_ is an array of the _values_ matched by each expression.

### Alternative (ordered choice)

```javascript
{
    "kind": "alternative",
    "of": [
        <expression>,
        ...
    ]
}
```

Attempt to match each of the _expressions_ in _of_ in order until one matches the current _source_. If matched, the result _value_ is the _value_ matched.

### Negation (look-ahead)

```javascript
{
    "kind": "negation",
    "next": <expression>
}
```

Matches, with a result value `null` and _remainder_ = _source_, if _next_ **fails** to match.

## Derived Expressions

Many common (and useful) parsing expressions can be defined in terms of the primitives already specified.

Name | Description | Derivation
-----|-------------|-----------
anything | matches any single _value_ | alternative( _...all possible values..._ )
star(_expr_) | zero-or-more repetition | alternative(sequence(_expr_, star(_expr_)), empty)
plus(_expr_) | one-or-more repetition | sequence(_expr_, star(_expr_))
optional(_expr_) | zero-or-one occurance | alternative(_expr_, empty)
suffix(_expr_) | positive look-ahead | negation(negation(_expr_))
range(_from_, _to_) | matches any single _value_, where _from_ &#x2266; _value_ &#x2266; _to_ | alternative(_from_, ..., _to_)

## Examples

Illustrations of PEG grammars represented in CRLF format applied to real-world examples.

### JSON

[RFC 8259](http://ftp.ripe.net/rfc/authors/rfc8259.txt) and [ECMA-404](http://www.ecma-international.org/publications/files/ECMA-ST/ECMA-404.pdf) describe _The JavaScript Object Notation (JSON) Data Interchange Format_. Within these standards, the grammar for JSON is expressed in ABNF:

```ABNF
JSON-text = ws value ws

begin-array     = ws %x5B ws  ; [ left square bracket

begin-object    = ws %x7B ws  ; { left curly bracket

end-array       = ws %x5D ws  ; ] right square bracket

end-object      = ws %x7D ws  ; } right curly bracket

name-separator  = ws %x3A ws  ; : colon

value-separator = ws %x2C ws  ; , comma

ws = *(
      %x20 /              ; Space
      %x09 /              ; Horizontal tab
      %x0A /              ; Line feed or New line
      %x0D )              ; Carriage return

value = false / null / true / object / array / number / string

false = %x66.61.6c.73.65   ; false

null  = %x6e.75.6c.6c      ; null

true  = %x74.72.75.65      ; true

object = begin-object [ member *( value-separator member ) ]
       end-object

member = string name-separator value

array = begin-array [ value *( value-separator value ) ] end-array

number = [ minus ] int [ frac ] [ exp ]

decimal-point = %x2E       ; .

digit1-9 = %x31-39         ; 1-9

e = %x65 / %x45            ; e E

exp = e [ minus / plus ] 1*DIGIT

frac = decimal-point 1*DIGIT

int = zero / ( digit1-9 *DIGIT )

minus = %x2D               ; -

plus = %x2B                ; +

zero = %x30                ; 0

string = quotation-mark *char quotation-mark

char = unescaped /
  escape (
      %x22 /          ; "    quotation mark  U+0022
      %x5C /          ; \    reverse solidus U+005C
      %x2F /          ; /    solidus         U+002F
      %x62 /          ; b    backspace       U+0008
      %x66 /          ; f    form feed       U+000C
      %x6E /          ; n    line feed       U+000A
      %x72 /          ; r    carriage return U+000D
      %x74 /          ; t    tab             U+0009
      %x75 4HEXDIG )  ; uXXXX                U+XXXX

escape = %x5C              ; \

quotation-mark = %x22      ; "

unescaped = %x20-21 / %x23-5B / %x5D-10FFFF
```

### ABNF

[RFC 5234](https://www.ietf.org/rfc/rfc5234.txt) describes _Augmented BNF for Syntax Specifications_. Within this standard a set of "Core" rules are defined, which are often assumed by other standards (such as JSON).

```ABNF
ALPHA          =  %x41-5A / %x61-7A   ; A-Z / a-z

BIT            =  "0" / "1"

CHAR           =  %x01-7F
                    ; any 7-bit US-ASCII character,
                    ;  excluding NUL

CR             =  %x0D
                    ; carriage return

CRLF           =  CR LF
                    ; Internet standard newline

CTL            =  %x00-1F / %x7F
                    ; controls

DIGIT          =  %x30-39
                    ; 0-9

DQUOTE         =  %x22
                    ; " (Double Quote)

HEXDIG         =  DIGIT / "A" / "B" / "C" / "D" / "E" / "F"

HTAB           =  %x09
                    ; horizontal tab

LF             =  %x0A
                    ; linefeed

LWSP           =  *(WSP / CRLF WSP)
                    ; Use of this linear-white-space rule
                    ;  permits lines containing only white
                    ;  space that are no longer legal in
                    ;  mail headers and have caused
                    ;  interoperability problems in other
                    ;  contexts.
                    ; Do not use when defining mail
                    ;  headers and use with caution in
                    ;  other contexts.

OCTET          =  %x00-FF
                    ; 8 bits of data

SP             =  %x20

VCHAR          =  %x21-7E
                    ; visible (printing) characters

WSP            =  SP / HTAB
                    ; white space
```

The definition of ABNF syntax in ABNF (building on the Core rules) is:

```ABNF
rulelist       =  1*( rule / (*c-wsp c-nl) )

rule           =  rulename defined-as elements c-nl
                    ; continues if next line starts
                    ;  with white space

rulename       =  ALPHA *(ALPHA / DIGIT / "-")

defined-as     =  *c-wsp ("=" / "=/") *c-wsp
                    ; basic rules definition and
                    ;  incremental alternatives

elements       =  alternation *c-wsp

c-wsp          =  WSP / (c-nl WSP)

c-nl           =  comment / CRLF
                    ; comment or newline

comment        =  ";" *(WSP / VCHAR) CRLF

alternation    =  concatenation
               *(*c-wsp "/" *c-wsp concatenation)

concatenation  =  repetition *(1*c-wsp repetition)

repetition     =  [repeat] element

repeat         =  1*DIGIT / (*DIGIT "*" *DIGIT)

element        =  rulename / group / option /
               char-val / num-val / prose-val

group          =  "(" *c-wsp alternation *c-wsp ")"

option         =  "[" *c-wsp alternation *c-wsp "]"

char-val       =  DQUOTE *(%x20-21 / %x23-7E) DQUOTE
                    ; quoted string of SP and VCHAR
                    ;  without DQUOTE

num-val        =  "%" (bin-val / dec-val / hex-val)

bin-val        =  "b" 1*BIT
               [ 1*("." 1*BIT) / ("-" 1*BIT) ]
                    ; series of concatenated bit values
                    ;  or single ONEOF range

dec-val        =  "d" 1*DIGIT
               [ 1*("." 1*DIGIT) / ("-" 1*DIGIT) ]

hex-val        =  "x" 1*HEXDIG
               [ 1*("." 1*HEXDIG) / ("-" 1*HEXDIG) ]

prose-val      =  "<" *(%x20-3D / %x3F-7E) ">"
                    ; bracketed string of SP and VCHAR
                    ;  without angles
                    ; prose description, to be used as
                    ;  last resort
```
