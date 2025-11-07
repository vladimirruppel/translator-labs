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
    // Вывод построенного дерева для отладки и отчета
    sem_analyzer.printTree(); 
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
    DataType type = Tp();
    Z(type);
    consume(T_SEMICOLON, "Ожидалась ';' после описания переменных.");
}

// Tp -> t1 | t2 | t3 | t4 | ...
DataType Parser::Tp() {
    DataType type = TYPE_UNDEFINED;
    if (current_token.type == T_SHORT || current_token.type == T_LONG ||
        current_token.type == T_INT   || current_token.type == T_DOUBLE ||
        current_token.type == T_CHAR) 
    {
        type = SemanticAnalyzer::tokenTypeToDataType(current_token.type);
        advance();
    } else {
        error("Ожидался тип данных (int, double, etc.).");
    }
    return type;
}

// Z -> a | a = V | Z, a | Z, a = V
void Parser::Z(DataType type) {
    do {
        Token id_token = current_token;
        if (id_token.type != T_IDENT && id_token.type != T_MAIN) {
            error("Ожидался идентификатор переменной.");
        }
        
        Symbol* new_var = new Symbol{id_token.text, CAT_VARIABLE, type};
        if (!sem_analyzer.addSymbol(new_var)) {
            error("Повторное объявление переменной '" + id_token.text + "'");
        }

        advance();

        if (current_token.type == T_ASSIGN) {
            advance();
            DataType expr_type = V();

            sem_analyzer.semCheckAssignment(new_var, expr_type, id_token.line);
        }
    } while (current_token.type == T_COMMA ? (advance(), true) : false);
}

// F -> void a (G) Q
void Parser::F() {
    consume(T_VOID, "Ожидалось ключевое слово 'void' в описании функции.");
    
    Token func_id = current_token;
    if (func_id.type == T_IDENT || func_id.type == T_MAIN) {
        advance();
    } else {
        error("Ожидалось имя функции (идентификатор или 'main').");
    }

    consume(T_LPAREN, "Ожидалась '(' после имени функции.");
    std::vector<Param*> params = G();
    consume(T_RPAREN, "Ожидалась ')' после списка параметров.");

    // Объявляем функцию
    Symbol* new_func = new Symbol{func_id.text, CAT_FUNCTION, TYPE_VOID};
    new_func->func_info.param_count = params.size();
    
    for (size_t i = 0; i < params.size(); ++i) {
        if (i + 1 < params.size()) params[i]->next = params[i+1];
    }
    new_func->func_info.params = params.empty() ? nullptr : params[0];

    if (!sem_analyzer.addSymbol(new_func)) {
        error("Повторное объявление функции '" + func_id.text + "'");
    }
    
    sem_analyzer.enterScope(); // Входим в область видимости функции

    // Объявляем параметры в новой области
    for(Param* p : params) {
        Symbol* param_sym = new Symbol{p->name, CAT_PARAMETER, p->type};
        if (!sem_analyzer.addSymbol(param_sym)) {
            error("Повторное объявление параметра '" + p->name + "'");
        }
    }

    // Q(); // Разбираем тело функции

    consume(T_LBRACE, "Ожидался символ '{' для начала тела функции.");
    K(); // Разбираем список операторов
    consume(T_RBRACE, "Ожидался символ '}' для завершения тела функции.");
    
    sem_analyzer.leaveScope(); // Выходим из области видимости функции
}

// G -> Zf | ε
std::vector<Param*> Parser::G() {
    std::vector<Param*> params;
    if (current_token.type == T_SHORT || current_token.type == T_LONG ||
        current_token.type == T_INT   || current_token.type == T_DOUBLE ||
        current_token.type == T_CHAR) 
    {
        Zf(params);
    }
    return params;
}

// Zf -> Ps | Zf, Ps
void Parser::Zf(std::vector<Param*>& params) {
    do {
        params.push_back(Ps());
    } while (current_token.type == T_COMMA ? (advance(), true) : false);

    if (current_token.type == T_INT || current_token.type == T_SHORT ||
        current_token.type == T_LONG || current_token.type == T_DOUBLE ||
        current_token.type == T_CHAR)
    {
        error("Возможно, пропущена запятая ',' между параметрами функции.");
    }
}

// Ps -> Tp a
Param* Parser::Ps() {
    DataType type = Tp();
    Token id_token = current_token;
    consume(T_IDENT, "Ожидался идентификатор параметра.");
    
    Param* new_param = new Param{id_token.text, type};
    return new_param;
}

// O -> P; | Q | U | H; | D | ;
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

        case T_SHORT:
        case T_LONG:
        case T_INT:
        case T_DOUBLE:
        case T_CHAR:
            D();
            break;

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
            error("Ожидался оператор или описание данных.");
            break;
    }
}

