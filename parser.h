#ifndef _PARSER_H_
#define _PARSER_H_

#include <list>
#include <vector>
#include <string>
#include <stack>
#include "tree.h"
#include "lexer.h"

class Parser {
public:
	Parser(std::vector<Token> tokens);
	virtual ~Parser();

	void parse(); //Парсинг
	TreeNode* getAST();
	Token getNext(); //Вернуть следующий токен
	Token getCurrent(); //Вернуть текущий токен
	TokenType currentType(); //Тип текущего токена
	void printTree(TreeNode* node);

	TreeNode* rootNode;
	TreeNode* parse_compilation_unit(); //Проверка типа функции и ее парсинг
	TreeNode* parse_function_declaration(); //Парсинг функции
	TreeNode* parse_function_header(); //Ветвь инициализации функции (тип, идентификатор, параметры)
	TreeNode* parse_function_returntype(); //Вернуть тип функции в виде узла (содержащего этот тип), являющегося потомком созданного узла RETURNTYPE (возвращается созданный узел)
	TreeNode* parse_function_param_list(); //Вернуть параметры функции
	TreeNode* parse_param();
	TreeNode* parse_block(); //Парсинг блока (содержимого фигурных скобок)
	TreeNode* parse_print();
	TreeNode* parse_if();
	TreeNode* parse_else();
	TreeNode* parse_statement(); //Полный парсинг кода в блоке функции
	TreeNode* parse_assembly(); //Парсинг ассемблера
	TreeNode* parse_declaration_statement(); //Парсинг кода в блоке функции
	TreeNode* parse_expression(); //Парсинг выражения
	TreeNode* parse_initializer(); //Парсинг инициализации
	TreeNode* parse_expression_relation(); //Самый полный парсинг выражения (с учетом сравнения)
	TreeNode* parse_expression_addition(); //Полный парсинг выражения
	TreeNode* parse_expression_multiplication(); //Парсинг выражения
	TreeNode* parse_factor(); //Парсинг идентификатора (переменной, функции) или числа
	TreeNode* parse_unary1(); //Парсинг переменной/функции/числа
	TreeNode* parse_unary2(); //Парсинг действия выражения
	TreeNode* parse_function_args();

	std::vector<Token> m_tokenList;
	unsigned int m_currentTokenIndex;
};

#endif
