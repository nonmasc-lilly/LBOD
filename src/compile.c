#include "inc/main.h"



unsigned int state_control(enum state_control_type cmd, enum state_argument arg, unsigned int var) {
    static unsigned int forever_label=0, loop_label=0, comparison_label=0, current=0;
    static unsigned int curlnum=0;
    switch(cmd) {
    case SC_get:
        switch(arg) {
        case SC_forever:    return forever_label;
        case SC_loop:       return loop_label;
        case SC_comparison: return comparison_label;
        default:            return 0;
        }
    case SC_set:
        switch(arg) {
        case SC_forever:    forever_label    = var; return 1;
        case SC_loop:       loop_label       = var; return 1;
        case SC_comparison: comparison_label = var; return 1;
        default:                                    return 0;
        }
    case SC_increment:
        switch(arg) {
        case SC_forever:    ++forever_label;    return 1;
        case SC_loop:       ++loop_label;       return 1;
        case SC_comparison: ++comparison_label; return 1;
        default:                                return 0;
        }
    case SC_set_current:
        current = arg; return 1;
    case SC_get_current: return current;
    case SC_get_curl: return curlnum;
    case SC_set_curl: curlnum = var; return 1;
    }
}

char *compile_string(struct parse_node *root) {
    char *ret;
    if(root->type != PC_string) return NULL;
    ret = malloc(strlen(root->value.s)+3);
    sprintf(ret, "\"%s\"", root->value.s);
    return ret;
}

char *compile_iden(struct parse_node *root) {
    char *ret;
    if(root->type != PC_iden) return NULL;
    ret = malloc(strlen(root->value.s)+1);
    sprintf(ret, "%s", root->value.s);
    return ret;
}

char *compile_int_literal(struct parse_node *root) {
    char *ret;
    if(root->type != PC_ilit) return NULL;
    ret = malloc(20);
    sprintf(ret, "0x%x", root->value.u);
    return ret;
}

char *compile_literal(struct parse_node *root) {
    char *ret, *temp;
    if(root->type != PC_literal) return NULL;
    ret = compile_int_literal(root->children[0]);
    if(!ret) ret = compile_string(root->children[0]);
    if(root->children_len >= 2) {
        temp = compile_literal(root->children[1]);
        ret = realloc(ret, strlen(ret)+strlen(temp)+3);
        strcat(ret, ", ");
        strcat(ret, temp);
    }
    return ret;
}

char *compile_dereference(struct parse_node *root) {
    char *ret, *temp;
    const char *format;
    if(root->type != PC_dereference) return NULL;
    format = "[%s]";
    temp = compile_iden(root->children[0]);
    if(!temp) temp = compile_register(root->children[0]);
    ret = malloc(strlen(format)+strlen(temp)+1);
    sprintf(ret, format, temp);
    free(temp);
    return ret;
}

char *compile_register(struct parse_node *root) {
    char *ret;
    if(root->type != PC_register) return NULL;
    ret = malloc(strlen(root->value.s)+1);
    strcpy(ret, root->value.s);
    return ret;
}

char *compile_exregister(struct parse_node *root) {
    char *ret;
    if(root->type != PC_exregister) return NULL;
    ret = malloc(strlen(root->value.s)+1);
    strcpy(ret, root->value.s);
    return ret;
}

char *compile_save(struct parse_node *root) {
    char *ret, *temp;
    const char *format;
    unsigned int i;
    if(root->type != PC_save) return NULL;
    format = "push ";
    ret = calloc(1,1);
    for(i=0; i<root->children_len; i++) {
        if(!(temp = compile_register(root->children[i]))) continue;
        ret = realloc(ret, strlen(ret)+strlen(format)+strlen(temp)+2);
        strcat(ret, format);
        strcat(ret, temp);
        strcat(ret, "\n");
        free(temp);
    }
    return ret;
}

