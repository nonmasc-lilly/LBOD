#include "inc/main.h"
#include <stdio.h>
#include <stdlib.h>

#define VERSION "D.6.18"

enum error_type {
    E_SUCCESS,
    E_USAGE,
    E_MULTIPLE_OUTPUT,
    E_NO_INPUT,
    E_NO_OUTPUT
};

static void user_error(enum error_type error_num, unsigned int argc,
        const char **argv) {
    printf("User error: ");
    switch(error_num) {
    case E_USAGE:
        printf("\n\tUsage: %s *[<option> / <input file>]\n", *argv);
        printf("\tUse option \"-h\" for a list of options\n");
        break;
    case E_MULTIPLE_OUTPUT:
        printf("multiple output files.\n");
        break;
    case E_NO_INPUT:
        printf("no input provided.\n");
        break;
    case E_NO_OUTPUT:
        printf("no output provided.\n");
        break;
    default:
        printf("Unknown\n");
        break;
    }
    exit(error_num);
}

static void help_option() {
    printf("================help================\n");
    printf("| \"-o <file>\": set output file   |\n");
    printf("| \"-h\"       : show this menu    |\n");
    printf("| \"-d\"       : show debug items  |\n");
    printf("====================================\n");
    printf("lbodc %s Copyright (C) 2024 Lilly H. St Claire\n", VERSION);
    printf("This program comes with ABSOLUTELY NO WARRANTY.\n");
    printf("This is free software and you are welcome to redistribute is\n");
    printf("under the conditions of the Gnu General Public License 3.0\n");
    printf("or at your discretion, any later version.\n");
    exit(0);
}

int main(int argc, const char **argv) {
    const char **input_files, *output_file;
    char *output, *file_content, *temp;
    unsigned int input_files_len, i, file_size;
    struct lex_node *lexed;
    struct parse_node *parsed;
    bool debug;
    FILE *fp;
    output_file = NULL;
    input_files = NULL;
    input_files_len = 0;
    debug = false;
    for(i=1; i<argc; i++) {
        if(argv[i][0] == '-') switch(argv[i][1]) {
        case 'o':
            if(output_file) user_error(E_MULTIPLE_OUTPUT, argc, argv);
            output_file = argv[++i];
            break;
        case 'd': debug = true; break;
        case 'h': help_option(); break;
        } else {
            if(!input_files)
                input_files = malloc((++input_files_len)*sizeof(const char *));
            else input_files = realloc(input_files,
                (++input_files_len)*sizeof(const char *));
            input_files[input_files_len-1] = argv[i];
        }
    }
    if(!input_files) user_error(E_NO_INPUT, argc, argv);
    if(!output_file) user_error(E_NO_OUTPUT, argc, argv);
    output = calloc(1,1);
    for(i=0; i<input_files_len; i++) {
        if(debug) printf("compiling: %s\n", input_files[i]);
        fp = fopen(input_files[i], "r");
        file_size = (fseek(fp, 0L, SEEK_END), ftell(fp));
        fseek(fp, 0L, SEEK_SET);
        file_content = calloc(file_size+1,1);
        fread(file_content, 1, file_size, fp);
        fclose(fp);

        lexed = lex_string(file_content);
        if(debug) represent_tokens(lexed, 0);
        parsed = parse_program(lexed);
        if(debug) represent_parse_node(parsed, 0);
        temp = compile_program(parsed);
        if(debug) printf("%s\n", temp);
        output = realloc(output, strlen(output)+strlen(temp)+1);
        strcat(output, temp);
        free(temp);
        destroy_lex_node(lexed);
        destroy_parse_node(parsed);
        free(file_content);
    }
    if(debug) printf("%s\n", output);
    fp = fopen(output_file, "w");
    fwrite(output, 1, strlen(output), fp);
    fclose(fp);
    free(output);
    return 0;
}
