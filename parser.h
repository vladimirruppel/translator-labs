#ifndef PARSER_H
#define PARSER_H

#include "scanner.h"
#include <iostream>
#include <string>

class Parser {
public:
    Parser(Scanner* scanner);

    // Главный метод для запуска анализа
    void parse();

private:
    Scanner* scanner;
    Token current_token;

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
    void Tp(); // <тип>
    void Z(); // <список_переменных>

    // Параметры функции
    void G(); // <параметры>
    void Zf(); // <список_параметров>
    void Ps(); // <один_параметр>

    // Операторы
    void O(); // <оператор>
    void Q(); // <составной_оператор>
    void K(); // <список_операторов>
    void P(); // <оператор_присваивания>
    void U(); // <оператор_цикла>
    void H(); // <вызов_функции>

    // Параметры вызова функции
    void L(); // <входные_параметры>
    void M(); // <список_входных_параметров>
    
    // Выражения (по уровням приоритета)
    void V();  // <выражение>
    void Vx(); // <выражение_xor>
    void Va(); // <выражение_и>
    void Ve(); // <выражение_равенства>
    void Vr(); // <выражение_отношения>
    void Vs(); // <выражение_сдвига>
    void A();  // <слагаемое>
    void B();  // <множитель>
    void Vu(); // <унарное_выражение>
    void E();  // <эл.выр.>
    void C();  // <константа>
};

#endif // PARSER_H