char *compile_load(struct parse_node *root) {
    char *ret, *temp;
    const char *format;
    unsigned int i;
    if(root->type != PC_load) return NULL;
    format = "pop ";
    ret = calloc(1,1);
    printf("%u\n", root->children_len);
    for(i=root->children_len-1; i!=-1; i--) {
        if(!(temp = compile_register(root->children[i]))) continue;
        ret = realloc(ret, strlen(ret)+strlen(format)+strlen(temp)+2);
        strcat(ret, format);
        strcat(ret, temp);
        strcat(ret, "\n");
        free(temp);
    }
    return ret;
}

char *compile_interrupt(struct parse_node *root) {
    char *ret, *temp;
    const char *format;
    if(root->type != PC_interrupt) return NULL;
    format = "int %s\n";
    temp = compile_int_literal(root->children[0]);
    ret = malloc(strlen(format)+strlen(temp)+1);
    sprintf(ret, format, temp);
    free(temp);
    return ret;
}

char *compile_move(struct parse_node *root) {
    char *ret, *temp[2];
    const char *format;
    if(root->type != PC_move) return NULL;
    format = "mov %s, %s\n";
    temp[0] = compile_register(root->children[0]);
    if(!temp[0]) temp[0] = compile_exregister(root->children[0]);
    if(!temp[0]) temp[0] = compile_dereference(root->children[0]);
    temp[1] = compile_register(root->children[1]);
    if(!temp[1]) temp[1] = compile_exregister(root->children[1]);
    if(!temp[1]) temp[1] = compile_dereference(root->children[1]);
    if(!temp[1]) temp[1] = compile_int_literal(root->children[1]);
    if(!temp[1]) temp[1] = compile_iden(root->children[1]);
    ret = malloc(strlen(temp[0])+strlen(temp[1])+strlen(format)+1);
    sprintf(ret, format, temp[0], temp[1]);
    free(temp[0]); free(temp[1]);
    return ret;
}

char *compile_add(struct parse_node *root) {
    char *ret, *temp[2];
    const char *format;
    if(root->type != PC_add) return NULL;
    format = "add %s, %s\n";
    temp[0] = compile_register(root->children[0]);
    if(!temp[0]) temp[0] = compile_exregister(root->children[0]);
    if(!temp[0]) temp[0] = compile_dereference(root->children[0]);
    temp[1] = compile_register(root->children[0]);
    if(!temp[1]) temp[1] = compile_exregister(root->children[1]);
    if(!temp[1]) temp[1] = compile_dereference(root->children[1]);
    if(!temp[1]) temp[1] = compile_int_literal(root->children[1]);
    ret = malloc(strlen(temp[0])+strlen(temp[1])+strlen(format)+1);
    sprintf(ret, format, temp[0], temp[1]);
    free(temp[0]); free(temp[1]);
    return ret;
}

char *compile_subtract(struct parse_node *root) {
    char *ret, *temp[2];
    const char *format;
    if(root->type != PC_subtract) return NULL;
    format = "sub %s, %s\n";
    temp[0] = compile_register(root->children[0]);
    if(!temp[0]) temp[0] = compile_exregister(root->children[0]);
    if(!temp[0]) temp[0] = compile_dereference(root->children[0]);
    temp[1] = compile_register(root->children[0]);
    if(!temp[1]) temp[1] = compile_exregister(root->children[1]);
    if(!temp[1]) temp[1] = compile_dereference(root->children[1]);
    if(!temp[1]) temp[1] = compile_int_literal(root->children[1]);
    ret = malloc(strlen(temp[0])+strlen(temp[1])+strlen(format)+1);
    sprintf(ret, format, temp[0], temp[1]);
    free(temp[0]); free(temp[1]);
    return ret;
}