// Q -> {K}
void Parser::Q() {
    consume(T_LBRACE, "Ожидался символ '{' для начала составного оператора.");
    sem_analyzer.enterScope();
    K();
    sem_analyzer.leaveScope();
    consume(T_RBRACE, "Ожидался символ '}' для завершения составного оператора.");
}

// K -> K O | ε
void Parser::K() {
    while (current_token.type == T_IDENT || current_token.type == T_MAIN ||
           current_token.type == T_LBRACE || current_token.type == T_WHILE ||
           current_token.type == T_SEMICOLON || 
           current_token.type == T_INT || current_token.type == T_SHORT || // описания данных внутри функций
           current_token.type == T_LONG || current_token.type == T_DOUBLE ||
           current_token.type == T_CHAR)
    {
        O();
    }
}

// P -> a = V
void Parser::P() {
    Token id_token = current_token;
    if (id_token.type == T_IDENT || id_token.type == T_MAIN) {
        advance();
    } else {
        error("Ожидался идентификатор (переменная) слева от '='.");
    }

    Symbol* var_sym = sem_analyzer.findSymbol(id_token.text);
    if (var_sym == nullptr) {
        error("Использование необъявленной переменной '" + id_token.text + "'");
    }
    
    consume(T_ASSIGN, "Ожидался оператор присваивания '='.");
    DataType right_type = V();
    
    sem_analyzer.semCheckAssignment(var_sym, right_type, id_token.line);
}

// U -> while (V) O
void Parser::U() {
    consume(T_WHILE, "Ожидался 'while'.");
    consume(T_LPAREN, "Ожидалась '(' после 'while'.");
    DataType cond_type = V(); // Получаем тип условия
    // Проверяем тип условия ---
    if (cond_type == TYPE_VOID) {
        error("Выражение в условии 'while' не может быть типа void.");
    }
    consume(T_RPAREN, "Ожидалась ')' после условия в 'while'.");
    O();
}

// H -> a(L)
void Parser::H() {
    Token id_token = current_token;
    if (id_token.type != T_IDENT && id_token.type != T_MAIN) {
        error("Ожидалось имя функции для вызова.");
    }

    // Проверяем идентификатор функции
    Symbol* func_sym = sem_analyzer.findSymbol(id_token.text);
    if (func_sym == nullptr) {
        error("Вызов необъявленной функции '" + id_token.text + "'");
    }
    if (func_sym->category != CAT_FUNCTION) {
        error("'" + id_token.text + "' не является функцией.");
    }

    advance(); 

    consume(T_LPAREN, "Ожидалась '(' при вызове функции.");
    L(func_sym); // Передаем информацию о функции для проверки параметров
    consume(T_RPAREN, "Ожидалась ')' после списка параметров функции.");
}

// L -> M | ε
void Parser::L(Symbol* func_sym) {
    // Проверяем, есть ли параметры, если они не требуются
    if (current_token.type == T_RPAREN) {
        if (func_sym->func_info.param_count != 0) {
            error("Неверное количество аргументов при вызове функции '" + func_sym->name + "'");
        }
        return; // Пустой список параметров
    }
    
    // Если параметры есть, разбираем их
    M(func_sym);
}

// M -> V | M, V
void Parser::M(Symbol* func_sym) {
    int arg_count = 0;
    Param* current_param = func_sym->func_info.params;

    do {
        arg_count++;
        DataType arg_type = V();
        // Проверяем тип параметра
        if (current_param == nullptr) {
            error("Слишком много аргументов при вызове функции '" + func_sym->name + "'");
        }
        if (arg_type != current_param->type) { // Упрощенная проверка
            error("Несоответствие типа для аргумента " + std::to_string(arg_count) + " при вызове функции '" + func_sym->name + "'");
        }
        current_param = current_param->next;
    } while (current_token.type == T_COMMA ? (advance(), true) : false);

    if (arg_count != func_sym->func_info.param_count) {
        error("Неверное количество аргументов при вызове функции '" + func_sym->name + "'");
    }
}

// --- Функции для разбора выражений ---

// V -> V '|' Vx | Vx
DataType Parser::V() {
    DataType left_type = Vx();
    while (current_token.type == T_BIT_OR) {
        Token op = current_token;
        advance();
        DataType right_type = Vx();
        
        left_type = sem_analyzer.semCheckBinaryExpr(left_type, op, right_type, op.line);
    }
    return left_type;
}

// Vx -> Vx '^' Va | Va
DataType Parser::Vx() {
    DataType left_type = Va();
    while (current_token.type == T_BIT_XOR) {
        Token op = current_token;
        advance();
        DataType right_type = Va();
        left_type = sem_analyzer.semCheckBinaryExpr(left_type, op, right_type, op.line);
    }
    return left_type;
}

