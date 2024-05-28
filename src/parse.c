#include "inc/main.h"

void report(enum error_level level, const char *string) {
    switch(level) {
    default:
    case EL_info:       printf("\033[97;40mINFO: %s\n", string);        break;
    case EL_warning:    printf("\033[33;40mWARNING:\033[96;40m %s\n");  break;
    case EL_error:      printf("\033[91;40mERROR:\033[96;40m %s\n");    break;
    }
}

enum value_type parse_class_value_type(enum parse_class type) {
    switch(type) {
    case PC_string:
    case PC_iden:
    case PC_type:
    case PC_asm:
    case PC_register:
    case PC_declaration:
    case PC_exregister: return VT_string;
    case PC_ilit:       return VT_int;
    default:            return VT_null;
    }
}

struct parse_node *create_parse_node(enum parse_class type, const void *value) {
    struct parse_node *ret;
    ret = calloc(1,sizeof(struct parse_node));
    ret->type = type;
    switch(parse_class_value_type(type)) {
    case VT_string:
        ret->value.s = calloc(1,strlen(*(const char**)value)+1);
        strcpy(ret->value.s, *(const char **)value);
        break;
    case VT_int:
        ret->value.u = *(unsigned int *)value;
        break;
    }
    return ret;
}

void destroy_parse_node(struct parse_node *parse_node) {
    unsigned int i;
    if(parse_class_value_type(parse_node->type) == VT_string)
        free(parse_node->value.s);
    for(i=0; i<parse_node->children_len; i++)
        if(parse_node->children[i]) destroy_parse_node(parse_node->children[i]);
    if(parse_node->parent) parse_node->parent->children[parse_node->pid] = NULL;
    free(parse_node);
}

void parse_node_add_child(struct parse_node *parent, struct parse_node *child) {
    if(child->parent)
        child->parent->children[child->pid] = NULL;
    child->pid = parent->children_len;
    parent->children = realloc(parent->children,
        sizeof(struct parse_node*)*(++parent->children_len));
    parent->children[parent->children_len-1] = child;
}


struct parse_node *parse_string(struct lex_node **current) {
    struct parse_node *ret;
    if((*current)->type != TT_string) return NULL;
    ret = create_parse_node(PC_string, &((*current)->value.s));
    (*current) = (*current)->next;
    return ret;
}
struct parse_node *parse_iden(struct lex_node **current) {
    struct parse_node *ret;
    if((*current)->type != TT_iden) return NULL;
    ret = create_parse_node(PC_iden, &((*current)->value.s));
    (*current) = (*current)->next;
    return ret;
}
struct parse_node *parse_int_literal(struct lex_node **current) {
    struct parse_node *ret;
    if((*current)->type != TT_string) return NULL;
    ret = create_parse_node(PC_ilit, &((*current)->value.u));
    (*current) = (*current)->next;
    return ret;
}
struct parse_node *parse_literal(struct lex_node **current) {
    struct parse_node *ret, *temp;
    temp = parse_int_literal(current);
    if(!temp) temp = parse_string(current);
    if(!temp) return NULL;
    ret = create_parse_node(PC_literal, NULL);
    parse_node_add_child(ret, temp);
    if((*current)->type == TT_comma) {
        (*current) = (*current)->next;
        temp = parse_literal(current);
        ASSERT(
            temp,
            "Misplaced ',' in literal list.",
            *current
        );
    }
    return ret;
}
struct parse_node *parse_dereference(struct lex_node **current) {
    struct parse_node *ret, *temp;
    if((*current)->type != TT_obracket) return NULL;
    (*current) = (*current)->next;
    ret = create_parse_node(PC_dereference, NULL);
    temp = parse_register(current);
    if(!temp) temp = parse_iden(current);
    ASSERT(
        temp,
        "Dereference expression missing subject (register or var identifier).",
        *current
    );
    parse_node_add_child(ret, temp);
    ASSERT(
        (*current)->type == TT_cbracket,
        "Dereference expression unclosed.",
        *current
    );
    return ret;
}
struct parse_node *parse_type(struct lex_node **current) {
    struct parse_node *ret;
    const char *type;
    switch((*current)->type) {
    case TT_bytes:      type = "bytes";     break;
    case TT_words:      type = "words";     break;
    case TT_doubles:    type = "doubles";   break;
    case TT_quads:      type = "quads";     break;
    default: return NULL;
    }
    ret = create_parse_node(PC_type, type);
    return ret;
}
struct parse_node *parse_register(struct lex_node **current) {
    struct parse_node *ret;
    const char *reg;
    switch((*current)->type) {
    case TT_ax: reg = "ax"; break;
    case TT_bx: reg = "bx"; break;
    case TT_cx: reg = "cx"; break;
    case TT_dx: reg = "dx"; break;
    case TT_si: reg = "si"; break;
    case TT_di: reg = "di"; break;
    case TT_bp: reg = "bp"; break;
    default:          return NULL;
    }
    ret = create_parse_node(PC_register, reg);
    return ret;
}
struct parse_node *parse_exregister(struct lex_node **current) {
    struct parse_node *ret;
    const char *ereg;
    switch((*current)->type) {
    case TT_ah: ereg = "ah"; break;
    case TT_al: ereg = "al"; break;
    case TT_bh: ereg = "bh"; break;
    case TT_bl: ereg = "bl"; break;
    case TT_ch: ereg = "ch"; break;
    case TT_cl: ereg = "cl"; break;
    case TT_dh: ereg = "dh"; break;
    case TT_dl: ereg = "dl"; break;
    default:          return NULL;
    }
    ret = create_parse_node(PC_exregister, ereg);
    return ret;
}

