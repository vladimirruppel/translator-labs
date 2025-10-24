#include "parser.h"
#include <stdexcept>

// --- Конструктор и вспомогательные методы ---

Parser::Parser(Scanner* scanner) : scanner(scanner) {
    advance();
}

void Parser::parse() {
    S();
    consume(T_EOF, "Обнаружены лишние символы после конца программы.");
}

void Parser::advance() {
    current_token = scanner->getNextToken();
}

void Parser::consume(TokenType expected, const std::string& error_message) {
    if (current_token.type == expected) {
        advance();
    } else {
        error(error_message);
    }
}

void Parser::error(const std::string& message) {
    std::string error_message = message + 
                                "\n\tНа строке " + std::to_string(current_token.line) + 
                                ", получен токен: \"" + current_token.text + "\"";
    throw std::runtime_error(error_message);
}

// --- Реализация функций-нетерминалов ---

// S -> T
void Parser::S() {
    T();
}

// T -> T W | ε
void Parser::T() {
    while (current_token.type == T_SHORT || current_token.type == T_LONG ||
           current_token.type == T_INT   || current_token.type == T_DOUBLE ||
           current_token.type == T_CHAR  || current_token.type == T_VOID)
    {
        W();
    }

    if (current_token.type == T_IDENT) {
        error("Недопустимый идентификатор в глобальной области. Возможно, пропущен тип данных или описание функции.");
    }
}

// W -> D | F
void Parser::W() {
    if (current_token.type == T_VOID) {
        F();
    } 
    else if (current_token.type == T_SHORT || current_token.type == T_LONG ||
             current_token.type == T_INT   || current_token.type == T_DOUBLE ||
             current_token.type == T_CHAR) 
    {
        D();
    }
    else {
        error("Ожидалось описание данных или функции.");
    }
}

// D -> Tp Z ;
void Parser::D() {
    Tp();
    Z();
    consume(T_SEMICOLON, "Ожидалась ';' после описания переменных.");
}

// Tp -> t1 | t2 | t3 | t4 | ...
void Parser::Tp() {
    if (current_token.type == T_SHORT || current_token.type == T_LONG ||
        current_token.type == T_INT   || current_token.type == T_DOUBLE ||
        current_token.type == T_CHAR) 
    {
        advance();
    } else {
        error("Ожидался тип данных (int, double, etc.).");
    }
}

// Z -> a | a = V | Z, a | Z, a = V
void Parser::Z() {
    do {
        if (current_token.type == T_IDENT || current_token.type == T_MAIN) {
            advance();
        } else {
            error("Ожидался идентификатор переменной.");
        }
        
        if (current_token.type == T_ASSIGN) {
            advance();
            V();
        }
    } while (current_token.type == T_COMMA ? (advance(), true) : false);
}

// F -> void a (G) Q
void Parser::F() {
    consume(T_VOID, "Ожидалось ключевое слово 'void' в описании функции.");
    
    if (current_token.type == T_IDENT || current_token.type == T_MAIN) {
        advance();
    } else {
        error("Ожидалось имя функции (идентификатор или 'main').");
    }

    consume(T_LPAREN, "Ожидалась '(' после имени функции.");
    G();
    consume(T_RPAREN, "Ожидалась ')' после списка параметров.");
    Q();
}

// G -> Zf | ε
void Parser::G() {
    if (current_token.type == T_SHORT || current_token.type == T_LONG ||
        current_token.type == T_INT   || current_token.type == T_DOUBLE ||
        current_token.type == T_CHAR) 
    {
        Zf();
    }
}

// Zf -> Ps | Zf, Ps
void Parser::Zf() {
    do {
        Ps();
    } while (current_token.type == T_COMMA ? (advance(), true) : false);

    if (current_token.type == T_INT || current_token.type == T_SHORT ||
        current_token.type == T_LONG || current_token.type == T_DOUBLE ||
        current_token.type == T_CHAR)
    {
        error("Возможно, пропущена запятая ',' между параметрами функции.");
    }
}

// Ps -> Tp a
void Parser::Ps() {
    Tp();
    consume(T_IDENT, "Ожидался идентификатор параметра.");
}

// O -> P; | Q | U | H; | ;
void Parser::O() {
    switch (current_token.type) {
        case T_IDENT:
        case T_MAIN:
        {
            // Lookahead на 1 токен, чтобы отличить вызов функции (H) от присваивания (P)
            size_t old_pos = scanner->getUK();
            int old_line = scanner->getLine();
            Token ident_token = current_token;
            
            advance();
            TokenType next_type = current_token.type;
            
            scanner->putUK(old_pos);
            scanner->setLine(old_line);
            current_token = ident_token;

            if (next_type == T_LPAREN) {
                H();
                consume(T_SEMICOLON, "Ожидалась ';' после вызова функции.");
            } else {
                P();
                consume(T_SEMICOLON, "Ожидалась ';' после оператора присваивания.");
            }
            break;
        }

        case T_LBRACE:
            Q();
            break;

        case T_WHILE:
            U();
            break;

        case T_SEMICOLON:
            advance();
            break;

        default:
            error("Ожидался оператор.");
            break;
    }
}

