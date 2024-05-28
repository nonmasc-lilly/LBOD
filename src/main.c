#include "inc/main.h"
#include <stdio.h>
#include <stdlib.h>

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
    printf("====================================\n");
    exit(0);
}

int main(int argc, const char **argv) {
    const char **input_files, *output_file;
    char *output, *file_content;
    unsigned int input_files_len, i, file_size;
    struct lex_node *lexed;
    bool debug;
    FILE *fp;
    output_file = NULL;
    input_files = NULL;
    input_files_len = 0;
    debug = false;
    if(argc < 4) user_error(E_USAGE, argc, argv);
    for(i=1; i<argc; i++) {
        if(argv[i][0] == '-') switch(argv[i][1]) {
        case 'o':
            if(output_file) user_error(E_MULTIPLE_OUTPUT, argc, argv);
            output_file = argv[++i];
            break;
        case 'd': debug = true; break;
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
        file_content = malloc(file_size);
        fread(file_content, 1, file_size, fp);
        fclose(fp);

        lexed = lex_string(file_content);
        if(debug) represent_tokens(lexed, 0);
        free(file_content);
    }
    free(output);
    return 0;
}
