#ifndef SEMANTIC_H
#define SEMANTIC_H

#include <string>
#include <vector>
#include "scanner.h"

// Перечисление категорий объектов
enum ObjectCategory {
    CAT_UNDEFINED,
    CAT_VARIABLE,
    CAT_FUNCTION,
    CAT_PARAMETER
};

// Перечисление типов данных
enum DataType {
    TYPE_UNDEFINED,
    TYPE_INT,
    TYPE_SHORT,
    TYPE_LONG,
    TYPE_DOUBLE,
    TYPE_CHAR,
    TYPE_VOID
};

// Предварительное объявление структуры Symbol
struct Symbol;

// Структура для описания одного параметра функции
struct Param {
    std::string name;
    DataType type;
    Param* next = nullptr;
};

// Структура узла семантического дерева (Symbol)
struct Symbol {
    std::string name;
    ObjectCategory category;
    DataType type;
    
    union {
        struct {
            Param* params;
            int param_count;
        } func_info;
        
        struct {
            bool is_initialized;
        } var_info;
    };

    Symbol* parent = nullptr;
    Symbol* child = nullptr;
    Symbol* next = nullptr;
};

// Класс семантического анализатора
class SemanticAnalyzer {
public:
    SemanticAnalyzer();
    ~SemanticAnalyzer();

    // Низкоуровневые функции
    void enterScope();
    void leaveScope();
    bool addSymbol(Symbol* sym);
    Symbol* findSymbol(const std::string& name);
    Symbol* findSymbolInCurrentScope(const std::string& name);

    // Высокоуровневые функции
    void semCheckAssignment(Symbol* left, DataType right_type, int line);
    DataType semCheckBinaryExpr(DataType left_type, const Token& op, DataType right_type, int line);
    
    // Функция для вывода дерева в консоль
    void printTree();
    // Функция для преобразования DataType в строку
    static std::string dataTypeToString(DataType type);
    // Функция для преобразования TokenType в DataType
    static DataType tokenTypeToDataType(TokenType type);

private:
    Symbol* root;          // Корень всего дерева
    Symbol* current_scope; // Указатель на текущую область видимости

    // Рекурсивные вспомогательные функции
    void deleteSubtree(Symbol* node);
    void printSubtree(Symbol* node, int depth);
};

#endif // SEMANTIC_H
