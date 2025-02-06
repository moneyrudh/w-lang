#include "operator_utils.h"

OperatorType char_to_operator(char op) {
    switch (op) {
        case '+': return OP_ADD;
        case '-': return OP_SUB;
        case '*': return OP_MUL;
        case '/': return OP_DIV;
        default: 
            // error handle invalid op
            return OP_ADD;
    }
}

char operator_to_char(OperatorType op) {
    switch (op) {
        case OP_ADD: return '+';
        case OP_SUB: return '-';
        case OP_MUL: return '*';
        case OP_DIV: return '/';
        default: return '+';
    }
}