// Va -> Va '&' Ve | Ve
DataType Parser::Va() {
    DataType left_type = Ve();
    while (current_token.type == T_BIT_AND) {
        Token op = current_token;
        advance();
        DataType right_type = Ve();
        left_type = sem_analyzer.semCheckBinaryExpr(left_type, op, right_type, op.line);
    }
    return left_type;
}

// Ve -> Ve '==' Vr | Ve '!=' Vr | Vr
DataType Parser::Ve() {
    DataType left_type = Vr();
    while (current_token.type == T_EQ || current_token.type == T_NE) {
        Token op = current_token;
        advance();
        DataType right_type = Vr();
        left_type = sem_analyzer.semCheckBinaryExpr(left_type, op, right_type, op.line);
    }
    return left_type;
}

// Vr -> Vr '<' Vs | ... | Vs
DataType Parser::Vr() {
    DataType left_type = Vs();
    while (current_token.type == T_LT || current_token.type == T_LE ||
           current_token.type == T_GT || current_token.type == T_GE) 
    {
        Token op = current_token;
        advance();
        DataType right_type = Vs();
        left_type = sem_analyzer.semCheckBinaryExpr(left_type, op, right_type, op.line);
    }
    return left_type;
}

// Vs -> Vs '<<' A | Vs '>>' A | A
DataType Parser::Vs() {
    DataType left_type = A();
    while (current_token.type == T_LSHIFT || current_token.type == T_RSHIFT) {
        Token op = current_token;
        advance();
        DataType right_type = A();
        left_type = sem_analyzer.semCheckBinaryExpr(left_type, op, right_type, op.line);
    }
    return left_type;
}

// A -> A '+' B | A '-' B | B
DataType Parser::A() {
    DataType left_type = B();
    while (current_token.type == T_PLUS || current_token.type == T_MINUS) {
        Token op = current_token;
        advance();
        DataType right_type = B();
        left_type = sem_analyzer.semCheckBinaryExpr(left_type, op, right_type, op.line);
    }
    return left_type;
}

// B -> B '*' Vu | ... | Vu
DataType Parser::B() {
    DataType left_type = Vu();
    while (current_token.type == T_MUL || current_token.type == T_DIV || current_token.type == T_MOD) {
        Token op = current_token;
        advance();
        DataType right_type = Vu();
        left_type = sem_analyzer.semCheckBinaryExpr(left_type, op, right_type, op.line);
    }
    return left_type;
}

// Vu -> '+' E | '-' E | E
DataType Parser::Vu() {
    if (current_token.type == T_PLUS || current_token.type == T_MINUS) {
        Token op = current_token;
        advance();
        DataType type = E();
        // Проверка для унарных операций
        if (type != TYPE_INT && type != TYPE_SHORT && type != TYPE_LONG && type != TYPE_DOUBLE && type != TYPE_CHAR) {
            error("Унарный оператор '" + op.text + "' применим только к числовым типам.");
        }
        return type;
    } else {
        return E();
    }
}

// E -> a | C | (V)
DataType Parser::E() {
    switch (current_token.type) {
        case T_IDENT:
        case T_MAIN: {
            Token id_token = current_token;
            Symbol* sym = sem_analyzer.findSymbol(id_token.text);
            if (sym == nullptr) {
                error("Использование необъявленного идентификатора '" + id_token.text + "'");
            }

            if (sym->category == CAT_FUNCTION) {
                error("Имя функции '" + id_token.text + "' не может быть использовано в выражении.");
            }

            advance();

            return sym->type;
        }

        case T_DEC_CONST:
        case T_HEX_CONST:
        case T_FLOAT_CONST:
        case T_CHAR_CONST:
            return C();
            
        case T_LPAREN: {
            advance();
            DataType type = V();
            consume(T_RPAREN, "Ожидалась ')' для закрытия выражения в скобках.");
            return type;
        }

        default:
            error("Ожидался операнд (переменная, константа или выражение в скобках).");
    }
    return TYPE_UNDEFINED; // Не должно произойти
}

// C -> c1 | c2 | c3 | c4
DataType Parser::C() {
    DataType type = TYPE_UNDEFINED;
    switch(current_token.type) {
        case T_DEC_CONST:
        case T_HEX_CONST:
            type = TYPE_INT;
            break;
        case T_FLOAT_CONST:
            type = TYPE_DOUBLE;
            break;
        case T_CHAR_CONST:
            type = TYPE_CHAR;
            break;
        default:
            error("Ожидалась константа.");
    }
    advance();
    return type;
}
