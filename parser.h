#ifndef PARSER_H
#define PARSER_H

#include "scanner.h"
#include "semantic.h"
#include <iostream>
#include <string>
#include <vector> 

class Parser {
public:
    Parser(Scanner* scanner);

    // Главный метод для запуска анализа
    void parse();

private:
    Scanner* scanner;
    Token current_token;
    SemanticAnalyzer sem_analyzer;

    // Вспомогательные методы
    void advance(); // Получить следующий токен от сканера
    void consume(TokenType expected, const std::string& error_message); // Проверить и "съесть" токен
    void error(const std::string& message); // Вывести сообщение об ошибке

    // --- Функции для нетерминалов ---
    // Общая структура программы
    void S(); // <программа>
    void T(); // <список_описаний>
    void W(); // <описание>

    // Описания
    void D(); // <описание_данных>
    void F(); // <описание_функции>
    DataType Tp(); // <тип>
    void Z(DataType type); // <список_переменных>

    // Параметры функции
    std::vector<Param*> G(); // <параметры>
    void Zf(std::vector<Param*>& params); // <список_параметров>
    Param* Ps(); // <один_параметр>

    // Операторы
    void O(); // <оператор>
    void Q(); // <составной_оператор>
    void K(); // <список_операторов>
    void P(); // <оператор_присваивания>
    void U(); // <оператор_цикла>
    void H(); // <вызов_функции>

    // Параметры вызова функции
    void L(Symbol* func_sym); // <входные_параметры>
    void M(Symbol* func_sym); // <список_входных_параметров>
    
    // Выражения (по уровням приоритета)
    DataType V();  // <выражение>
    DataType Vx(); // <выражение_xor>
    DataType Va(); // <выражение_и>
    DataType Ve(); // <выражение_равенства>
    DataType Vr(); // <выражение_отношения>
    DataType Vs(); // <выражение_сдвига>
    DataType A();  // <слагаемое>
    DataType B();  // <множитель>
    DataType Vu(); // <унарное_выражение>
    DataType E();  // <эл.выр.>
    DataType C();  // <константа>
};

#endif // PARSER_H