char *compile_multiply(struct parse_node *root) {
    char *ret, *temp[2];
    const char *format;
    if(root->type != PC_multiply) return NULL;
    format = "push ax\n"
             "push dx\n"
             "xor dx, dx\n"
             "mov ax, %s\n"
             "mul %s\n"
             "mov %s, ax\n"
             "pop dx\n"
             "pop ax\n";
    temp[0] = compile_register(root->children[0]);
    if(!temp[0]) temp[0] = compile_exregister(root->children[0]);
    if(!temp[0]) temp[0] = compile_dereference(root->children[0]);
    temp[1] = compile_register(root->children[0]);
    if(!temp[1]) temp[1] = compile_exregister(root->children[1]);
    if(!temp[1]) temp[1] = compile_dereference(root->children[1]);
    if(!temp[1]) temp[1] = compile_int_literal(root->children[1]);
    ret = malloc(strlen(temp[0])+strlen(temp[1])+strlen(format)+1);
    sprintf(ret, format, temp[0], temp[1], temp[0]);
    free(temp[0]); free(temp[1]);
    return ret;
}

char *compile_divide(struct parse_node *root) {
    char *ret, *temp[2];
    const char *format;
    if(root->type != PC_divide) return NULL;
    format = "push ax\n"
             "push dx\n"
             "xor dx, dx\n"
             "mov ax, %s\n"
             "div %s\n"
             "mov %s, ax\n"
             "pop dx\n"
             "pop ax\n";
    temp[0] = compile_register(root->children[0]);
    if(!temp[0]) temp[0] = compile_exregister(root->children[0]);
    if(!temp[0]) temp[0] = compile_dereference(root->children[0]);
    temp[1] = compile_register(root->children[0]);
    if(!temp[1]) temp[1] = compile_exregister(root->children[1]);
    if(!temp[1]) temp[1] = compile_dereference(root->children[1]);
    if(!temp[1]) temp[1] = compile_int_literal(root->children[1]);
    ret = malloc(strlen(temp[0])+strlen(temp[1])+strlen(format)+1);
    sprintf(ret, format, temp[0], temp[1], temp[0]);
    free(temp[0]); free(temp[1]);
    return ret;
}

char *compile_xor(struct parse_node *root) {
    char *ret, *temp[2];
    const char *format;
    if(root->type != PC_xor) return NULL;
    format = "xor %s, %s\n";
    temp[0] = compile_register(root->children[0]);
    if(!temp[0]) temp[0] = compile_exregister(root->children[0]);
    if(!temp[0]) temp[0] = compile_dereference(root->children[0]);
    temp[1] = compile_register(root->children[0]);
    if(!temp[1]) temp[1] = compile_exregister(root->children[1]);
    if(!temp[1]) temp[1] = compile_dereference(root->children[1]);
    if(!temp[1]) temp[1] = compile_int_literal(root->children[1]);
    ret = malloc(strlen(temp[0])+strlen(temp[1])+strlen(format)+1);
    sprintf(ret, format, temp[0], temp[1]);
    free(temp[0]); free(temp[1]);
    return ret;
}

char *compile_or(struct parse_node *root) {
    char *ret, *temp[2];
    const char *format;
    if(root->type != PC_or) return NULL;
    format = "or %s, %s\n";
    temp[0] = compile_register(root->children[0]);
    if(!temp[0]) temp[0] = compile_exregister(root->children[0]);
    if(!temp[0]) temp[0] = compile_dereference(root->children[0]);
    temp[1] = compile_register(root->children[0]);
    if(!temp[1]) temp[1] = compile_exregister(root->children[1]);
    if(!temp[1]) temp[1] = compile_dereference(root->children[1]);
    if(!temp[1]) temp[1] = compile_int_literal(root->children[1]);
    ret = malloc(strlen(temp[0])+strlen(temp[1])+strlen(format)+1);
    sprintf(ret, format, temp[0], temp[1]);
    free(temp[0]); free(temp[1]);
    return ret;
}

