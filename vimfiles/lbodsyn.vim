
syntax keyword lbodmeta PROGRAM END
syntax keyword lbodkey var const asm function match compare loop forever
syntax keyword lbodtypes bytes words doubles quads
syntax keyword lbodstate break continue save load interrupt move add subtract divide multiply compare equal to greater than less or and negate not increment xor return call inc int default branch
syntax keyword lbodreg ax bx cx dx si di bp al ah bl bh cl ch dl dh
syntax region lbodlabel start=/@\|\(label \)\|\(lbl \)/ end=/:/

syntax match lbodconstint /\(\(\(\%[o]\|\%[b]\)\d\+\)\|\$\(\(\([A-F]\|[a-f]\)\|\d\)\+\)\)/
syntax region lbodasmstring start=/"""/ end=/"""/
syntax region lbodstring start=/"/ end=/"/
syntax region lbodcomment start=/\/\*/ end=/\*\//

highlight def link lbodcomment Comment
highlight def link lbodasmstring Constant
highlight def link lbodstring Constant
highlight def link lbodconstint Constant
highlight def link lbodreg Identifier
highlight def link lbodstate Statement
highlight def link lbodkey Type
highlight def link lbodtypes Identifier
highlight def link lbodmeta Type
highlight def link lbodlabel Statement
