#include "inc/main.h"

enum value_type token_value_type(enum token_type type) {
    switch(type) {
    case TT_string:
    case TT_iden:
    case TT_asm_string: return VT_string;
    case TT_ilit: return VT_int;
    default: return VT_null;
    }
}

struct lex_node *create_lex_node(enum token_type type, void *value) {
    struct lex_node *ret;
    ret = calloc(1,sizeof(struct lex_node));
    ret->type = type;
    if(value) switch(token_value_type(type)) {
    case VT_int: ret->value.u = *(unsigned int *)value; break;
    case VT_string:
        ret->value.s = malloc(strlen(*(const char**)value)+1);
        strcpy(ret->value.s, *(const char**)value);
        break;
    }
    return ret;
}
void destroy_lex_node(struct lex_node *node) {
    if(token_value_type(node->type) == VT_string) free(node->value.s);
    if(node->prev) node->prev->next = NULL;
    if(node->next) destroy_lex_node(node->next);
    free(node);
}

void lex_node_add_child(struct lex_node *parent, struct lex_node *child) {
    child->prev = parent;
    parent->next = child;
}

bool FWSP(char c) {
    switch(c) {
    case ' ':
    case '\n':
    case '\t':
    case '\r': return true;
    default: return false;
    }
}

void represent_tokens(struct lex_node *root, unsigned int i) {
    char *v;
    if(i == 0) {
        printf("%5s | (%5s :: %15s) | %15s | %12s\n",
            "tnum", "line", "byte", "type", "value");
    }
    switch(token_value_type(root->type)) {
    case VT_string:
        v = malloc(strlen(root->value.s)+1);
        strcpy(v, root->value.s);
        break;
    case VT_int:
        v = calloc(1,13);
        snprintf(v, 12, "$%x", root->value.u);
        break;
    default:
        v = calloc(1,4);
        strcpy(v, "N/A");
        break;
    }
    printf("%5d | (%5u :: %15u) | %15s | %12s\n", i, root->line, root->ch,
        token_type_rep[root->type], v);
    free(v);
    if(root->next != NULL) represent_tokens(root->next, i+1);
}

enum token_type token_from_string(const char *str) {
    char *eptr;
    bool d;
    d = false;
    SETMATCH(str)
        BEGINMATCH("")      return TT_null;
        MATCH("PROGRAM")    return TT_program;
        MATCH("END")        return TT_end;
        MATCH("const")      return TT_const;
        MATCH("var")        return TT_var;
        MATCH("bytes")      return TT_bytes;
        MATCH("words")      return TT_bytes;
        MATCH("doubles")    return TT_bytes;
        MATCH("quads")      return TT_quads;
        MATCH("asm")        return TT_asm;
        MATCH("function")   return TT_function;
        MATCH("break")      return TT_break;
        MATCH("continue")   return TT_continue;
        MATCH("save")       return TT_save;
        MATCH("load")       return TT_load;
        MATCH("ax")         return TT_ax;
        MATCH("bx")         return TT_bx;
        MATCH("cx")         return TT_cx;
        MATCH("dx")         return TT_dx;
        MATCH("si")         return TT_si;
        MATCH("di")         return TT_di;
        MATCH("bp")         return TT_bp;
        MATCH("ah")         return TT_ah;
        MATCH("al")         return TT_al;
        MATCH("bh")         return TT_bh;
        MATCH("bl")         return TT_bl;
        MATCH("ch")         return TT_ch;
        MATCH("cl")         return TT_cl;
        MATCH("dh")         return TT_dh;
        MATCH("dl")         return TT_dl;
        MATCH("int")        return TT_interrupt;
        MATCH("interrupt")  return TT_interrupt;
        MATCH("move")       return TT_move;
        MATCH("add")        return TT_add;
        MATCH("subtract")   return TT_sub;
        MATCH("multiply")   return TT_mul;
        MATCH("divide")     return TT_div;
        MATCH("compare")    return TT_compare;
        MATCH("equal")      return TT_equal;
        MATCH("to")         return TT_to;
        MATCH("than")       return TT_than;
        MATCH("greater")    return TT_greater;
        MATCH("less")       return TT_less;
        MATCH("or")         return TT_or;
        MATCH("and")        return TT_and;
        MATCH("negate")     return TT_negate;
        MATCH("not")        return TT_flip;
        MATCH("xor")        return TT_xor;
        MATCH("inc")        return TT_increment;
        MATCH("increment")  return TT_increment;
        MATCH("loop")       return TT_loop;
        MATCH("forever")    return TT_forever;
        MATCH("return")     return TT_return;
        MATCH("call")       return TT_call;
        MATCH("match")      return TT_match;
        DEFAULTMATCH() {
            switch(*MATCHSUBJECT) {
            case '$': strtol(MATCHSUBJECT+1, &eptr, 16);           goto check;
            case 'o': strtol(MATCHSUBJECT+1, &eptr,  8);           goto check;
            case 'b': strtol(MATCHSUBJECT+1, &eptr,  2);           goto check;
            default:  strtol(MATCHSUBJECT,   &eptr, 10); d = true; goto check;
            }
        check:
            if(     *(MATCHSUBJECT+1) && !(*eptr)) return TT_ilit;
            if(d && *(MATCHSUBJECT  ) && !(*eptr)) return TT_ilit;
            return TT_iden;
        }
    FINMATCH()
}

