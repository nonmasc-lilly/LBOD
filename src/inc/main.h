#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#ifndef MAIN_H
#define MAIN_H

#define NCMP(a, b) !strcmp((a),(b))
#define IFNCMP(a, b) if(NCMP((a),(b)))
#define ELNCMP(a, b) else IFNCMP((a),(b))
#define MATCHSUBJECT T__tomatch__
#define SETMATCH(s) { const char *MATCHSUBJECT = s;
#define BEGINMATCH(s) IFNCMP(T__tomatch__, s)
#define MATCH(s) ELNCMP(T__tomatch__, s)
#define DEFAULTMATCH() else
#define FINMATCH() }

static const char *token_type_rep[] = {
    "null",
    "string",       "iden",         "program",
    "end",          "const",        "int literal",
    "var",          ":",            "bytes",        /*    :    */
    "words",        "doubles",      "quads",
    "asm",          "asm string",   "function",
    "break",        "continue",     "save",
    "(",            ")",            "ax",           /*  ( )    */
    "bx",           "cx",           "dx",
    "si",           "di",           "bp",
    "ah",           "al",           "bh",
    "bl",           "ch",           "cl",
    "dh",           "dl",           "load",
    "interrupt",    "move",         "add",          /*      +  */
    "subtract",     "divide",       "multiply",     /*  - / *  */
    "compare",      "=",            ">",            /*  ? = >  */
    "<",            "or",           "and",          /*  < | &  */
    "negate",       "not",          "increment",    /*    ~    */
    "match",        ";",            "loop",         /*    ;    */
    "forever",      "[",            "]",            /*    [ ]  */
    "than",         "to",           ",",            /*      ,  */
    "xor"                                           /*  ^*/
};

enum token_type {
    TT_null,
    TT_string,      TT_iden,        TT_program,
    TT_end,         TT_const,       TT_ilit,
    TT_var,         TT_colon,       TT_bytes,
    TT_words,       TT_doubles,     TT_quads,
    TT_asm,         TT_asm_string,  TT_function,
    TT_break,       TT_continue,    TT_save,
    TT_oparen,      TT_cparen,      TT_ax,
    TT_bx,          TT_cx,          TT_dx,
    TT_si,          TT_di,          TT_bp,
    TT_ah,          TT_al,          TT_bh,
    TT_bl,          TT_ch,          TT_cl,
    TT_dh,          TT_dl,          TT_load,
    TT_interrupt,   TT_move,        TT_add,
    TT_sub,         TT_div,         TT_mul,
    TT_compare,     TT_equal,       TT_greater,
    TT_less,        TT_or,          TT_and,
    TT_negate,      TT_flip,        TT_increment,
    TT_match,       TT_semicolon,   TT_loop,
    TT_forever,     TT_obracket,    TT_cbracket,
    TT_than,        TT_to,          TT_comma,
    TT_xor
};

enum value_type {
    VT_null,        VT_int,         VT_string
};

union value {
    unsigned int u;
    char *s;
};

struct lex_node {
    enum token_type type;
    union value value;
    unsigned int line, ch;
    struct lex_node *prev, *next;
};

bool FWSP(char c);

void represent_tokens(struct lex_node *root, unsigned int i);
enum value_type token_value_type(enum token_type type);
struct lex_node *create_lex_node(enum token_type type, void *value);
void destroy_lex_node(struct lex_node *node);
void lex_node_add_child(struct lex_node *parent, struct lex_node *child);

enum token_type token_from_string(const char *str);
enum token_type optoken_from_stroff(const char *str, unsigned int *offset);
void *alloc_token_value_from_string(const char *str);

struct lex_node *lex_string(const char *string);














#endif