struct parse_node *parse_asm(struct lex_node **current) {
    struct parse_node *ret;
    if((*current)->type != TT_asm) return NULL;
    (*current) = (*current)->next;
    ASSERT(
        (*current)->type == TT_asm_string,
        "Expected assembly string in asm primary statement.",
        *current
    );
    ret = create_parse_node(PC_asm, (*current)->value.s);
    (*current) = (*current)->next;
    return ret;
}

struct parse_node *parse_function(struct lex_node **current) {
    struct parse_node *ret, *temp;
    if((*current)->type != TT_function) return NULL;
    ret = create_parse_node(PC_function, NULL);
    (*current) = (*current)->next;
    temp = parse_iden(current);
    ASSERT(
        temp,
        "Expected identifier in function primary statement.",
        *current
    );
    parse_node_add_child(ret, temp);
    ASSERT(
        (*current)->type == TT_obracket,
        "Expected argument number ('[' int literal ']') in function primary statement.",
        *current
    );
    (*current) = (*current)->next;
    temp = parse_int_literal(current);
    ASSERT(
        temp,
        "Expected argument number ('[' int literal ']') in function primary statement.",
        *current
    );
    parse_node_add_child(ret, temp);
    ASSERT(
        (*current)->type == TT_cbracket,
        "Expected argument number ('[' int literal ']') in function primary statement.",
        *current
    );
    (*current) = (*current)->next;
    ASSERT(
        (*current)->type == TT_oparen,
        "Expected '(' in function primary statement.",
        *current
    );
    (*current) = (*current)->next;
    while(1) {
        if((*current)->type == TT_cparen) break;
        temp = parse_statement(current);
        if(!temp) temp = parse_asm(current);
        ASSERT(
            temp,
            "Function primary statement does not end.",
            *current
        );
        parse_node_add_child(ret, temp);
    }
    (*current) = (*current)->next;
    return ret;
}

struct parse_node *parse_declaration(struct lex_node **current) {
    struct parse_node *ret, *temp;
    if((*current)->type != TT_var && (*current)->type != TT_const) return NULL;
    ret = create_parse_node(PC_declaration, (*current)->type == TT_var ? "var" : "const");
    (*current) = (*current)->next;
    if((*current)->prev->type == TT_var) {
        temp = parse_type(current);
        ASSERT(
            temp,
            "Expected type ('bytes' / 'words' / 'doubles' / 'quads') in "
            "var declaration primary statement.",
            *current
        );
        parse_node_add_child(ret, temp);
        temp = parse_iden(current);
        ASSERT(
            temp,
            "Expected identifier in var declaration primary statement.",
            *current
        );
        parse_node_add_child(ret, temp);
        ASSERT(
            (*current)->type == TT_colon,
            "Expected ':' after iden in var declaration primary statement.",
            *current
        );
        (*current) = (*current)->next;
        temp = parse_literal(current);
        ASSERT(
            temp,
            "Expected literal after var declaration primary statement.",
            *current
        );
        parse_node_add_child(ret, temp);
    } else {
        temp = parse_iden(current);
        ASSERT(
            temp,
            "Expected iden in const declaration primary statement.",
            *current
        );
        parse_node_add_child(ret, temp);
        ASSERT(
            (*current)->type == TT_colon,
            "Expected ':' after iden in const declaration primary statement.",
            *current
        );
        (*current) = (*current)->next;
        temp = parse_int_literal(current);
        ASSERT(
            temp,
            "Expected integer literal after const declaration primary statement.",
            *current
        );
        parse_node_add_child(ret, temp);
    }
    return ret;
}

struct parse_node *parse_primary_statement(struct lex_node **current) {
    unsigned int i;
    struct parse_node *options[3] = {
        parse_declaration(current), parse_function(current), parse_asm(current)
    };
    for(i=0; i<3; i++) if(options[i]) return options[i];
    return NULL;
}

struct parse_node *parse_program(struct lex_node *tokens) {
    struct lex_node *root = tokens;
    struct lex_node *current = tokens->next;
    struct parse_node *program, *temp;
    
    root    = tokens;
    current = tokens->next;
    program = create_parse_node(PC_program, NULL);


    if(current->type != TT_program)
        THROW("Program must begin with program clause.",     current);
    current = current->next;
    if(current->next->type != TT_string)
        THROW("Incomplete program clause, missing string.",  current);
    parse_node_add_child(program, parse_string(&current));
    current = current->next;

    while(current->type != TT_end || current->next->type != TT_program) {
        temp = parse_primary_statement(&current);
        ASSERT(temp, "Expected primary statement in program clause.", current);
        parse_node_add_child(program, temp);
        ASSERT(current, "Program clause does not end.", NULL);
    }
    return program;
}











