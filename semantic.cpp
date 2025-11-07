#include "semantic.h"
#include <iostream>

// --- Реализация низкоуровневых функций ---

SemanticAnalyzer::SemanticAnalyzer() {
    root = new Symbol{"global", CAT_UNDEFINED, TYPE_UNDEFINED};
    current_scope = root;
}

SemanticAnalyzer::~SemanticAnalyzer() {
    deleteSubtree(root);
}

void SemanticAnalyzer::enterScope() {
    Symbol* new_scope_node = new Symbol{"scope", CAT_UNDEFINED, TYPE_UNDEFINED};
    new_scope_node->parent = current_scope;

    // Ищем последнего ребенка в текущей области
    if (current_scope->child == nullptr) {
        current_scope->child = new_scope_node;
    } else {
        Symbol* temp = current_scope->child;
        while (temp->next != nullptr) {
            temp = temp->next;
        }
        temp->next = new_scope_node;
    }
    current_scope = new_scope_node;
}

void SemanticAnalyzer::leaveScope() {
    if (current_scope->parent != nullptr) {
        current_scope = current_scope->parent;
    }
}

bool SemanticAnalyzer::addSymbol(Symbol* sym) {
    if (findSymbolInCurrentScope(sym->name) != nullptr) {
        return false; // Символ уже существует в этой области
    }
    
    sym->parent = current_scope;

    // Добавляем в список дочерних узлов текущей области
    if (current_scope->child == nullptr) {
        current_scope->child = sym;
    } else {
        Symbol* temp = current_scope->child;
        while (temp->next != nullptr) {
            temp = temp->next;
        }
        temp->next = sym;
    }
    return true;
}

Symbol* SemanticAnalyzer::findSymbolInCurrentScope(const std::string& name) {
    for (Symbol* current = current_scope->child; current != nullptr; current = current->next) {
        if (current->name == name) {
            return current;
        }
    }
    return nullptr;
}

Symbol* SemanticAnalyzer::findSymbol(const std::string& name) {
    for (Symbol* scope = current_scope; scope != nullptr; scope = scope->parent) {
        for (Symbol* current = scope->child; current != nullptr; current = current->next) {
            if (current->name == name) {
                return current;
            }
        }
    }
    return nullptr;
}


// --- Реализация вспомогательных функций ---

void SemanticAnalyzer::deleteSubtree(Symbol* node) {
    if (node == nullptr) return;
    deleteSubtree(node->child);
    deleteSubtree(node->next);
    // Дополнительная очистка памяти для параметров функции
    if(node->category == CAT_FUNCTION) {
        Param* p = node->func_info.params;
        while(p) {
            Param* next = p->next;
            delete p;
            p = next;
        }
    }
    delete node;
}

void SemanticAnalyzer::printTree() {
    std::cout << "\n--- Semantic Tree ---\n";
    printSubtree(root, 0);
    std::cout << "---------------------\n";
}

void SemanticAnalyzer::printSubtree(Symbol* node, int depth) {
    if (node == nullptr) return;

    for (int i = 0; i < depth; ++i) std::cout << "  ";

    if (node->name == "scope") {
        std::cout << "[Scope]\n";
    } else {
        std::cout << node->name << " (" << dataTypeToString(node->type) << ")\n";
    }

    printSubtree(node->child, depth + 1);
    printSubtree(node->next, depth);
}

std::string SemanticAnalyzer::dataTypeToString(DataType type) {
    switch(type) {
        case TYPE_INT: return "int";
        case TYPE_SHORT: return "short";
        case TYPE_LONG: return "long";
        case TYPE_DOUBLE: return "double";
        case TYPE_CHAR: return "char";
        case TYPE_VOID: return "void";
        default: return "undefined";
    }
}

