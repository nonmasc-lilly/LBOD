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
    "xor",          "return",       "call",         /*  ^      */
    "default",      "TT_lbl",       "branch"        /*    @    */
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
    TT_xor,         TT_return,      TT_call,
    TT_default,     TT_lbl,         TT_branch
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


enum error_level {
    EL_info, EL_warning, EL_error
};

static const char *parse_class_rep[] = {
    "program",              "string",       "identifier",
    "primary statement",    "declaration",  "literal",
    "int literal",          "type",         "asm",
    "function",             "statement",    "save",
    "register",             "exregister",   "load",
    "interrupt",            "move",         "add",
    "subtract",             "divide",       "multiply",
    "dereference",          "compare",      "comparison",
    "or",                   "xor",          "and",
    "negate",               "flip",         "increment",
    "match",                "loop",         "forever",
    "call",                 "return",       "end",
    "break",                "continue",     "default",
    "lbl",                  "branch"
};

enum parse_class {
    PC_program,             PC_string,      PC_iden,
    PC_primary_statement,   PC_declaration, PC_literal,
    PC_ilit,                PC_type,        PC_asm,
    PC_function,            PC_statement,   PC_save,
    PC_register,            PC_exregister,  PC_load,
    PC_interrupt,           PC_move,        PC_add,
    PC_subtract,            PC_divide,      PC_multiply,
    PC_dereference,         PC_compare,     PC_comparison,
    PC_or,                  PC_xor,         PC_and,
    PC_negate,              PC_flip,        PC_increment,
    PC_match,               PC_loop,        PC_forever,
    PC_call,                PC_return,      PC_end,
    PC_break,               PC_continue,    PC_default,
    PC_lbl,                 PC_branch
};

struct parse_node {
    unsigned int pid;
    enum parse_class type;
    unsigned int children_len;
    union value value;
    struct parse_node *parent, **children;
};

#define THROW(str, token) do {\
        report(EL_error, (str));\
        if(token) printf("at:\n\tline: %u\n\tbyte: %u\n", ((struct lex_node*)(token))->line,\
            ((struct lex_node*)(token))->ch);\
        exit(0);\
    } while(0)
#define ASSERT(condition, str, token) if(!(condition)) THROW((str), (token))

void report(enum error_level level, const char *string);
enum value_type parse_class_value_type(enum parse_class type);
struct parse_node *create_parse_node(enum parse_class type, const void *value);
void destroy_parse_node(struct parse_node *parse_node);
void parse_node_add_child(struct parse_node *parent, struct parse_node *child);
void represent_parse_node(struct parse_node *root, unsigned int offset);

/*TODO: Name check and other various state problems lol*/