char *compile_and(struct parse_node *root) {
    char *ret, *temp[2];
    const char *format;
    if(root->type != PC_and) return NULL;
    format = "and %s, %s\n";
    temp[0] = compile_register(root->children[0]);
    if(!temp[0]) temp[0] = compile_exregister(root->children[0]);
    if(!temp[0]) temp[0] = compile_dereference(root->children[0]);
    temp[1] = compile_register(root->children[0]);
    if(!temp[1]) temp[1] = compile_exregister(root->children[1]);
    if(!temp[1]) temp[1] = compile_dereference(root->children[1]);
    if(!temp[1]) temp[1] = compile_int_literal(root->children[1]);
    ret = malloc(strlen(temp[0])+strlen(temp[1])+strlen(format)+1);
    sprintf(ret, format, temp[0], temp[1]);
    free(temp[0]); free(temp[1]);
    return ret;
}

char *compile_negate(struct parse_node *root) {
    char *ret, *temp;
    const char *format;
    if(root->type != PC_negate) return NULL;
    format = "neg %s\n";
    temp = compile_register(root->children[0]);
    if(!temp) temp = compile_exregister(root->children[0]);
    if(!temp) temp = compile_dereference(root->children[0]);
    ret = malloc(strlen(format)+strlen(temp)+1);
    sprintf(ret, format, temp);
    free(temp);
    return ret;
}

char *compile_flip(struct parse_node *root) {
    char *ret, *temp;
    const char *format;
    if(root->type != PC_flip) return NULL;
    format = "not %s\n";
    temp = compile_register(root->children[0]);
    if(!temp) temp = compile_exregister(root->children[0]);
    if(!temp) temp = compile_dereference(root->children[0]);
    ret = malloc(strlen(format)+strlen(temp)+1);
    sprintf(ret, format, temp);
    free(temp);
    return ret;
}

char *compile_increment(struct parse_node *root) {
    char *ret, *temp;
    const char *format;
    if(root->type != PC_increment) return NULL;
    format = "inc %s\n";
    temp = compile_register(root->children[0]);
    if(!temp) temp = compile_exregister(root->children[0]);
    if(!temp) temp = compile_dereference(root->children[0]);
    ret = malloc(strlen(format)+strlen(temp)+1);
    sprintf(ret, format, temp);
    free(temp);
    return ret;
}

char *compile_return(struct parse_node *root) {
    char *ret;
    if(root->type != PC_return) return NULL;
    ret = malloc(strlen("ret\n")+1);
    strcpy(ret, "ret\n");
    return ret;
}

char *compile_break(struct parse_node *root) {
    unsigned int lnum;
    char *ret, c;
    const char *format;
    if(root->type != PC_continue) return NULL;
    format = "jmp .E%c%d\n";
    switch(state_control(SC_get_current, 0, 0)) {
    case SC_forever: c = 'F'; break;
    case SC_loop:    c = 'L'; break;
    }
    ret = malloc(strlen(format)+10);
    sprintf(ret, format, c, state_control(SC_get_curl, 0, 0));
    return ret;
}

char *compile_continue(struct parse_node *root) {
    unsigned int lnum;
    char *ret, c;
    const char *format;
    if(root->type != PC_continue) return NULL;
    format = "jmp .%c%d\n";
    switch(state_control(SC_get_current, 0, 0)) {
    case SC_forever: c = 'F'; break;
    case SC_loop:    c = 'L'; break;
    }
    ret = malloc(strlen(format)+10);
    sprintf(ret, format, c, state_control(SC_get_curl, 0, 0));
    return ret;
}

