// Every pair of W types against every binary operator, and every type with unary `not`: the
// result type, or "-" when it's an error. The grids are written out by hand (not computed), so a change
// to the rules in src/checker/types.c shows up here as a failing cell.
//
// Built and run by `make test` (see the makefile's unit target).

#include <stdio.h>
#include <string.h>

#include "checker.h"
#include "checker/internal.h"
#include "language/operators.h"

static const DataType TYPES[] = {TYPE_NUM, TYPE_REAL, TYPE_CHR, TYPE_STR, TYPE_BOOL, TYPE_ZIL};
static const char* const NAMES[] = {"num", "real", "chr", "str", "bool", "zil"};
#define TYPE_COUNT 6

typedef struct {
    const char* title;
    OperatorType ops[6];
    int op_count;
    // rows: left type, columns: right type, both in TYPES order
    const char* grid[TYPE_COUNT][TYPE_COUNT];
} Table;

static const Table TABLES[] = {
    {"+", {OP_ADD}, 1, {
        //          num     real    chr     str     bool    zil
        /* num  */ {"num",  "real", "num",  "-",    "-",    "-"},
        /* real */ {"real", "real", "real", "-",    "-",    "-"},
        /* chr  */ {"num",  "real", "num",  "-",    "-",    "-"},
        /* str  */ {"-",    "-",    "-",    "str",  "-",    "-"},   // D8: only str + str joins
        /* bool */ {"-",    "-",    "-",    "-",    "-",    "-"},   // D9: no arithmetic on bool
        /* zil  */ {"-",    "-",    "-",    "-",    "-",    "-"},
    }},
    {"- * /", {OP_SUB, OP_MUL, OP_DIV}, 3, {
        /* num  */ {"num",  "real", "num",  "-",    "-",    "-"},
        /* real */ {"real", "real", "real", "-",    "-",    "-"},
        /* chr  */ {"num",  "real", "num",  "-",    "-",    "-"},
        /* str  */ {"-",    "-",    "-",    "-",    "-",    "-"},
        /* bool */ {"-",    "-",    "-",    "-",    "-",    "-"},
        /* zil  */ {"-",    "-",    "-",    "-",    "-",    "-"},
    }},
    {"== != < > <= >=", {OP_EQ, OP_NE, OP_LT, OP_GT, OP_LE, OP_GE}, 6, {
        /* num  */ {"bool", "bool", "bool", "-",    "bool", "-"},
        /* real */ {"bool", "bool", "bool", "-",    "bool", "-"},
        /* chr  */ {"bool", "bool", "bool", "-",    "bool", "-"},
        /* str  */ {"-",    "-",    "-",    "bool", "-",    "-"},   // str compares with str only
        /* bool */ {"bool", "bool", "bool", "-",    "bool", "-"},
        /* zil  */ {"-",    "-",    "-",    "-",    "-",    "-"},
    }},
    {"and or", {OP_AND, OP_OR}, 2, {
        /* num  */ {"bool", "bool", "bool", "-",    "bool", "-"},
        /* real */ {"bool", "bool", "bool", "-",    "bool", "-"},
        /* chr  */ {"bool", "bool", "bool", "-",    "bool", "-"},
        /* str  */ {"-",    "-",    "-",    "-",    "-",    "-"},
        /* bool */ {"bool", "bool", "bool", "-",    "bool", "-"},
        /* zil  */ {"-",    "-",    "-",    "-",    "-",    "-"},
    }},
};

// unary operators for each type, in TYPES order:         num     real    chr    str  bool zil
static const char* const NOT_RESULTS[TYPE_COUNT] = {"bool", "bool", "bool", "-", "bool", "-"};
static const char* const NEG_RESULTS[TYPE_COUNT] = {"num",  "real", "num",  "-", "-",    "-"};   // D9: no -bool

static const char* type_name(DataType type) {
    for (int i = 0; i < TYPE_COUNT; i++) {
        if (TYPES[i] == type) return NAMES[i];
    }
    return "?";
}

int main(void) {
    int checked = 0, failed = 0;
    for (size_t t = 0; t < sizeof(TABLES) / sizeof(TABLES[0]); t++) {
        const Table* table = &TABLES[t];
        for (int o = 0; o < table->op_count; o++) {
            OperatorType op = table->ops[o];
            for (int l = 0; l < TYPE_COUNT; l++) {
                for (int r = 0; r < TYPE_COUNT; r++) {
                    const char* error = NULL;
                    DataType result = operation_type(TYPES[l], TYPES[r], op, &error);
                    const char* actual = (result == TYPE_ERROR) ? "-" : type_name(result);
                    const char* expected = table->grid[l][r];
                    checked++;

                    // an error must come with a message, a result must not
                    bool consistent = (result == TYPE_ERROR) == (error != NULL);
                    if (strcmp(actual, expected) != 0 || !consistent) {
                        failed++;
                        printf("FAIL  type rules: %s %s %s: expected %s, got %s%s\n",
                               NAMES[l], operator_symbol(op), NAMES[r], expected, actual,
                               consistent ? "" : " (error message and result disagree)");
                    }
                }
            }
        }
    }
    const struct { OperatorType op; const char* const* results; } UNARY[] = {
        {OP_NOT, NOT_RESULTS}, {OP_NEG, NEG_RESULTS},
    };
    for (size_t u = 0; u < sizeof(UNARY) / sizeof(UNARY[0]); u++) {
        for (int t = 0; t < TYPE_COUNT; t++) {
            const char* error = NULL;
            DataType result = unary_operation_type(TYPES[t], UNARY[u].op, &error);
            const char* actual = (result == TYPE_ERROR) ? "-" : type_name(result);
            checked++;
            bool consistent = (result == TYPE_ERROR) == (error != NULL);
            if (strcmp(actual, UNARY[u].results[t]) != 0 || !consistent) {
                failed++;
                printf("FAIL  type rules: %s %s: expected %s, got %s\n", operator_symbol(UNARY[u].op),
                       NAMES[t], UNARY[u].results[t], actual);
            }
        }
    }

    printf("%s  type rules: %d operator / type combinations, %d failed\n", failed ? "FAIL" : "ok  ", checked, failed);
    return failed ? 1 : 0;
}
