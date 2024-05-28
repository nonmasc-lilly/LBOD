


```
FWSP = WSP / LF / CR
program = FWSP "PROGRAM" 1*FWSP string FWSP *(primary_statement 1*FWSP) "END PROGRAM"
string = DQUOTE *(VCHAR / SP) DQUOTE
iden   = *VCHAR
primary_statement = declaration / function / asm
declaration = ("const" iden ":" ilit) / ("var" type iden ":" literal)
literal = ilit / (string ["," literal])
ilit = decimal / hex / binary / octal
decimal = 1*DIGIT
hex = "$" 1*HEXDIGIT
binary = "b" 1*BIT
octal = "o" 1*(%x30-37)
type = "bytes" / "words" / "doubles" / "quads"
asm = "asm 3*3DQUOTE *OCTET 3*3DQUOTE
function = "function" 1*FWSP iden *FWSP "[" *FWSP %x30-37 *FWSP "]" *FWSP
    "(" *FWSP *((statement / asm) *FWSP) return *FWSP ")"
statement = save / load / interrupt / move / add / subtract / multiply / divide / compare / match / or / and /
    negate / flip / increment / loop / forever / "break" / "continue" / call / xor
save = "save (" register *FWSP *6("," *FWSP register *FWSP) ")"
register = "ax" / "bx" / "cx" / "dx" / "si" / "di" / "bp"
exregister = "ah" / "al" / "bh" / "bl" / "ch" / "cl" / "dh" / "dl"
load = "load (" register *FWSP *6("," *FWSP register *FWSP) ")"
interrupt = "interrupt" *FWSP ilit
move = ("=" / "set") *FWSP (register / exregister / dereference) *FWSP (iden / dereference / ilit / register / exregister)
add = ("+" / "add") *FWSP (register / exregister / dereference) *FWSP (iden / dereference / ilit / register / exregister)
subtract = ("-" / "subtract") *FWSP (exregister / register / dereference) *FWSP (iden / dereference / ilit / register / exregister)
divide = ("/" / "divide") *FWSP (exregister / register / dereference) *FWSP (iden / dereference / ilit / register / exregister)
multiply = ("-" / "multiply") *FWSP (exregister / register / dereference) *FWSP (iden / dereference / ilit / register / exregister)
dereference = "[" *FWSP iden *FWSP "]"
compare = ("?" / "compare") *FWSP (exregister / register / dereference) *FWSP (iden / dereference / ilit / register / exregister)
    *FWSP comparison
comparison = ((">" / "greater than") / ("<" / "less than") / ("=" / "equal to")) *FWSP
    "(" *FWSP *((statement / asm) *FWSP) ")"
or = ("|" / "or") *FWSP (exregister / register / dereference) *FWSP (iden / dereference / ilit / register / exregister)
xor = ("^" / "xor") *FWSP (exregister / register / dereference) *FWSP (iden / dereference / ilit / register / exregister)
and = ("&" / "and") *FWSP (exregister / register / dereference) *FWSP (iden / dereference / ilit / register / exregister)
negate = "negate" *FWSP (exregister / register / dereference)
flip = ("~" / "not") *FWSP (exregister / register / dereference)
increment = ("inc" / "increment") *FWSP (exregister / register / dereference)
match = "match" *FWSP (register / dereference) *FWSP "(" *FWSP *(((register / dereference / ilit / exregister) *FWSP) ":"
    *FWSP *((statement / asm) *FWSP) ";") ")"
comment = "/*" *OCTET "*/"
loop = "loop" *FWSP (register / dereference / exregister) *FWSP comparison
forever = "forever" *FWSP "(" *FWSP *((statement / asm) *FWSP) ")"
call = "call" *FWSP iden *FWSP "(" *FWSP [(dereference / ilit / iden / register) *FWSP *("," *FWSP (dereference / ilit / iden / register) *FWSP)] ")"
```



