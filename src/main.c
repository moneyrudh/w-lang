#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "types.h"
#include "symbol_table.h"
#include "ast.h"
#include "gen.h"
#include "parser.h"

YYSTYPE yylval;
TokenType token;
char* output_file_name;

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s input.w output.c\n", argv[0]);
        return 1;
    }

    FILE* input = fopen(argv[1], "r");
    if (!input) {
        perror("Error opening input file.");
        return 1;
    }

    output_file_name = argv[2];
    FILE* output = fopen(output_file_name, "w");
    if (!output) {
        perror("Error opening output file.");
        fclose(input);
        return 1;
    }

    init_parser();
    yyin = input;
    token = yylex();
    // printf("Token: %d\n", token);
    ast = parse();

    if (!ast || ast->type != NODE_PROGRAM) {
        fprintf(stderr, "Failed to parse program.\n");
        return 1;
    }

    generate_code(output, ast);

    free_ast(ast);
    fclose(input);
    fclose(output);

    cleanup_parser();

    // if (getParserState().error_count > 0) {
    //     unlink(output_file_name);
    //     return 1;
    // }

    return 0;
}