struct parse_node *parse_string(struct lex_node **current);      /* done {value}                         */
struct parse_node *parse_iden(struct lex_node **current);        /* done {value}                         */
struct parse_node *parse_int_literal(struct lex_node **current); /* done {value}                         */
struct parse_node *parse_literal(struct lex_node **current);     /* done {str / ilit, lit}               */
struct parse_node *parse_dereference(struct lex_node **current); /* done {iden / register}               */
struct parse_node *parse_type(struct lex_node **current);        /* done {value}                         */
struct parse_node *parse_register(struct lex_node **current);    /* done {value}                         */
struct parse_node *parse_exregister(struct lex_node **current);  /* done {value}                         */
struct parse_node *parse_save(struct lex_node **current);        /* done {*register}                     */
struct parse_node *parse_load(struct lex_node **current);        /* done {*register}                     */
struct parse_node *parse_interrupt(struct lex_node **current);   /* done {ilit}                          */
struct parse_node *parse_move(struct lex_node **current);        /* done {eb, rb}                        */
struct parse_node *parse_add(struct lex_node **current);         /* done {eb, rb}                        */
struct parse_node *parse_subtract(struct lex_node **current);    /* done {eb, rb}                        */
struct parse_node *parse_multiply(struct lex_node **current);    /* done {eb, rb}                        */
struct parse_node *parse_divide(struct lex_node **current);      /* done {eb, rb}                        */
struct parse_node *parse_or(struct lex_node **current);          /* done {eb, rb}                        */
struct parse_node *parse_xor(struct lex_node **current);         /* done {eb, rb}                        */
struct parse_node *parse_and(struct lex_node **current);         /* done {eb, rb}                        */
struct parse_node *parse_negate(struct lex_node **current);      /* done {eb}                            */
struct parse_node *parse_flip(struct lex_node **current);        /* done {eb}                            */
struct parse_node *parse_increment(struct lex_node **current);   /* done {eb}                            */
struct parse_node *parse_return(struct lex_node **current);      /* done {}                              */
struct parse_node *parse_break(struct lex_node **current);       /* done {}                              */
struct parse_node *parse_continue(struct lex_node **current);    /* done {}                              */
struct parse_node *parse_call(struct lex_node **current);        /* done {iden, *rb}                     */
struct parse_node *parse_match(struct lex_node **current);       /* done {reg/deref, *(eb, *state, end)} */
struct parse_node *parse_comparison(struct lex_node **current);  /* done {value, reg/deref/ilit/ereg, *statebn}*/
struct parse_node *parse_compare(struct lex_node **current);     /* done {reg/deref, comparison}         */
struct parse_node *parse_loop(struct lex_node **current);        /* done {reg/deref, comparison}         */
struct parse_node *parse_label(struct lex_node **current);       /* done {iden}                          */
struct parse_node *parse_branch(struct lex_node **current);      /* done {iden}                          */
struct parse_node *parse_forever(struct lex_node **current);     /* done {*state}                        */
struct parse_node *parse_statement(struct lex_node **current);   /*      */
struct parse_node *parse_asm(struct lex_node **current);                /* done {value}                  */
struct parse_node *parse_function(struct lex_node **current);           /* done {iden, ilit, *state}     */
struct parse_node *parse_declaration(struct lex_node **current);        /* done {value, {type,iden,lit}  */
                                                                        /*    / {iden, lit}              */
struct parse_node *parse_primary_statement(struct lex_node **current);  /* done <asm / func / decl>      */
struct parse_node *parse_program(struct lex_node *tokens);              /* done {*primstate}             */

enum state_control_type { SC_get, SC_set, SC_increment, SC_get_current, SC_set_current,
    SC_get_curl, SC_set_curl };
enum state_argument { SC_null, SC_forever, SC_loop, SC_comparison };

unsigned int state_control(enum state_control_type cmd, enum state_argument arg, unsigned int var);

char *compile_string(struct parse_node *root);
char *compile_iden(struct parse_node *root);
char *compile_int_literal(struct parse_node *root);
char *compile_literal(struct parse_node *root);
char *compile_dereference(struct parse_node *root);
char *compile_register(struct parse_node *root);
char *compile_exregister(struct parse_node *root);
char *compile_save(struct parse_node *root);
char *compile_load(struct parse_node *root);
char *compile_interrupt(struct parse_node *root);
char *compile_move(struct parse_node *root);
char *compile_add(struct parse_node *root);
char *compile_subtract(struct parse_node *root);
char *compile_multiply(struct parse_node *root);
char *compile_divide(struct parse_node *root);
char *compile_or(struct parse_node *root);
char *compile_xor(struct parse_node *root);
char *compile_and(struct parse_node *root);
char *compile_negate(struct parse_node *root);
char *compile_flip(struct parse_node *root);
char *compile_increment(struct parse_node *root);
char *compile_return(struct parse_node *root);
char *compile_break(struct parse_node *root);
char *compile_continue(struct parse_node *root);
char *compile_call(struct parse_node *root);
char *compile_match(struct parse_node *root);
char *compile_compare(struct parse_node *root);
char *compile_loop(struct parse_node *root);
char *compile_label(struct parse_node *root);
char *compile_branch(struct parse_node *root);
char *compile_forever(struct parse_node *root);
char *compile_statement(struct parse_node *root);
char *compile_declaration(struct parse_node *root);
char *compile_function(struct parse_node *root);
char *compile_asm(struct parse_node *root);
char *compile_primary_statement(struct parse_node *root);
char *compile_program(struct parse_node *root);



#endif
