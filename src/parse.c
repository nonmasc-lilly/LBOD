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
    case PC_comparison:
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
        ret->value.s = calloc(1,strlen((const char*)value)+1);
        strcpy(ret->value.s, (const char *)value);
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

void represent_parse_node(struct parse_node *root, unsigned int off) {
    unsigned int i;
    char *v;
    switch(parse_class_value_type(root->type)) {
    case VT_string:
        v = calloc(1, strlen(root->value.s)+1);
        strcpy(v, root->value.s);
        break;
    case VT_int:
        v = calloc(1, 13);
        snprintf(v, 12, "$%x", root->value.u);
        break;
    default: v = calloc(1,1);
    }
    for(i = 0; i < off; i++) printf("|");
    printf("> %-15s | %-15s\n", parse_class_rep[root->type], v);
    for(i = 0; i < root->children_len; i++)
        represent_parse_node(root->children[i], off+1);
    free(v);
}


struct parse_node *parse_string(struct lex_node **current) {
    struct parse_node *ret;
    if((*current)->type != TT_string) return NULL;
    ret = create_parse_node(PC_string, (*current)->value.s);
    (*current) = (*current)->next;
    return ret;
}
struct parse_node *parse_iden(struct lex_node **current) {
    struct parse_node *ret;
    if((*current)->type != TT_iden) return NULL;
    ret = create_parse_node(PC_iden, (*current)->value.s);
    (*current) = (*current)->next;
    return ret;
}
struct parse_node *parse_int_literal(struct lex_node **current) {
    struct parse_node *ret;
    if((*current)->type != TT_ilit) return NULL;
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
        parse_node_add_child(ret, temp);
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
    (*current) = (*current)->next;
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
    (*current) = (*current)->next;
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
    (*current) = (*current)->next;
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
    (*current) = (*current)->next;
    return ret;
}
struct parse_node *parse_save(struct lex_node **current) {
    struct parse_node *ret, *temp;
    if((*current)->type != TT_save) return NULL;
    (*current) = (*current)->next;
    ret = create_parse_node(PC_save, NULL);
    ASSERT(
        (*current)->type == TT_oparen,
        "Expected '(' in save statement.",
        *current
    );
    (*current) = (*current)->next;
    temp = parse_register(current);
    ASSERT(
        temp,
        "Expected register in save statement.",
        *current
    );
    parse_node_add_child(ret, temp);
    while((*current)->type == TT_comma) {
        (*current) = (*current)->next;
        temp = parse_register(current);
        ASSERT(
            temp,
            "Misplaced comma in save statement.",
            *current
        );
        parse_node_add_child(ret, temp);
    }
    (*current) = (*current)->next;
    return ret;
}
struct parse_node *parse_load(struct lex_node **current) {
    struct parse_node *ret, *temp;
    if((*current)->type != TT_load) return NULL;
    (*current) = (*current)->next;
    ret = create_parse_node(PC_load, NULL);
    ASSERT(
        (*current)->type == TT_oparen,
        "Expected '(' in load statement.",
        *current
    );
    (*current) = (*current)->next;
    temp = parse_register(current);
    ASSERT(
        temp,
        "Expected register in load statement.",
        *current
    );
    parse_node_add_child(ret, temp);
    while((*current)->type == TT_comma) {
        (*current) = (*current)->next;
        temp = parse_register(current);
        ASSERT(
            temp,
            "Misplaced comma in load statement.",
            *current
        );
        parse_node_add_child(ret, temp);
    }
    (*current) = (*current)->next;
    return ret;
}
struct parse_node *parse_interrupt(struct lex_node **current) {
    struct parse_node *ret, *temp;
    if((*current)->type != TT_interrupt) return NULL;
    (*current) = (*current)->next;
    ret = create_parse_node(PC_interrupt, NULL);
    temp = parse_int_literal(current);
    ASSERT(
        temp,
        "Expected int literal for interrupt statement.",
        *current
    );
    parse_node_add_child(ret, temp);
    return ret;
}
struct parse_node *parse_move(struct lex_node **current) {
    struct parse_node *ret, *temp;
    enum parse_class t1, t2;
    if((*current)->type != TT_move && (*current)->type != TT_equal) return NULL;
    (*current) = (*current)->next;
    ret = create_parse_node(PC_move, NULL);
    temp = parse_register(current);
    if(!temp) temp = parse_exregister(current);
    if(!temp) temp = parse_dereference(current);
    ASSERT(
        temp,
        "Expected register, exregister or dereference in move statement.",
        *current
    );
    t1 = temp->type;
    parse_node_add_child(ret, temp);
    temp = parse_iden(current);
    if(!temp) temp = parse_int_literal(current);
    if(!temp) temp = parse_dereference(current);
    if(!temp) temp =    parse_register(current);
    if(!temp) temp =  parse_exregister(current);
    ASSERT(
        temp,
        "Expected register, exregister, dereference, int literal, or identifier in movestatement.",
        *current
    );
    t2 = temp->type;
    parse_node_add_child(ret, temp);
    ASSERT(
        !(t1 == PC_dereference && t2 == PC_dereference),
        "Cannot move dereference into dereference.",
        *current
    );
    ASSERT(
        !(t1 == PC_register && t2 == PC_exregister) && !(t1 == PC_exregister && t2 == PC_register),
        "Cannot move between register and exregister. Size mismatch.",
        *current
    );
    ASSERT(
        !(t1 == PC_exregister && t2 == PC_ilit && temp->value.u > 0xFF),
        "Cannot move integer literal greater than $FF into exregister. Size mismatch.",
        *current
    );
    return ret;
}
struct parse_node *parse_add(struct lex_node **current) {
    struct parse_node *ret, *temp;
    enum parse_class t1, t2;
    if((*current)->type != TT_add) return NULL;
    (*current) = (*current)->next;
    ret = create_parse_node(PC_add, NULL);
    temp = parse_register(current);
    if(!temp) temp = parse_exregister(current);
    if(!temp) temp = parse_dereference(current);
    ASSERT(
        temp,
        "Expected register, exregister or dereference in add statement.",
        *current
    );
    t1 = temp->type;
    parse_node_add_child(ret, temp);
    temp = parse_iden(current);
    if(!temp) temp = parse_int_literal(current);
    if(!temp) temp = parse_dereference(current);
    if(!temp) temp =    parse_register(current);
    if(!temp) temp =  parse_exregister(current);
    ASSERT(
        temp,
        "Expected register, exregister, dereference, int literal, or identifier in add statement.",
        *current
    );
    t2 = temp->type;
    parse_node_add_child(ret, temp);
    ASSERT(
        !(t1 == PC_dereference && t2 == PC_dereference),
        "Cannot add dereference to dereference.",
        *current
    );
    ASSERT(
        !(t1 == PC_register && t2 == PC_exregister) && !(t1 == PC_exregister && t2 == PC_register),
        "Cannot add between register and exregister. Size mismatch.",
        *current
    );
    ASSERT(
        !(t1 == PC_exregister && t2 == PC_ilit && temp->value.u > 0xFF),
        "Cannot add integer literal greater than $FF to exregister. Size mismatch.",
        *current
    );
    return ret;
}
struct parse_node *parse_subtract(struct lex_node **current) {
    struct parse_node *ret, *temp;
    enum parse_class t1, t2;
    if((*current)->type != TT_sub) return NULL;
    (*current) = (*current)->next;
    ret = create_parse_node(PC_subtract, NULL);
    temp = parse_register(current);
    if(!temp) temp = parse_exregister(current);
    if(!temp) temp = parse_dereference(current);
    ASSERT(
        temp,
        "Expected register, exregister or dereference in subtract statement.",
        *current
    );
    t1 = temp->type;
    parse_node_add_child(ret, temp);
    temp = parse_iden(current);
    if(!temp) temp = parse_int_literal(current);
    if(!temp) temp = parse_dereference(current);
    if(!temp) temp =    parse_register(current);
    if(!temp) temp =  parse_exregister(current);
    ASSERT(
        temp,
        "Expected register, exregister, dereference, int literal, or identifier in subtract statement.",
        *current
    );
    t2 = temp->type;
    parse_node_add_child(ret, temp);
    ASSERT(
        !(t1 == PC_dereference && t2 == PC_dereference),
        "Cannot subtract dereference from dereference.",
        *current
    );
    ASSERT(
        !(t1 == PC_register && t2 == PC_exregister) && !(t1 == PC_exregister && t2 == PC_register),
        "Cannot subtract between register and exregister. Size mismatch.",
        *current
    );
    ASSERT(
        !(t1 == PC_exregister && t2 == PC_ilit && temp->value.u > 0xFF),
        "Cannot subtract integer literal greater than $FF from exregister. Size mismatch.",
        *current
    );
    return ret;
}
struct parse_node *parse_multiply(struct lex_node **current) {
    struct parse_node *ret, *temp;
    enum parse_class t1, t2;
    if((*current)->type != TT_mul) return NULL;
    (*current) = (*current)->next;
    ret = create_parse_node(PC_multiply, NULL);
    temp = parse_register(current);
    if(temp) ASSERT(
        !strcmp(temp->value.s, "ax") || !strcmp(temp->value.s, "dx"),
        "Cannot pass ax or dx as arfuments to multiply",
        *current
    );
    if(!temp) {
        temp = parse_exregister(current);
        if(temp) ASSERT(
            !strcmp(temp->value.s, "ah") || !strcmp(temp->value.s, "al") ||
                !strcmp(temp->value.s, "dh") || !strcmp(temp->value.s, "dl"),
            "Cannot pass ax or dx as arfuments to multiply",
            *current
        );
    }
    if(!temp) temp = parse_dereference(current);
    ASSERT(
        temp,
        "Expected register, exregister or dereference in multiply statement.",
        *current
    );
    t1 = temp->type;
    parse_node_add_child(ret, temp);
    temp = parse_iden(current);
    if(!temp) temp = parse_int_literal(current);
    if(!temp) temp = parse_dereference(current);
    if(!temp) {
        temp =    parse_register(current);
        if(temp) ASSERT(
            !strcmp(temp->value.s, "ax") || !strcmp(temp->value.s, "dx"),
            "Cannot pass ax or dx as arfuments to multiply",
            *current
        );
    }
    if(!temp) {
        temp =  parse_exregister(current);
        if(temp) ASSERT(
            !strcmp(temp->value.s, "ah") || !strcmp(temp->value.s, "al") ||
                !strcmp(temp->value.s, "dh") || !strcmp(temp->value.s, "dl"),
            "Cannot pass ax or dx as arfuments to multiply",
            *current
        );
    }
    ASSERT(
        temp,
        "Expected register, exregister, dereference, int literal, or identifier in multiply statement.",
        *current
    );
    t2 = temp->type;
    parse_node_add_child(ret, temp);
    ASSERT(
        !(t1 == PC_dereference && t2 == PC_dereference),
        "Cannot multiply dereference with dereference.",
        *current
    );
    ASSERT(
        !(t1 == PC_register && t2 == PC_exregister) && !(t1 == PC_exregister && t2 == PC_register),
        "Cannot multiply between register and exregister. Size mismatch.",
        *current
    );
    ASSERT(
        !(t1 == PC_exregister && t2 == PC_ilit && temp->value.u > 0xFF),
        "Cannot multiply integer literal greater than $FF with exregister. Size mismatch.",
        *current
    );
    return ret;
}
struct parse_node *parse_divide(struct lex_node **current) {
    struct parse_node *ret, *temp;
    enum parse_class t1, t2;
    if((*current)->type != TT_div) return NULL;
    (*current) = (*current)->next;
    ret = create_parse_node(PC_divide, NULL);
    temp = parse_register(current);
    if(temp) ASSERT(
        !strcmp(temp->value.s, "ax") || !strcmp(temp->value.s, "dx"),
        "Cannot pass ax or dx as arfuments to divide",
        *current
    );
    if(!temp) {
        temp = parse_exregister(current);
        if(temp) ASSERT(
            !strcmp(temp->value.s, "ah") || !strcmp(temp->value.s, "al") ||
                !strcmp(temp->value.s, "dh") || !strcmp(temp->value.s, "dl"),
            "Cannot pass ax or dx as arfuments to divide",
            *current
        );
    }
    if(!temp) temp = parse_dereference(current);
    ASSERT(
        temp,
        "Expected register, exregister or dereference in divide statement.",
        *current
    );
    t1 = temp->type;
    parse_node_add_child(ret, temp);
    temp = parse_iden(current);
    if(!temp) temp = parse_int_literal(current);
    if(!temp) temp = parse_dereference(current);
    if(!temp) {
        temp =    parse_register(current);
        if(temp) ASSERT(
            !strcmp(temp->value.s, "ax") || !strcmp(temp->value.s, "dx"),
            "Cannot pass ax or dx as arfuments to divide",
            *current
        );
    }
    if(!temp) {
        temp =  parse_exregister(current);
        if(temp) ASSERT(
            !strcmp(temp->value.s, "ah") || !strcmp(temp->value.s, "al") ||
                !strcmp(temp->value.s, "dh") || !strcmp(temp->value.s, "dl"),
            "Cannot pass ax or dx as arfuments to divide",
            *current
        );
    }
    ASSERT(
        temp,
        "Expected register, exregister, dereference, int literal, or identifier in divide statement.",
        *current
    );
    t2 = temp->type;
    parse_node_add_child(ret, temp);
    ASSERT(
        !(t1 == PC_dereference && t2 == PC_dereference),
        "Cannot divide dereference with dereference.",
        *current
    );
    ASSERT(
        !(t1 == PC_register && t2 == PC_exregister) && !(t1 == PC_exregister && t2 == PC_register),
        "Cannot divide between register and exregister. Size mismatch.",
        *current
    );
    ASSERT(
        !(t1 == PC_exregister && t2 == PC_ilit && temp->value.u > 0xFF),
        "Cannot divide integer literal greater than $FF with exregister. Size mismatch.",
        *current
    );
    return ret;
}
struct parse_node *parse_or(struct lex_node **current) {
    struct parse_node *ret, *temp;
    enum parse_class t1, t2;
    if((*current)->type != TT_or) return NULL;
    (*current) = (*current)->next;
    ret = create_parse_node(PC_or, NULL);
    temp = parse_register(current);
    if(!temp) temp = parse_exregister(current);
    if(!temp) temp = parse_dereference(current);
    ASSERT(
        temp,
        "Expected register, exregister or dereference in or statement.",
        *current
    );
    t1 = temp->type;
    parse_node_add_child(ret, temp);
    temp = parse_iden(current);
    if(!temp) temp = parse_int_literal(current);
    if(!temp) temp = parse_dereference(current);
    if(!temp) temp =    parse_register(current);
    if(!temp) temp =  parse_exregister(current);
    ASSERT(
        temp,
        "Expected register, exregister, dereference, int literal, or identifier in or statement.",
        *current
    );
    t2 = temp->type;
    parse_node_add_child(ret, temp);
    ASSERT(
        !(t1 == PC_dereference && t2 == PC_dereference),
        "Cannot or dereference with dereference.",
        *current
    );
    ASSERT(
        !(t1 == PC_register && t2 == PC_exregister) && !(t1 == PC_exregister && t2 == PC_register),
        "Cannot or between register and exregister. Size mismatch.",
        *current
    );
    ASSERT(
        !(t1 == PC_exregister && t2 == PC_ilit && temp->value.u > 0xFF),
        "Cannot or integer literal greater than $FF with exregister. Size mismatch.",
        *current
    );
    return ret;
}
struct parse_node *parse_xor(struct lex_node **current) {
    struct parse_node *ret, *temp;
    enum parse_class t1, t2;
    if((*current)->type != TT_xor) return NULL;
    (*current) = (*current)->next;
    ret = create_parse_node(PC_xor, NULL);
    temp = parse_register(current);
    if(!temp) temp = parse_exregister(current);
    if(!temp) temp = parse_dereference(current);
    ASSERT(
        temp,
        "Expected register, exregister or dereference in xor statement.",
        *current
    );
    t1 = temp->type;
    parse_node_add_child(ret, temp);
    temp = parse_iden(current);
    if(!temp) temp = parse_int_literal(current);
    if(!temp) temp = parse_dereference(current);
    if(!temp) temp =    parse_register(current);
    if(!temp) temp =  parse_exregister(current);
    ASSERT(
        temp,
        "Expected register, exregister, dereference, int literal, or identifier in xor statement.",
        *current
    );
    t2 = temp->type;
    parse_node_add_child(ret, temp);
    ASSERT(
        !(t1 == PC_dereference && t2 == PC_dereference),
        "Cannot xor dereference with dereference.",
        *current
    );
    ASSERT(
        !(t1 == PC_register && t2 == PC_exregister) && !(t1 == PC_exregister && t2 == PC_register),
        "Cannot xor between register and exregister. Size mismatch.",
        *current
    );
    ASSERT(
        !(t1 == PC_exregister && t2 == PC_ilit && temp->value.u > 0xFF),
        "Cannot xor integer literal greater than $FF with exregister. Size mismatch.",
        *current
    );
    return ret;
}
struct parse_node *parse_and(struct lex_node **current) {
    struct parse_node *ret, *temp;
    enum parse_class t1, t2;
    if((*current)->type != TT_and) return NULL;
    (*current) = (*current)->next;
    ret = create_parse_node(PC_and, NULL);
    temp = parse_register(current);
    if(!temp) temp = parse_exregister(current);
    if(!temp) temp = parse_dereference(current);
    ASSERT(
        temp,
        "Expected register, exregister or dereference in and statement.",
        *current
    );
    t1 = temp->type;
    parse_node_add_child(ret, temp);
    temp = parse_iden(current);
    if(!temp) temp = parse_int_literal(current);
    if(!temp) temp = parse_dereference(current);
    if(!temp) temp =    parse_register(current);
    if(!temp) temp =  parse_exregister(current);
    ASSERT(
        temp,
        "Expected register, exregister, dereference, int literal, or identifier in and statement.",
        *current
    );
    t2 = temp->type;
    parse_node_add_child(ret, temp);
    ASSERT(
        !(t1 == PC_dereference && t2 == PC_dereference),
        "Cannot and dereference with dereference.",
        *current
    );
    ASSERT(
        !(t1 == PC_register && t2 == PC_exregister) && !(t1 == PC_exregister && t2 == PC_register),
        "Cannot and between register and exregister. Size mismatch.",
        *current
    );
    ASSERT(
        !(t1 == PC_exregister && t2 == PC_ilit && temp->value.u > 0xFF),
        "Cannot and integer literal greater than $FF with exregister. Size mismatch.",
        *current
    );
    return ret;
}
struct parse_node *parse_negate(struct lex_node **current) {
    struct parse_node *ret, *temp;
    if((*current)->type != TT_negate) return NULL;
    (*current) = (*current)->next;
    ret = create_parse_node(PC_negate, NULL);
    temp = parse_register(current);
    if(!temp) temp = parse_exregister(current);
    if(!temp) temp = parse_dereference(current);
    ASSERT(
        temp,
        "Expected register, exregister or dereference in negate statement.",
        *current
    );
    parse_node_add_child(ret, temp);
    return ret;
}
struct parse_node *parse_flip(struct lex_node **current) {
    struct parse_node *ret, *temp;
    if((*current)->type != TT_flip) return NULL;
    (*current) = (*current)->next;
    ret = create_parse_node(PC_flip, NULL);
    temp = parse_register(current);
    if(!temp) temp = parse_exregister(current);
    if(!temp) temp = parse_dereference(current);
    ASSERT(
        temp,
        "Expected register, exregister or dereference in not statement.",
        *current
    );
    parse_node_add_child(ret, temp);
    return ret;
}
struct parse_node *parse_increment(struct lex_node **current) {
    struct parse_node *ret, *temp;
    if((*current)->type != TT_increment) return NULL;
    (*current) = (*current)->next;
    ret = create_parse_node(PC_increment, NULL);
    temp = parse_register(current);
    if(!temp) temp = parse_exregister(current);
    if(!temp) temp = parse_dereference(current);
    ASSERT(
        temp,
        "Expected register, exregister or dereference in increment statement.",
        *current
    );
    parse_node_add_child(ret, temp);
    return ret;
}
struct parse_node *parse_return(struct lex_node **current) {
    struct parse_node *ret;
    if((*current)->type != TT_return) return NULL;
    (*current) = (*current)->next;
    ret = create_parse_node(PC_return, NULL);
    return ret;
}
struct parse_node *parse_break(struct lex_node **current) {
    struct parse_node *ret;
    if((*current)->type != TT_break) return NULL;
    (*current) = (*current)->next;
    ret = create_parse_node(PC_break, NULL);
    return ret;
}
struct parse_node *parse_continue(struct lex_node **current) {
    struct parse_node *ret;
    if((*current)->type != TT_continue) return NULL;
    (*current) = (*current)->next;
    ret = create_parse_node(PC_continue, NULL);
    return ret;
}
struct parse_node *parse_call(struct lex_node **current) {
    struct parse_node *ret, *temp;
    bool v;
    if((*current)->type != TT_call) return NULL;
    (*current) = (*current)->next;
    ret = create_parse_node(PC_call, NULL);
    temp = parse_iden(current);
    ASSERT(
        temp,
        "Expected identifier in call statement",
        *current
    );
    parse_node_add_child(ret, temp);
    ASSERT(
        (*current)->type == TT_oparen,
        "Expected '(' in call statement.",
        *current
    );
    (*current) = (*current)->next;
    v = false;
    do {
        temp = parse_dereference(current);
        if(!temp) temp = parse_int_literal(current);
        if(!temp) temp = parse_iden(current);
        if(!temp) temp = parse_register(current);
        if(temp) parse_node_add_child(ret, temp);
        if(v) {
            ASSERT(
                temp,
                "Misplaced comma in call statement (or possibly unending call list).",
                *current
            );
        }
        v = true;
    } while((*current)->type != TT_cparen);
    (*current) = (*current)->next;
    return ret;
}
struct parse_node *parse_match(struct lex_node **current) {
    struct parse_node *ret, *temp;
    enum parse_class t1, t2;
    if((*current)->type != TT_match) return NULL;
    (*current) = (*current)->next;
    ret = create_parse_node(PC_match, NULL);
    temp = parse_register(current);
    if(!temp) temp = parse_dereference(current);
    if(!temp) temp = parse_exregister(current);
    ASSERT(
        temp,
        "Expected register, exregister, or dereference in match statement.",
        *current
    );
    t1 = temp->type;
    parse_node_add_child(ret, temp);
    ASSERT(
        (*current)->type == TT_oparen,
        "Expected '(' in match statement.",
        *current
    );
    (*current) = (*current)->next;
    while((*current)->type != TT_cparen) {
        temp = parse_register(current);
        if(!temp) temp = parse_dereference(current);
        if(!temp) temp = parse_exregister(current);
        if(!temp) temp = parse_int_literal(current);
        if(!temp) {
            ASSERT(
                (*current)->type == TT_default,
                "Expected register, exregister, dereference, default or int literal in match body.",
                *current
            );
            temp = create_parse_node(PC_default, NULL);
            (*current) = (*current)->next;
        }
        t2 = temp->type;
        parse_node_add_child(ret, temp);
        ASSERT(
            !(t1 == PC_exregister && t2 == PC_register) && !(t1 == PC_register && t2 == PC_exregister),
            "Cannot compare register and exregister. Size mismatch.",
            *current
        );
        ASSERT(
            !(t1 == PC_dereference && t2 == PC_dereference),
            "Cannot compare dereference with dereference.",
            *current
        );
        ASSERT(
            (*current)->type == TT_colon,
            "Missing ':' in match body.",
            *current
        );
        (*current) = (*current)->next;
        while((*current)->type != TT_semicolon) {
            temp = parse_statement(current);
            ASSERT(
                temp,
                "Missing ';' to end match body.",
                *current
            );
            parse_node_add_child(ret, temp);
        }
        parse_node_add_child(ret, create_parse_node(PC_end, NULL));
        (*current) = (*current)->next;
    }
    (*current) = (*current)->next;
    return ret;   
}
struct parse_node *parse_comparison(struct lex_node **current) {
    struct parse_node *ret, *temp;
    enum token_type type, ntype;
    const char *tstr;
    type = (*current)->type;
    ntype = (*current)->next->type;
    if(type != TT_equal && type != TT_greater && type != TT_less) return NULL;
    (*current) = (*current)->next;
    if(     (type == TT_equal && ntype == TT_to)     ||
            (type == TT_greater && ntype == TT_than) ||
            (type == TT_less && ntype == TT_than)
        ) (*current) = (*current)->next;
    switch(type) {
    case TT_equal:      tstr = "equal";     break;
    case TT_greater:    tstr = "greater";   break;
    case TT_less:       tstr = "less";      break;
    }
    ret = create_parse_node(PC_comparison, tstr);
    temp = parse_iden(current);
    if(!temp) temp = parse_dereference(current);
    if(!temp) temp = parse_int_literal(current);
    if(!temp) temp =    parse_register(current);
    if(!temp) temp =  parse_exregister(current);
    ASSERT(
        temp,
        "Expected register, exregister, int literal, dereference, or identifier.",
        *current
    );
    parse_node_add_child(ret, temp);
    ASSERT(
        (*current)->type == TT_oparen,
        "Expected '(' in comparison expression.",
        *current
    );
    (*current) = (*current)->next;
    while((*current)->type != TT_cparen) {
        temp = parse_statement(current);
        if(!temp) temp = parse_asm(current);
        ASSERT(
            temp,
            "Unclosed comparison body.",
            *current
        );
        parse_node_add_child(ret, temp);
    }
    (*current) = (*current)->next;
    return ret;
}
struct parse_node *parse_compare(struct lex_node **current) {
    struct parse_node *ret, *temp;
    enum parse_class t1, t2;
    if((*current)->type != TT_compare) return NULL;
    (*current) = (*current)->next;
    ret = create_parse_node(PC_compare, NULL);
    temp = parse_register(current);
    if(!temp) temp = parse_exregister(current);
    ASSERT(
        temp,
        "Expected register or exregister in compare statement.",
        *current
    );
    t1 = temp->type;
    parse_node_add_child(ret, temp);
    temp = parse_comparison(current);
    ASSERT(
        temp,
        "Expected comparison expression following compare statement.",
        *current
    );
    parse_node_add_child(ret, temp);
    t2 = temp->children[0]->type;
    ASSERT(
        !(t1 == PC_dereference && t2 == PC_dereference),
        "Cannot compare dereference with dereference.",
        *current
    );
    ASSERT(
        !(t1 == PC_register && t2 == PC_exregister) && !(t1 == PC_exregister && t2 == PC_register),
        "Cannot compare register with exregister. Size mismatch.",
        *current
    );
    ASSERT(
        !(t1 == PC_exregister && t2 == PC_ilit && temp->children[0]->value.u > 0xFF),
        "Cannot compare exregister with integer literal greater than $FF. Size mismatch.",
        *current
    );
    return ret;
}
struct parse_node *parse_loop(struct lex_node **current) {
    struct parse_node *ret, *temp;
    enum parse_class t1, t2;
    if((*current)->type != TT_loop) return NULL;
    (*current) = (*current)->next;
    ret = create_parse_node(PC_loop, NULL);
    temp = parse_register(current);
    if(!temp) temp = parse_exregister(current);
    ASSERT(
        temp,
        "Expected register or exregister in loop statement in loop statement.",
        *current
    );
    t1 = temp->type;
    parse_node_add_child(ret, temp);
    temp = parse_comparison(current);
    ASSERT(
        temp,
        "Expected comparison expression following compare statement in loop statement.",
        *current
    );
    parse_node_add_child(ret, temp);
    t2 = temp->children[0]->type;
    ASSERT(
        !(t1 == PC_dereference && t2 == PC_dereference),
        "Cannot compare dereference with dereference in loop statement.",
        *current
    );
    ASSERT(
        !(t1 == PC_register && t2 == PC_exregister) && !(t1 == PC_exregister && t2 == PC_register),
        "Cannot compare register with exregister in loop statement. Size mismatch.",
        *current
    );
    ASSERT(
        !(t1 == PC_exregister && t2 == PC_ilit && temp->children[0]->value.u > 0xFF),
        "Cannot compare exregister with integer literal greater than $FF in loop statement. Size mismatch.",
        *current
    );
    return ret;
}
struct parse_node *parse_forever(struct lex_node **current) {
    struct parse_node *ret, *temp;
    if((*current)->type != TT_forever) return NULL;
    (*current) = (*current)->next;
    ASSERT(
        (*current)->type == TT_oparen,
        "Expected '(' to begin forever statement.",
        *current
    );
    ret = create_parse_node(PC_forever, NULL);
    (*current) = (*current)->next;
    while((*current)->type != TT_cparen) {
        temp = parse_statement(current);
        represent_parse_node(temp, 0);
        ASSERT(
            temp,
            "Unending forever statement.",
            *current
        );
        parse_node_add_child(ret, temp);
    }
    (*current) = (*current)->next;
    return ret;
}