char *compile_call(struct parse_node *root) {
    char *ret, *temp[2], *id;
    const char *format[2];
    unsigned int i;
    if(root->type != PC_call) return NULL;
    id = compile_iden(root->children[0]);
    format[0] = "%s\ncall %s\n";
    format[1] = "push %s\n";
    ret = calloc(1,1);
    for(i=1; i<root->children_len; i++) {
        temp[1] = compile_register(root->children[i]);
        if(!temp[1]) temp[1] = compile_dereference(root->children[i]);
        if(!temp[1]) temp[1] = compile_int_literal(root->children[i]);
        if(!temp[1]) temp[1] = compile_iden(root->children[i]);
        if(!temp[1]) continue;
        temp[0] = malloc(strlen(format[1])+strlen(temp[1])+1);
        sprintf(temp[0], format[1], temp[1]);
        ret = realloc(ret, strlen(ret)+strlen(temp[0])+1);
        strcat(ret, temp[0]);
        free(temp[0]);
        free(temp[1]);
    }
    temp[0] = ret;
    ret = malloc(strlen(format[0])+strlen(id)+strlen(temp[0])+1);
    sprintf(ret, format[0], temp[0], id);
    free(temp[0]);
    return ret;
}

char *compile_match(struct parse_node *root) {
    char *ret, *temp[2];
    char *v1, *v2;
    const char *format;
    unsigned int i, cn;
    if(root->type != PC_match) return NULL;
    v1 = compile_register(root->children[0]);
    if(!v1) v1 = compile_exregister(root->children[0]);
    if(!v1) v1 = compile_dereference(root->children[0]);
    ret = calloc(1,1);
    for(i=1; i<root->children_len; i++) {
        format = "cmp %s, %s\njnz .M%d\n%s\n.M%d:\n";
        v2 = compile_register(root->children[i]);
        if(!v2) v2 = compile_exregister(root->children[i]);
        if(!v2) v2 = compile_dereference(root->children[i]);
        if(!v2) v2 = compile_int_literal(root->children[i]);
        temp[0] = calloc(1,1);
        for(i++; root->children[i]->type!=PC_end; i++) {
            if(!(temp[1] = compile_statement(root->children[i]))) continue;
            temp[0] = realloc(temp[0], strlen(temp[0])+strlen(temp[1])+1);
            strcat(temp[0], temp[1]);
            free(temp[1]);
        }
        temp[1] = temp[0];
        temp[0] = malloc(strlen(temp[1])+strlen(format)+strlen(v1)+strlen(v2)+31);
        sprintf(temp[0], format, v1, v2, state_control(SC_get, SC_comparison, 0),
            temp[1], state_control(SC_get, SC_comparison, 0));
        state_control(SC_increment, SC_comparison, 0);
        free(temp[1]);
        ret = realloc(ret, strlen(ret)+strlen(temp[0])+1);
        strcat(ret, temp[0]);
        free(temp[0]);
    }
    return ret;
}

char *compile_compare(struct parse_node *root) {
    unsigned int i, loop_number;
    const char *format;
    char *ret, *temp, *v1, *v2;
    const char *jt;
    if(root->type != PC_compare) return NULL;
    format = "cmp %s, %s\nj%s .EI%d\n%s\n.EI%d:\n";
    loop_number = state_control(SC_get, SC_comparison, 0);
    v1 = compile_register(root->children[0]);
    if(!v1) v1 = compile_dereference(root->children[0]);
    if(!v1) v1 = compile_exregister(root->children[0]);
    v2 = compile_register(root->children[1]->children[0]);
    if(!v2) v2 = compile_dereference(root->children[1]->children[0]);
    if(!v2) v2 = compile_int_literal(root->children[1]->children[0]);
    if(!v2) v2 = compile_exregister(root->children[1]->children[0]);
    IFNCMP(root->children[1]->value.s, "equal")   jt = "nz";
    ELNCMP(root->children[1]->value.s, "greater") jt = "le";
    ELNCMP(root->children[1]->value.s, "less")    jt = "ge";
    ret = calloc(1,1);
    for(i=1; i<root->children[1]->children_len; i++) {
        if(!(temp = compile_statement(root->children[1]->children[1]))) continue;
        ret = realloc(ret, strlen(ret)+strlen(temp)+1);
        strcat(ret,temp);
        free(temp);
    }
    temp = ret;
    ret = malloc(strlen(temp)+strlen(format)+strlen(v1)+strlen(v2)+31);
    sprintf(ret, format, v1, v2, jt, loop_number, temp, loop_number);
    state_control(SC_increment, SC_comparison, 0);
    free(temp);
    free(v1);
    free(v2);
    return ret;

}

