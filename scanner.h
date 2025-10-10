#ifndef SCANNER_H
#define SCANNER_H

#include <string>
#include <vector>
#include <map>

enum TokenType {
    // Ключевые слова
    T_VOID, T_SHORT, T_LONG, T_INT, T_DOUBLE, T_CHAR, T_WHILE,

    // Идентификаторы
    T_IDENT,

    // Константы
    T_DEC_CONST,  // 123
    T_HEX_CONST,  // 0x1A
    T_FLOAT_CONST,// 1.23
    T_CHAR_CONST, // 'a'

    // Операторы
    T_BIT_OR,     // |
    T_BIT_XOR,    // ^
    T_BIT_AND,    // &
    T_EQ,         // ==
    T_NE,         // !=
    T_LT,         // <
    T_LE,         // <=
    T_GT,         // >
    T_GE,         // >=
    T_LSHIFT,     // <<
    T_RSHIFT,     // >>
    T_PLUS,       // +
    T_MINUS,      // -
    T_MUL,        // *
    T_DIV,        // /
    T_MOD,        // %
    T_ASSIGN,     // =

    // Разделители (пунктуация)
    T_SEMICOLON,  // ;
    T_COMMA,      // ,
    T_LPAREN,     // (
    T_RPAREN,     // )
    T_LBRACE,     // {
    T_RBRACE,     // }

    // Специальные лексемы
    T_EOF,        // Конец файла/ввода
    T_ERROR       // Ошибочная лексема
};

// Структура для хранения информации о распознанной лексеме
struct Token {
    TokenType type;
    std::string text;
    int line; // Номер строки, где найдена лексема
};
    
// Класс лексического анализатора (сканера)
class Scanner {
public:
    // Конструктор, принимающий исходный код в виде строки
    Scanner(const std::string& source);

    // Главный метод, который возвращает следующую лексему из потока
    Token getNextToken();

private:
    std::string source_code; // Строка с исходным кодом
    size_t current_pos;      // Текущая позиция в строке (аналог 'uk' в пособии)
    int current_line;        // Текущая строка

    std::map<std::string, TokenType> keywords; // Таблица для быстрой проверки ключевых слов

    // Вспомогательные методы
    char peek();             // "Заглянуть" на следующий символ, не сдвигая позицию
    char advance();          // Прочитать текущий символ и сдвинуть позицию
    void skipWhitespaceAndComments(); // Пропустить все незначащие символы
};

#endif // SCANNER_H
