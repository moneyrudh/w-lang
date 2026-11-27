#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "parser.h"
#include "parser/internal.h"
#include "language/token_registry.h"
#include "language/type_registry.h"
#include "user_types.h"
#include "diagnostics.h"

DataType parse_type_specifier(void) {
    // a name is a type the program defines (a pod); the checker makes sure one exists
    if (token == IDENTIFIER) {
        DataType type = user_type_named(yylval.string);
        eat(IDENTIFIER);
        return type;
    }
    if (token == REF) {
        parser_error("'ref' only goes before a parameter's type (b: ref Ball)");
        return TYPE_ERROR;
    }
    if (!token_registry_is_type(token)) {
        parser_error("Expected a type (num, real, chr, str, bool, zil, or a pod's name)");
        return TYPE_ERROR;
    }

    DataType type = convert_token_to_data_type(token);
    eat(token);
    return type;
}

// ==================== variables ====================

// dec name: type [= expr]   or   dec name := expr (the type is the value's)
ASTNode* parse_variable_declaration(void) {
    SourceLocation loc = current_location();
    eat(DEC);

    if (token != IDENTIFIER) {
        parser_error("Expected identifier in variable declaration");
        skip_rest_of_line();
        return NULL;
    }
    char* name = strdup(yylval.string);
    eat(IDENTIFIER);

    if (token == INFER_ASSIGN) {
        eat(INFER_ASSIGN);
        ASTNode* init_expr = parse_expression();
        expect_end_of_statement();
        ASTNode* node = create_var_declaration_node(name, TYPE_INFER, init_expr, loc);
        free(name);
        return node;
    }

    if (token != COLON) {
        parser_error("Expected ':' or ':=' after variable name in declaration");
        skip_rest_of_line();
        free(name);
        return NULL;
    }
    eat(COLON);

    DataType type = parse_type_specifier();
    if (type == TYPE_ERROR) {
        skip_rest_of_line();
        free(name);
        return NULL;
    }

    ASTNode* init_expr = NULL;
    if (token == ASSIGNMENT) {
        eat(ASSIGNMENT);
        init_expr = parse_expression();
    } else if (token == INFER_ASSIGN) {
        parser_error("Use '=' after a type (':=' is for leaving the type out: dec x := 5)");
    }

    expect_end_of_statement();

    ASTNode* node = create_var_declaration_node(name, type, init_expr, loc);
    free(name);
    return node;
}

ASTNode* parse_assignment(char* name, SourceLocation loc) {
    eat(ASSIGNMENT);
    ASTNode* value = parse_expression();
    expect_end_of_statement();
    return create_assignment_node(name, value, loc);
}

// ==================== pods ====================

// pod Name
//     field: type
//     ...
ASTNode* parse_pod(void) {
    SourceLocation loc = current_location();
    eat(POD);

    if (token != IDENTIFIER) {
        parser_error("Expected the pod's name after 'pod'");
        skip_rest_of_line();
        skip_indented_block();   // its fields, unreported: they'd only cascade
        return NULL;
    }
    char* name = strdup(yylval.string);
    DataType type = user_type_named(name);
    eat(IDENTIFIER);

    if (token != NEWLINE) {
        parser_error("Expected the pod's fields on the lines below its name");
        free(name);
        skip_rest_of_line();
        skip_indented_block();   // its fields, unreported: they'd only cascade
        return NULL;
    }
    eat(NEWLINE);
    if (token != INDENT) {
        // reported on the pod's own line; parsing goes on with whatever comes next
        report_error(loc.line, "A pod needs at least one field");
        free(name);
        return NULL;
    }
    eat(INDENT);

    Parameter* head = NULL;
    Parameter* tail = NULL;
    int count = 0;
    while (token != DEDENT && token != 0) {
        start_statement();   // each field line reports its own error
        if (token == INDENT) {
            // lines indented further than the fields above them: one error, then carry on
            parser_error("This field is indented further than the ones above it");
            skip_indented_block();
            continue;
        }
        if (token != IDENTIFIER) {
            parser_error("Expected a field: name: type");
            skip_rest_of_line();
            continue;
        }
        char* field_name = strdup(yylval.string);
        int field_line = token_line;
        eat(IDENTIFIER);
        if (!eat(COLON)) {
            free(field_name);
            skip_rest_of_line();
            continue;
        }
        DataType field_type = parse_type_specifier();
        if (field_type == TYPE_ERROR) {
            free(field_name);
            skip_rest_of_line();
            continue;
        }
        expect_end_of_statement();

        Parameter* field = malloc(sizeof(Parameter));
        field->name = field_name;
        field->type = field_type;
        field->line = field_line;
        field->is_ref = false;
        field->next = NULL;
        if (tail) tail->next = field; else head = field;
        tail = field;
        count++;
    }
    if (token == DEDENT) eat(DEDENT);

    return create_pod_node(name, type, head, count, loc);
}

// ==================== log ====================

// log(expression, ...): each one is printed, separated by spaces
ASTNode* parse_log(void) {
    SourceLocation loc = current_location();
    eat(LOG);

    int arg_count;
    ASTNode** args = parse_argument_list("log", &arg_count, NULL, NULL);
    expect_end_of_statement();

    return create_log_node(args, arg_count, loc);
}