char *compile_loop(struct parse_node *root) {
    unsigned int i, loop_number, last_loop_number;
    unsigned int last;
    const char *format;
    char *ret, *temp, *v1, *v2;
    char jt;
    if(root->type != PC_loop) return NULL;
    format = ".L%d:\n%s\ncmp %s, %s\njn%c .L%d\n.EL%d:\n";
    loop_number = state_control(SC_get, SC_loop, 0);
    last = state_control(SC_get_current, 0, 0);
    state_control(SC_set_current, SC_loop, 0);
    last_loop_number = state_control(SC_get_curl, SC_loop, 0);
    state_control(SC_set_curl, SC_loop, loop_number);
    v1 = compile_register(root->children[0]);
    if(!v1) v1 = compile_dereference(root->children[0]);
    if(!v1) v1 = compile_exregister(root->children[0]);
    v2 = compile_register(root->children[1]->children[0]);
    if(!v2) v2 = compile_dereference(root->children[1]->children[0]);
    if(!v2) v2 = compile_int_literal(root->children[1]->children[0]);
    if(!v2) v2 = compile_exregister(root->children[1]->children[0]);
    IFNCMP(root->children[1]->value.s, "equal")   jt = 'z';
    ELNCMP(root->children[1]->value.s, "greater") jt = 'g';
    ELNCMP(root->children[1]->value.s, "less")    jt = 'l';
    ret = calloc(1,1);
    for(i=1; i<root->children[1]->children_len; i++) {
        if(!(temp = compile_statement(root->children[1]->children[i]))) continue;
        ret = realloc(ret, strlen(ret)+strlen(temp)+1);
        strcat(ret,temp);
        free(temp);
    }
    temp = ret;
    ret = malloc(strlen(temp)+strlen(format)+strlen(v1)+strlen(v2)+31);
    sprintf(ret, format, loop_number, temp, v1, v2, jt, loop_number, loop_number);
    state_control(SC_increment, SC_loop, 0);
    state_control(SC_set_curl, 0, last_loop_number);
    state_control(SC_set_current, last, 0);
    free(temp);
    free(v1);
    free(v2);
    return ret;
}

char *compile_forever(struct parse_node *root) {
    unsigned int i, last, lastnum;
    const char *format;
    char *ret, *temp;
    if(root->type != PC_forever) return NULL;
    format = ".F%d:\n%s\njmp .F%d\n.EF%d:\n";
    last = state_control(SC_get_current, 0, 0);
    lastnum = state_control(SC_set_current, SC_forever, 0);
    state_control(SC_set_curl, SC_forever, state_control(SC_get, SC_forever, 0));
    ret = calloc(1,1);
    for(i=0; i<root->children_len; i++) {
        if(!(temp = compile_statement(root->children[i]))) continue;
        ret = realloc(ret, strlen(ret)+strlen(temp)+1);
        strcat(ret, temp);
        free(temp);
    }
    temp = ret;
    ret = malloc(strlen(format)+strlen(temp)+30);
    sprintf(ret, format, state_control(SC_get, SC_forever, 0), temp, state_control(SC_get, SC_forever, 0),
        state_control(SC_get, SC_forever, 0));
    state_control(SC_increment, SC_forever, 0);
    state_control(SC_set_current, last, 0);
    state_control(SC_set_curl, 0, lastnum);
    free(temp);
    return ret;
}