// Q -> {K}
void Parser::Q() {
    consume(T_LBRACE, "Ожидался символ '{' для начала составного оператора.");
    K();
    consume(T_RBRACE, "Ожидался символ '}' для завершения составного оператора.");
}

// K -> K O | ε
void Parser::K() {
    while (current_token.type == T_IDENT || current_token.type == T_MAIN ||
           current_token.type == T_LBRACE || current_token.type == T_WHILE ||
           current_token.type == T_SEMICOLON)
    {
        O();
    }

    if (current_token.type == T_INT || current_token.type == T_SHORT ||
        current_token.type == T_LONG || current_token.type == T_DOUBLE ||
        current_token.type == T_CHAR)
    {
        error("Описание переменных внутри функций не поддерживается в этом языке.");
    }
}

// P -> a = V
void Parser::P() {
    if (current_token.type == T_IDENT || current_token.type == T_MAIN) {
        advance();
    } else {
        error("Ожидался идентификатор (переменная) слева от '='.");
    }
    consume(T_ASSIGN, "Ожидался оператор присваивания '='.");
    V();
}

// U -> while (V) O
void Parser::U() {
    consume(T_WHILE, "Ожидался 'while'.");
    consume(T_LPAREN, "Ожидалась '(' после 'while'.");
    V();
    consume(T_RPAREN, "Ожидалась ')' после условия в 'while'.");
    O();
}

// H -> a(L)
void Parser::H() {
    if (current_token.type == T_IDENT || current_token.type == T_MAIN) {
        advance();
    } else {
        error("Ожидалось имя функции для вызова.");
    }
    consume(T_LPAREN, "Ожидалась '(' при вызове функции.");
    L();
    consume(T_RPAREN, "Ожидалась ')' после списка параметров функции.");
}

// L -> M | ε
void Parser::L() {
    if (current_token.type == T_IDENT || current_token.type == T_MAIN ||
        current_token.type == T_DEC_CONST || current_token.type == T_HEX_CONST ||
        current_token.type == T_FLOAT_CONST || current_token.type == T_CHAR_CONST ||
        current_token.type == T_LPAREN || current_token.type == T_PLUS || 
        current_token.type == T_MINUS)
    {
        M();
    }
}

// M -> V | M, V
void Parser::M() {
    do {
        V();
    } while (current_token.type == T_COMMA ? (advance(), true) : false);
}

// --- Функции для разбора выражений ---

// V -> V '|' Vx | Vx
void Parser::V() {
    Vx();
    while (current_token.type == T_BIT_OR) {
        advance();
        Vx();
    }
}

// Vx -> Vx '^' Va | Va
void Parser::Vx() {
    Va();
    while (current_token.type == T_BIT_XOR) {
        advance();
        Va();
    }
}

// Va -> Va '&' Ve | Ve
void Parser::Va() {
    Ve();
    while (current_token.type == T_BIT_AND) {
        advance();
        Ve();
    }
}

// Ve -> Ve '==' Vr | Ve '!=' Vr | Vr
void Parser::Ve() {
    Vr();
    while (current_token.type == T_EQ || current_token.type == T_NE) {
        advance();
        Vr();
    }
}

// Vr -> Vr '<' Vs | ... | Vs
void Parser::Vr() {
    Vs();
    while (current_token.type == T_LT || current_token.type == T_LE ||
           current_token.type == T_GT || current_token.type == T_GE) 
    {
        advance();
        Vs();
    }
}

// Vs -> Vs '<<' A | Vs '>>' A | A
void Parser::Vs() {
    A();
    while (current_token.type == T_LSHIFT || current_token.type == T_RSHIFT) {
        advance();
        A();
    }
}

// A -> A '+' B | A '-' B | B
void Parser::A() {
    B();
    while (current_token.type == T_PLUS || current_token.type == T_MINUS) {
        advance();
        B();
    }
}

// B -> B '*' Vu | ... | Vu
void Parser::B() {
    Vu();
    while (current_token.type == T_MUL || current_token.type == T_DIV || current_token.type == T_MOD) {
        advance();
        Vu();
    }
}

// Vu -> '+' E | '-' E | E
void Parser::Vu() {
    if (current_token.type == T_PLUS || current_token.type == T_MINUS) {
        advance();
        E();
    } else {
        E();
    }
}

// E -> a | C | (V)
void Parser::E() {
    switch (current_token.type) {
        case T_IDENT:
        case T_MAIN:
            advance();
            break;

        case T_DEC_CONST:
        case T_HEX_CONST:
        case T_FLOAT_CONST:
        case T_CHAR_CONST:
            C();
            break;
            
        case T_LPAREN:
            advance();
            V();
            consume(T_RPAREN, "Ожидалась ')' для закрытия выражения в скобках.");
            break;

        default:
            error("Ожидался операнд (переменная, константа или выражение в скобках).");
            break;
    }
}

// C -> c1 | c2 | c3 | c4
void Parser::C() {
    if (current_token.type == T_DEC_CONST || current_token.type == T_HEX_CONST ||
        current_token.type == T_FLOAT_CONST || current_token.type == T_CHAR_CONST) 
    {
        advance();
    } else {
        error("Ожидалась константа.");
    }
}
