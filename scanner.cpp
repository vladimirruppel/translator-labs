#include "scanner.h"
#include <cctype>

// Конструктор инициализирует состояние сканера и заполняет таблицу ключевых слов
Scanner::Scanner(const std::string& source) 
    : source_code(source), current_pos(0), current_line(1) {
    // Заполняем карту ключевых слов для быстрой проверки
    keywords["void"] = T_VOID;
    keywords["short"] = T_SHORT;
    keywords["long"] = T_LONG;
    keywords["int"] = T_INT;
    keywords["double"] = T_DOUBLE;
    keywords["char"] = T_CHAR;
    keywords["while"] = T_WHILE;
}

// "Заглядывание" вперед
char Scanner::peek() {
    if (current_pos >= source_code.length()) return '\0';
    return source_code[current_pos];
}

// Чтение символа с продвижением
char Scanner::advance() {
    if (current_pos < source_code.length()) {
        if (source_code[current_pos] == '\n') {
            current_line++;
        }
        return source_code[current_pos++];
    }
    return '\0';
}

// Пропуск пробелов, табуляций, новых строк и комментариев
void Scanner::skipWhitespaceAndComments() {
    while (true) {
        char c = peek();
        if (isspace(c)) { // Пробел, \t, \n, \r
            advance();
        } else if (c == '/' && (current_pos + 1 < source_code.length() && source_code[current_pos + 1] == '/')) {
            // Комментарий "//", пропускаем до конца строки
            while (peek() != '\n' && peek() != '\0') {
                advance();
            }
        } else {
            break; // Нашли значащий символ
        }
    }
}

// Основной метод, реализующий логику конечного автомата
Token Scanner::getNextToken() {
    skipWhitespaceAndComments();

    size_t start_pos = current_pos;
    int start_line = current_line;

    if (current_pos >= source_code.length()) {
        return {T_EOF, "EOF", start_line};
    }

    char c = advance();

    // 1. Ветка для идентификаторов и ключевых слов (состояние R на диаграмме)
    if (isalpha(c) || c == '_') {
        while (isalnum(peek()) || peek() == '_') {
            advance();
        }
        std::string text = source_code.substr(start_pos, current_pos - start_pos);
        auto it = keywords.find(text);
        if (it != keywords.end()) {
            return {it->second, text, start_line}; // Нашли ключевое слово
        }
        return {T_IDENT, text, start_line}; // Это идентификатор
    }

    // 2. Ветка для числовых констант
    if (isdigit(c) || c == '.') {
        bool isFloat = (c == '.');
        // Обработка 16-ричных констант (0x...)
        if (c == '0' && (peek() == 'x' || peek() == 'X')) {
            advance(); // съедаем 'x'
            start_pos += 2; // смещаем начало лексемы
            while (isxdigit(peek())) {
                advance();
            }
            return {T_HEX_CONST, source_code.substr(start_pos, current_pos - start_pos), start_line};
        }
        // Обработка 10-ричных и вещественных
        while (isdigit(peek())) {
            advance();
        }
        if (peek() == '.') {
            isFloat = true;
            advance();
            while (isdigit(peek())) {
                advance();
            }
        }
        std::string text = source_code.substr(start_pos, current_pos - start_pos);
        if (isFloat) {
            return {T_FLOAT_CONST, text, start_line};
        }
        return {T_DEC_CONST, text, start_line};
    }

    // 3. Ветка для символьных констант
    if (c == '\'') {
        std::string text;
        if (peek() == '\\') { // Экранированный символ
            advance(); 
            text += advance();
        } else {
            text += advance();
        }
        if (peek() == '\'') {
            advance();
            return {T_CHAR_CONST, text, start_line};
        }
        return {T_ERROR, "Unclosed char literal", start_line};
    }
    
    // 4. Ветка для операторов и разделителей
    switch (c) {
        case '(': return {T_LPAREN, "(", start_line};
        case ')': return {T_RPAREN, ")", start_line};
        case '{': return {T_LBRACE, "{", start_line};
        case '}': return {T_RBRACE, "}", start_line};
        case ';': return {T_SEMICOLON, ";", start_line};
        case ',': return {T_COMMA, ",", start_line};
        case '+': return {T_PLUS, "+", start_line};
        case '-': return {T_MINUS, "-", start_line};
        case '*': return {T_MUL, "*", start_line};
        case '/': return {T_DIV, "/", start_line};
        case '%': return {T_MOD, "%", start_line};
        case '|': return {T_BIT_OR, "|", start_line};
        case '&': return {T_BIT_AND, "&", start_line};
        case '^': return {T_BIT_XOR, "^", start_line};

        // Двухсимвольные операторы
        case '=':
            if (peek() == '=') { advance(); return {T_EQ, "==", start_line}; }
            return {T_ASSIGN, "=", start_line};
        case '!':
            if (peek() == '=') { advance(); return {T_NE, "!=", start_line}; }
            break; // Одиночный '!' - ошибка в вашем языке
        case '<':
            if (peek() == '=') { advance(); return {T_LE, "<=", start_line}; }
            if (peek() == '<') { advance(); return {T_LSHIFT, "<<", start_line}; }
            return {T_LT, "<", start_line};
        case '>':
            if (peek() == '=') { advance(); return {T_GE, ">=", start_line}; }
            if (peek() == '>') { advance(); return {T_RSHIFT, ">>", start_line}; }
            return {T_GT, ">", start_line};
    }

    // 5. Если ничего не подошло - это ошибка
    return {T_ERROR, std::string(1, c), start_line};
}