char *compile_statement(struct parse_node *root) {
    unsigned int i;
    char *ret;
    char *(*choices[23]) (struct parse_node*) = {
        compile_forever,    compile_loop,
        compile_compare,    compile_match,      compile_call,
        compile_continue,   compile_break,      compile_return,
        compile_increment,  compile_flip,       compile_negate,
        compile_and,        compile_xor,        compile_or,
        compile_divide,     compile_multiply,   compile_subtract,
        compile_add,        compile_move,       compile_interrupt,
        compile_save,       compile_load,       compile_asm
    };
    for(i=0; i<23; i++) if(ret=choices[i](root)) return ret;
    return NULL;
}

char *compile_declaration(struct parse_node *root) {
    char *ret, *temp[2];
    const char *ctemp, *format;
    IFNCMP(root->value.s, "var") {
        format = "%s: %s %s\n";
        SETMATCH(root->children[0]->value.s)
            BEGINMATCH("bytes") ctemp = "db";
            MATCH("words")      ctemp = "dw";
            MATCH("doubles")    ctemp = "dd";
            MATCH("quads")      ctemp = "dq";
        FINMATCH()
        temp[0] = compile_iden(root->children[1]);
        temp[1] = compile_literal(root->children[2]);
        ret = malloc(strlen(format)+strlen(ctemp)+strlen(temp[0])+strlen(temp[1])+1);
        sprintf(ret, format, temp[0], ctemp, temp[1]);
    } ELNCMP(root->value.s, "const") {
        format = "%s = %s\n";
        temp[0] = compile_iden(root->children[0]);
        temp[1] = compile_int_literal(root->children[1]);
        ret = malloc(strlen(format)+strlen(temp[0])+strlen(temp[1])+1);
        sprintf(ret, format, temp[0], temp[1]);
    } else return NULL;
    free(temp[0]);
    free(temp[1]);
    return ret;
}

char *compile_function(struct parse_node *root) {
    char *ret, *temp;
    unsigned int branch;
    if(root->type != PC_function) return NULL;
    temp = compile_iden(root->children[0]);
    ret = malloc(strlen(temp)+27);
    strcpy(ret, temp);
    strcat(ret, ":  ; arg num ");
    free(temp);
    temp = compile_int_literal(root->children[1]);
    strcat(ret, temp);
    strcat(ret, "\n");
    free(temp);
    for(branch=2; branch<root->children_len; branch++) {
        if(!(temp = compile_statement(root->children[branch]))) continue;
        ret = realloc(ret, strlen(ret)+strlen(temp)+1);
        strcat(ret, temp);
        free(temp);
    }
    if(ret[0] == '_') {
        ret = realloc(ret, strlen(ret)+5);
        strcat(ret, "ret\n");
    }
    return ret;
}

char *compile_asm(struct parse_node *root) {
    char *ret;
    if(root->type != PC_asm) return NULL;
    ret = malloc(strlen(root->value.s)+2);
    strcpy(ret, root->value.s);
    strcat(ret, "\n");
    return ret;
}

char *compile_primary_statement(struct parse_node *root) {
    unsigned int i;
    char *temp;
    char *(*choices[]) (struct parse_node*) = {
        compile_asm, compile_function, compile_declaration
    };
    for(i=0; i<3; i++) if(temp = choices[i](root)) return temp;
    return NULL;
}

#define BOILERPLATE "use16\norg 0x7C00\n;"
char *compile_program(struct parse_node *root) {
    char *ret, *temp;
    unsigned int branch;
    ret = malloc(strlen(BOILERPLATE)+strlen(root->children[0]->value.s)+2);
    strcpy(ret, BOILERPLATE);
    strcat(ret, root->children[0]->value.s);
    strcat(ret, "\n");
    for(branch=1; branch<root->children_len; branch++) {
        if(!(temp = compile_primary_statement(root->children[branch]))) continue;
        ret = realloc(ret, strlen(ret)+strlen(temp)+1);
        strcat(ret, temp);
        free(temp);
    }
    return ret;
}