struct parse_node *parse_label(struct lex_node **current) {
    struct parse_node *ret, *temp;
    if((*current)->type != TT_lbl) return NULL;
    printf("Hey\n");
    (*current) = (*current)->next;
    ret = create_parse_node(PC_lbl, NULL);
    temp = parse_iden(current);
    ASSERT(
        temp,
        "Expected iden after label",
        *current
    );
    parse_node_add_child(ret, temp);
    ASSERT(
        (*current)->type == TT_colon,
        "Expected ':' to end label definition",
        *current
    );
    (*current) = (*current)->next;
    return ret;
}

struct parse_node *parse_branch(struct lex_node **current) {
    struct parse_node *ret, *temp;
    if((*current)->type != TT_branch) return NULL;
    (*current) = (*current)->next;
    ret = create_parse_node(PC_branch, NULL);
    temp = parse_iden(current);
    ASSERT(
        temp,
        "Expected iden after branch statement",
        *current
    );
    parse_node_add_child(ret, temp);
    return ret;
}

#define STATEMENT_LEN 24
struct parse_node *parse_statement(struct lex_node **current) {
    unsigned int i;
    struct parse_node *temp;
    struct parse_node *(*choice[STATEMENT_LEN])(struct lex_node **) = {
        parse_save,     parse_load,         parse_interrupt,
        parse_move,     parse_add,          parse_subtract,
        parse_multiply, parse_divide,       parse_or,
        parse_xor,      parse_and,          parse_negate,
        parse_flip,     parse_increment,    parse_return,
        parse_break,    parse_continue,     parse_call,
        parse_match,    parse_compare,      parse_loop,
        parse_forever,  parse_label,        parse_branch
    };
    for(i=0; i<STATEMENT_LEN; i++) if(temp = choice[i](current)) return temp;
    return NULL;
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
    ASSERT(
        temp->value.u < 8,
        "Function argument number must be less than 8.",
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
    struct parse_node *temp;
    struct parse_node *(*options[3]) (struct lex_node **) = {
        parse_declaration, parse_function, parse_asm
    };
    for(i=0; i<3; i++) if(temp = options[i](current)) return temp;
    return NULL;
}

struct parse_node *parse_program(struct lex_node *tokens) {
    struct lex_node *root = tokens;
    struct lex_node *current = tokens->next;
    struct parse_node *program, *temp;
    
    root    = tokens;
    current = tokens->next;
    program = create_parse_node(PC_program, NULL);


    ASSERT(
        current->type == TT_program,
        "Program must begin with program clause.",
         current
    );
    current = current->next;
    ASSERT(
        current->type == TT_string,
        "Incomplete program clause, missing string.",
        current
    );
    parse_node_add_child(program, parse_string(&current));

    while(current->type != TT_end || current->next->type != TT_program) {
        temp = parse_primary_statement(&current);
        ASSERT(temp, "Expected primary statement in program clause.", current);
        parse_node_add_child(program, temp);
        ASSERT(current->next, "Program clause does not end.", NULL);
    }
    return program;
}