enum token_type optoken_from_stroff(const char *string, unsigned int *offset) {
    switch(string[*offset]) {
    case ':':   return TT_colon;
    case '(':   return TT_oparen;
    case ')':   return TT_cparen;
    case '+':   return TT_add;
    case '-':   return TT_sub;
    case '*':   return TT_mul;
    case '/':   return TT_div;
    case '?':   return TT_compare;
    case '=':   return TT_equal;
    case '>':   return TT_greater;
    case '<':   return TT_less;
    case '|':   return TT_or;
    case '&':   return TT_and;
    case '~':   return TT_flip;
    case ';':   return TT_semicolon;
    case '[':   return TT_obracket;
    case ']':   return TT_cbracket;
    case ',':   return TT_comma;
    case '^':   return TT_xor;
    default: return TT_null;
    }
}

void *alloc_token_value_from_string(const char *str) {
    enum token_type type;
    enum value_type vtype;
    void *ret;
    char *eptr;
    type = token_from_string(str);
    vtype = token_value_type(type);
    switch(vtype) {
    case VT_int:
        ret = malloc(sizeof(unsigned int));
        switch(*str) {
        case '$': *(unsigned int*)ret = strtol(str+1, &eptr, 16); break;
        case 'o': *(unsigned int*)ret = strtol(str+1, &eptr,  8); break;
        case 'b': *(unsigned int*)ret = strtol(str+1, &eptr,  2); break;
        default:  *(unsigned int*)ret = strtol(str  , &eptr, 10); break;
        }
        break;
    case VT_string:
        ret = malloc(sizeof(const char *));
        *(const char **)ret = str;
        break;
    default: ret = calloc(1,sizeof(void*));
    }
    return ret;
}

struct lex_node *lex_string(const char *string) {
    struct lex_node *root, *current;
    unsigned int i, blen, line;
    enum token_type temp_type;
    char *buf;
    void *temp_value;
    root = create_lex_node(TT_null, NULL);
    current = root;
    buf = calloc(1,1);
    blen = 0;
    line = 1;
    for(i=0; string[i]; i++) {
        switch(string[i]) {
        case '/':
            if(string[i+1] == '*') for(++i; string[i-1] != '*' || string[i] != '/'; ++i) {
                if(string[i] == '\n') ++line;
            }
            continue;
        case '"':
            temp_type = token_from_string(buf);
            temp_value = alloc_token_value_from_string(buf); 
            if(temp_type) {
                lex_node_add_child(current, create_lex_node(temp_type, temp_value));
                current = current->next;
                current->line = line;
                current->ch = i;
            }
            free(temp_value);
            buf = realloc(buf, 1);
            *buf = blen = 0;
            if(string[i+1] == '"' && string[i+2] == '"') {
                for(i+=3; string[i]   != '"' ||
                          string[i+1] != '"' ||
                          string[i+2] != '"'; i++) {
                    if(string[i] == '\n') ++line;
                    buf = realloc(buf, ++blen + 1);
                    buf[blen-1] = string[i];
                    buf[blen] = 0;
                }
                lex_node_add_child(current, create_lex_node(TT_asm_string, &buf));
                current = current->next;
                current->line = line;
                current->ch = i+=2;
            } else {
                for(++i; string[i] != '"'; i++) {
                    if(string[i] == '\n') ++line;
                    buf = realloc(buf, ++blen + 1);
                    buf[blen-1] = string[i];
                    buf[blen] = 0;
                }
                lex_node_add_child(current, create_lex_node(TT_string, &buf));
                current = current->next;
                current->line = line;
                current->ch = i;
            }
            buf = realloc(buf, 1);
            *buf = blen = 0;
            continue;
        case '\n': ++line; case ' ': case '\t': case '\r':
            temp_type = token_from_string(buf);
            temp_value = alloc_token_value_from_string(buf);
            if(temp_type) {
                lex_node_add_child(current, create_lex_node(temp_type, temp_value));
                current = current->next;
                current->line = line;
                current->ch = i;
            }
            free(temp_value);
            buf = realloc(buf, 1);
            *buf = blen = 0;
            continue;
        default:
            if(optoken_from_stroff(string, &i)) {
                temp_type = token_from_string(buf);
                temp_value = alloc_token_value_from_string(buf);
                if(temp_type) {
                    lex_node_add_child(current, create_lex_node(temp_type, temp_value));
                    current = current->next;
                    current->line = line;
                    current->ch = i;
                }
                free(temp_value);
                buf = realloc(buf,1);
                *buf = blen = 0;
                lex_node_add_child(current, create_lex_node(
                    optoken_from_stroff(string, &i),
                    NULL
                ));
                current = current->next;
                current->line = line;
                current->ch = i;
                continue;
            }
            break;
        }
        buf = realloc(buf, ++blen + 1);
        buf[blen-1] = string[i];
        buf[blen] = 0;
    }
    return root;
}





