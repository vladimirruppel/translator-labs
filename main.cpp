#include <iostream>
#include <fstream>
#include <sstream>
#include "scanner.h"
#include "parser.h"

// Функция для удобного вывода имени токена
std::string tokenTypeToString(TokenType type) {
    switch (type) {
        case T_VOID: return "T_VOID";
        case T_SHORT: return "T_SHORT";
        case T_LONG: return "T_LONG";
        case T_INT: return "T_INT";
        case T_DOUBLE: return "T_DOUBLE";
        case T_CHAR: return "T_CHAR";
        case T_WHILE: return "T_WHILE";
        case T_MAIN: return "T_MAIN";
        case T_IDENT: return "T_IDENT";
        case T_DEC_CONST: return "T_DEC_CONST";
        case T_HEX_CONST: return "T_HEX_CONST";
        case T_FLOAT_CONST: return "T_FLOAT_CONST";
        case T_CHAR_CONST: return "T_CHAR_CONST";
        case T_BIT_OR: return "T_BIT_OR";
        case T_BIT_XOR: return "T_BIT_XOR";
        case T_BIT_AND: return "T_BIT_AND";
        case T_EQ: return "T_EQ";
        case T_NE: return "T_NE";
        case T_LT: return "T_LT";
        case T_LE: return "T_LE";
        case T_GT: return "T_GT";
        case T_GE: return "T_GE";
        case T_LSHIFT: return "T_LSHIFT";
        case T_RSHIFT: return "T_RSHIFT";
        case T_PLUS: return "T_PLUS";
        case T_MINUS: return "T_MINUS";
        case T_MUL: return "T_MUL";
        case T_DIV: return "T_DIV";
        case T_MOD: return "T_MOD";
        case T_ASSIGN: return "T_ASSIGN";
        case T_SEMICOLON: return "T_SEMICOLON";
        case T_COMMA: return "T_COMMA";
        case T_LPAREN: return "T_LPAREN";
        case T_RPAREN: return "T_RPAREN";
        case T_LBRACE: return "T_LBRACE";
        case T_RBRACE: return "T_RBRACE";
        case T_EOF: return "T_EOF";
        case T_ERROR: return "T_ERROR";
        default: return "UNKNOWN";
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
        return 1;
    }

    std::ifstream file(argv[1]);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << argv[1] << std::endl;
        return 1;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();

    try {
        Scanner scanner(source);
        Parser parser(&scanner);
        parser.parse();
        
        std::cout << "Syntax analysis finished successfully." << std::endl;

    } catch (const std::runtime_error& e) {
        std::cerr << "Syntax error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