DataType SemanticAnalyzer::tokenTypeToDataType(TokenType type) {
    switch(type) {
        case T_INT: return TYPE_INT;
        case T_SHORT: return TYPE_SHORT;
        case T_LONG: return TYPE_LONG;
        case T_DOUBLE: return TYPE_DOUBLE;
        case T_CHAR: return TYPE_CHAR;
        case T_VOID: return TYPE_VOID;
        default: return TYPE_UNDEFINED;
    }
}

// --- Реализация высокоуровневых функций ---

// Проверка операции присваивания
void SemanticAnalyzer::semCheckAssignment(Symbol* left, DataType right_type, int line) {
    if (left->category != CAT_VARIABLE && left->category != CAT_PARAMETER) {
        throw std::runtime_error("Ошибка на строке " + std::to_string(line) + ": Нельзя присвоить значение не-переменной '" + left->name + "'");
    }

    DataType left_type = left->type;

    // Правила приведения типов при присваивании
    // double = int/short/long/char (OK)
    // int/long/short = char (OK)
    // Все остальное, где типы не совпадают - ошибка (упрощенно)
    
    bool is_compatible = (left_type == right_type);
    
    if (!is_compatible) {
        bool is_numeric_left = (left_type == TYPE_INT || left_type == TYPE_SHORT || left_type == TYPE_LONG || left_type == TYPE_DOUBLE);
        bool is_numeric_right = (right_type == TYPE_INT || right_type == TYPE_SHORT || right_type == TYPE_LONG || right_type == TYPE_DOUBLE || right_type == TYPE_CHAR);
        
        if (is_numeric_left && is_numeric_right) {
            // Разрешаем любое присваивание между числовыми типами и char
            // В реальном компиляторе здесь были бы предупреждения о потере данных (double -> int)
            is_compatible = true;
        }
    }
    
    if (!is_compatible) {
         throw std::runtime_error("Ошибка на строке " + std::to_string(line) + ": Несовместимые типы при присваивании. Нельзя присвоить '" + dataTypeToString(right_type) + "' переменной типа '" + dataTypeToString(left_type) + "'");
    }
}

// Проверка типов в бинарной операции
DataType SemanticAnalyzer::semCheckBinaryExpr(DataType left_type, const Token& op, DataType right_type, int line) {
    bool is_left_int_family = (left_type == TYPE_INT || left_type == TYPE_SHORT || left_type == TYPE_LONG || left_type == TYPE_CHAR);
    bool is_right_int_family = (right_type == TYPE_INT || right_type == TYPE_SHORT || right_type == TYPE_LONG || right_type == TYPE_CHAR);

    switch (op.type) {
        // Арифметические операции
        case T_PLUS: case T_MINUS: case T_MUL: case T_DIV:
            if (left_type == TYPE_DOUBLE || right_type == TYPE_DOUBLE) return TYPE_DOUBLE;
            if (is_left_int_family && is_right_int_family) return TYPE_INT; // Упрощенно, возвращаем INT для всех целых
            break;

        case T_MOD:
            if (is_left_int_family && is_right_int_family) return TYPE_INT;
            break;
            
        // Побитовые операции
        case T_BIT_OR: case T_BIT_XOR: case T_BIT_AND: case T_LSHIFT: case T_RSHIFT:
            if (is_left_int_family && is_right_int_family) return TYPE_INT;
            break;
            
        // Операции сравнения
        case T_EQ: case T_NE: case T_LT: case T_LE: case T_GT: case T_GE:
            // Сравнивать можно любые числовые типы и char между собой
            if ((is_left_int_family || left_type == TYPE_DOUBLE) && (is_right_int_family || right_type == TYPE_DOUBLE)) {
                return TYPE_INT; // Результат сравнения - логический (int)
            }
            break;
        
        default:
            break;
    }
    
    // Если ни одно правило не подошло, это ошибка
    throw std::runtime_error("Ошибка на строке " + std::to_string(line) + ": Операция '" + op.text + "' не применима к операндам типов '" + dataTypeToString(left_type) + "' и '" + dataTypeToString(right_type) + "'");
}
