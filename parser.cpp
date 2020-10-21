#include "parser.h"

Parser::Parser(std::vector<Token> tokens) {
	m_tokenList = tokens;
	m_currentTokenIndex = 0;
}

Parser::~Parser() {
	delete rootNode;
}

TreeNode* Parser::getAST() {
	return rootNode;
}

Token Parser::getCurrent() { //Вернуть текущий токен
	if (m_currentTokenIndex >= m_tokenList.size()) { //Если токены кончились
		Token t;
		t._type = TokenType::T_EOF; //Добавить EOF-токен
		return t;
	}

	return m_tokenList[m_currentTokenIndex];
}

Token Parser::getNext() { //Вернуть следующий токен
	if (m_currentTokenIndex > m_tokenList.size() - 1) { //Если токены кончились
		Token t;
		t._type = TokenType::T_EOF; //Добавить EOF-токен
		return t;
	}

	return m_tokenList[m_currentTokenIndex++];
}

TokenType Parser::currentType() { //Тип текущего токена
	return getCurrent().GetType();
}

void Parser::printTree(TreeNode* node) {
	std::cout << std::endl << std::endl;
	if (node == NULL)
		if (rootNode != NULL)
			node = rootNode;
		else
			std::cout << "ooops!" << std::endl;

	node->printNode(0);
	std::cout << std::endl;
	std::cout << std::endl;
}

void Parser::parse() { //Парсинг
	rootNode = new TreeNode(NodeType::NODE_COMPILATION_UNIT); //Создать узел COMPILATION_UNIT

	while (currentType() != TokenType::T_EOF) { //Пока не EOF-токен
		rootNode->addChild(parse_compilation_unit()); //Проверка типа функции и ее парсинг, добавление результата (узла FUNCTIONDECL) в качестве потомка COMPILATION_UNIT
	}
}

TreeNode* Parser::parse_compilation_unit() { //Проверка типа функции и ее парсинг
	if (currentType() == T_VOID || currentType() == T_INT || currentType() == T_CHAR || currentType() == T_IDENTIFIER) {
		return parse_function_declaration();
	}

	return nullptr;
}

TreeNode* Parser::parse_function_declaration() { //Парсинг функции
	TreeNode* node = new TreeNode(NodeType::NODE_FUNCTIONDECL); //Создать узел FUNCTIONDECL
	node->addChild(parse_function_header()); //Добавить ветвь инициализации функции (тип, идентификатор, параметры) в качестве потомка к FUNCTIONDECL

	if (currentType() == TokenType::T_SEMICOLON) { //Если текущий токен- точка с запятой
		getNext(); //Вернуть следующий токен
	}
	else if (currentType() == TokenType::T_LBRACE) { //Если текущий токен- открывающая фигурная скобка
		node->addChild(parse_block()); //Добавить к узлу FUNCTIONDECL узел BLOCK с парсингом блока (содержимого фигурных скобок)
	}
	else { //Иначе- ошибка
		std::cout << "Expection function definition block after function header!" << std::endl;
	}

	return node;
}


TreeNode* Parser::parse_function_header() { //Ветвь инициализации функции (тип, идентификатор, параметры)
	TreeNode* functionHeaderNode = new TreeNode(NodeType::NODE_FUNCTIONHEADER); //Создать узел FUNCTIONHEAD
	functionHeaderNode->addChild(parse_function_returntype()); //Добавить к этому узлу узел RETURNTYPE с потомком, содержащим тип функции

	if (currentType() == TokenType::T_IDENTIFIER) { //Если текущий токен- идентификатор функции
		functionHeaderNode->addChild(new TreeNode(NodeType::NODE_IDENTIFIER, getCurrent())); //Создать узел из токена-идентификатора функции и добавить его в качестве потомка

		getNext(); //Вернуть следующий токен

		if (getCurrent().GetType() == TokenType::T_LPAREN) { //Если это открывающаяся скобка
			getNext(); //Вернуть следующий токен
			functionHeaderNode->addChild(parse_function_param_list()); //Создать узел PARAMLIST и его потомка (узел из параметров функции) и добавить его в качестве потомка
		}
		else {
			std::cout << "Expecting '(' after function identifier." << std::endl;
		}
	}
	else { //Если текущий токен- не идентификатор функции, то ошибка
		std::cout << "Expecting function identifier." << std::endl;
	}

	return functionHeaderNode;
}

TreeNode* Parser::parse_function_returntype() { //Вернуть тип функции в виде узла (содержащего этот тип), являющегося потомком созданного узла RETURNTYPE (возвращается созданный узел)
	TreeNode* node = new TreeNode(NodeType::NODE_RETURNTYPE); //Создать узел RETURNTYPE
	node->addChild(new TreeNode(getNodeFromToken(getCurrent()))); //Добавить к узлу RETURNTYPE узел, с типом текущего токена, в виде потомка 
	getNext(); //Вернуть следующий токен

	return node;
}

TreeNode* Parser::parse_function_args() {
	TreeNode* node;

	getNext();
	node = new TreeNode(NodeType::NODE_FUNCARGS);

	bool _loop = false;

	if (currentType() != TokenType::T_RPAREN) {
		do {
			TreeNode* nodeExpr = parse_expression();
			if (nodeExpr->m_nodeType == NodeType::NODE_FACTOR)
				nodeExpr = nodeExpr->getChild(0);

			node->addChild(nodeExpr);

			_loop = (currentType() == TokenType::T_COMMA);

			if (_loop)
				getNext();
		} while (_loop);
	}

	if (currentType() != TokenType::T_RPAREN) {
		std::cout << "Error: Expecting ')' after argument list!" << std::endl;
	}

	getNext();

	return node;
}

TreeNode* Parser::parse_param() { //Вернуть параметры
	TreeNode* node = new TreeNode(NodeType::NODE_PARAM); //Создать узел PARAM

	if (currentType() == T_INT || currentType() == T_CHAR) { //Если тип параметров int или char
		node->addChild(new TreeNode(getNodeFromToken(getCurrent()))); //Добавить тип параметра в качестве потомка PARAM
	}
	else //Иначе (не int или char) ошибка
		std::cout << "Expecting valid data type in parameter list" << std::endl;


	getNext(); //Вернуть следующий токен
	//TreeNode* nodeReference = parse_reference();
	//getNext();

	if (currentType() != T_IDENTIFIER) { //Если после типа параметра не следует идентификатор параметра
		std::cout << "Expecting identifier in param list" << std::endl; //Ошибка
		exit(0);
	}
	//Если после типа параметра следует идентификатор параметра
	node->addChild(new TreeNode(NodeType::NODE_IDENTIFIER, getCurrent())); //Добавить идентификатор параметра в качестве потомка PARAM
	//ast->AddChild(node, nodeReference);
	getNext(); //Вернуть следующий токен
	//ast->AddChild(node, parse_dimension_block());

	return node;
}

TreeNode* Parser::parse_function_param_list() {
	TreeNode* node = new TreeNode(NodeType::NODE_PARAMLIST); //Создать узел PARAMLIST

	if (currentType() == TokenType::T_VOID || currentType() == TokenType::T_RPAREN) { //Если у функции нет параметров
		node->addChild(new TreeNode(NodeType::NODE_VOID)); //Добавить к узлу PARAMLIST узел VOID в виде потомка
		getNext(); //Вернуть следующий токен
		return node;
	}
	//Иначе (если у функции есть параметры)
	TokenType tokenType = TokenType::T_UNKNOWN;
	bool loop = true;

	while (loop) {
		tokenType = currentType();

		node->addChild(parse_param()); //добавить в узлу PARAMLIST параметры в виде потомков
		loop = (currentType() == (T_COMMA));
		getNext();
	}

	return node;
}

TreeNode* Parser::parse_block() { //Парсинг блока (содержимого фигурных скобок)
	TreeNode* node = new TreeNode(NodeType::NODE_BLOCK); //Создать узел BLOCK

	//getNext();

	if (currentType() == TokenType::T_LBRACE) { //Если текущий токен- открывающая фигурная скобка
		getNext(); //Вернуть следующий токен

		while (currentType() != TokenType::T_RBRACE) { //Пока не дошли до закрывающей фигурной скобки
			if (currentType() == TokenType::T_ASM) { //Если токен- ассемблер
				node->addChild(parse_assembly()); //Парсинг ассемблера
			}
			else {
				node->addChild(parse_statement()); //Добавить узел STATEMENT (с полным парсингом блока функции) в качестве потомка узла BLOCK
			}
		}
	}
	else { //Если текущий токен- не открывающая фигурная скобка
		std::cout << "A code block has to start with '{'." << std::endl;
	}

	getNext(); //Вернуть следующий токен

	return node;
}


TreeNode* Parser::parse_assembly() { //Парсинг ассемблера
	TreeNode* node = new TreeNode(NodeType::NODE_ASSEMBLY);
	TreeNode* result = NULL;

	getNext();
	if (currentType() == TokenType::T_LBRACE) {
		while (currentType() != TokenType::T_RBRACE) {
			getNext();
			if (currentType() == TokenType::T_STRING)
				node->addChild(new TreeNode(NodeType::NODE_ASSEMBLY_LINE, getCurrent()));
		}
	}
	else {
		std::cout << "Error: Inline assembly block must begin with '{'!" << std::endl;
	}

	getNext();

	return node;
}


TreeNode* Parser::parse_statement() { //Полный парсинг кода в блоке функции
	TreeNode* node = new TreeNode(NodeType::NODE_STATEMENT); //Создать узел STATEMENT

	/*if (currentType() == TokenType::T_IDENTIFIER)
	{
	}
	else */
	if (currentType() == TokenType::T_SEMICOLON) { //Если текущий токен- точка с запятой
		getNext(); //Вернуть следующий токен
		return nullptr;
	}
	/*else if (currentType() == TokenType::T_WHILE) { //Если текущий токен- WHILE (цикл) !!!УБРАТЬ!!!

	}*/
	else if (currentType() == TokenType::T_INT) { //Если текущий токен- int
		node->addChild(parse_declaration_statement()); //Добавить узел DECLBLOCK (с парсингов кода в блоке функции) в качестве потомка узла STATEMENT
	}
	else if (currentType() == TokenType::T_PRINT) {
		node->addChild(parse_print());
	}
	else if (currentType() == TokenType::T_IF) {
		node->addChild(parse_if());
	}
	else {
		node->addChild(parse_expression());
	}

	return node;
}

TreeNode* Parser::parse_if() {
	TreeNode* node = new TreeNode(NodeType::NODE_IF);
	getNext();
	getNext();

	if (currentType() == TokenType::T_IDENTIFIER || currentType() == TokenType::T_DIGIT) {
		node->addChild(parse_expression());
	}

	getNext();

	if (currentType() == TokenType::T_LBRACE) {
		//getNext();
		node->addChild(parse_block());
	}
	else if (currentType() == TokenType::T_IDENTIFIER) {
		getNext();
		node->addChild(parse_statement());
	}

	if (currentType() == TokenType::T_ELSE) {
		node->addChild(parse_else());
	}
	return node;
}

TreeNode* Parser::parse_else() {
	TreeNode* node = new TreeNode(NodeType::NODE_ELSE);
	getNext();
	if (currentType() == TokenType::T_LBRACE) {
		//getNext();
		node->addChild(parse_block());
	}
	else if (currentType() == TokenType::T_IDENTIFIER) {
		//getNext();
		node->addChild(parse_statement());
	}
	return node;
}

TreeNode* Parser::parse_print() {
	TreeNode* node = new TreeNode(NodeType::NODE_PRINT);

	getNext();
	getNext();

	if (currentType() == TokenType::T_STRING) {
		node->addChild(new TreeNode(NodeType::NODE_STRING, getCurrent()));
	}
	else if (currentType() == TokenType::T_IDENTIFIER || currentType() == TokenType::T_DIGIT) {
		node->addChild(parse_expression());
	}
	
	getNext();
	return node;
}

TreeNode* Parser::parse_declaration_statement() { //Парсинг кода в блоке функции
	TreeNode* node = new TreeNode(NodeType::NODE_DECLARATION); //Создал узел DECLBLOCK

	node->addChild(new TreeNode(getNodeFromToken(getCurrent()))); //Добавить в узлу DECLARATION узел, с типом текущего токена, в виде потомка

	getNext(); //Вернуть следующий токен

	if (currentType() == TokenType::T_IDENTIFIER) { //Если текущий токен- идентификатор
		node->addChild(new TreeNode(NodeType::NODE_IDENTIFIER, getCurrent())); //Добавить к узлу DECLARATION узел-идентификатор с value = имя идентификатора в качестве потомка
		getNext(); //Вернуть следующий токен
		node->addChild(parse_initializer()); //Добавить к узлу DECLARATION узел с парсингом инициализации в качестве потомка
	}
	else { //Если следующий токен- не идентификатор- ошибка
		std::cout << "Expecting identifier after type specifier!" << std::endl;
	}

	return node;
}

TreeNode* Parser::parse_expression() { //Парсинг выражения
	TreeNode* node = parse_expression_relation(); //Самый полный парсинг выражения (с учетом сравнения)
	TreeNode* opNode = nullptr;

	if (currentType() != TokenType::T_OP_ASSIGN) { //Если текущий токен- не присваивание ("=")
		//getNext();
		return node;
	}
	else { //Если текущий токен- присваивание ("=")
		opNode = new TreeNode(getNodeFromToken(getCurrent())); //Создать узел с типом текущего токена ("=" ASSIGN)

		getNext(); //Вернуть следующий токен

		opNode->addChild(node); //Добавить к узлу присваивания (ASSIGN) выражение в качестве потомка
		opNode->addChild(parse_expression()); //Добавить к узлу присваивания (ASSIGN) рекурсивный вызов парсинга выражения в качестве потомка

		return opNode;
	}

	return opNode;
}

TreeNode* Parser::parse_initializer() { //Парсинг инициализации
	TreeNode* node = nullptr;

	if (currentType() == TokenType::T_OP_ASSIGN) { //Если текущий токен- присваивание ("=")
		TreeNode* expressionNode = parse_expression(); //Создать узел и поместить в него парсинг выражения

		if (expressionNode != nullptr) { //Если выражение имеется
			node = new TreeNode(NodeType::NODE_INITIALIZER); //Создать узел INITIALIZER
			node->addChild(expressionNode);//Добавить к нему узел выражения в качестве потомка
		}
	}

	return node;
}

TreeNode* Parser::parse_expression_relation() { //Самый полный парсинг выражения (с учетом сравнения)
	TreeNode* node = parse_expression_addition(); //Создать узел и поместить в него полный парсинг выражения
	TreeNode* opNode = nullptr;

	while (currentType() == TokenType::T_OP_GREATER ||
		currentType() == TokenType::T_OP_GREATEREQUAL ||
		currentType() == TokenType::T_OP_LESS ||
		currentType() == TokenType::T_OP_LESSEQUAL) { //Пока токен- ">",">=","<","<="
		opNode = new TreeNode(getNodeFromToken(getCurrent())); //Создать узел с типом токена
		opNode->addChild(node); //Добавить к созданному узлу (с типом сравнения) потомка в виде выражения (левого, относительно знака сравнения)

		getNext(); //Вернуть следующий токен

		opNode->addChild(parse_expression_addition()); //Добавить к созданному узлу (с типом сравнения) потомка в виде выражения (правого, относительно знака сравнения)
		node = opNode; //Хранить все выражение в node
	}

	return node;
}

TreeNode* Parser::parse_expression_addition() { //Полный парсинг выражения
	TreeNode* node;
	TreeNode* opNode;

	node = parse_expression_multiplication(); //Создать узел и поместить в него парсинг выражения

	while (currentType() == TokenType::T_OP_BINARY_ADD || currentType() == TokenType::T_OP_BINARY_SUBSTRACT) { //Если текущий токен- сложение или вычитание
		opNode = new TreeNode(getNodeFromToken(getCurrent()), getCurrent()); //Создать узел с типом и значением токена
		opNode->addChild(node); //Добавить к созданному узлу парсинг выражения в виде потомка

		getNext(); //Вернуть следующий токен
		opNode->addChild(parse_expression_multiplication()); //Продожить парсинг выражения и добавить результаты также в качестве потомка
		node = opNode; //Хранить все выражение в node
	}

	return node;
}

TreeNode* Parser::parse_expression_multiplication() { //Парсинг выражения
	TreeNode* child;
	TreeNode* op;

	child = parse_unary2(); //Парсинг действия выражения

	while (currentType() == TokenType::T_STAR || currentType() == TokenType::T_OP_DIVIDE || currentType() == TokenType::T_MOD) { //Пока токен- умножение, целочисленное деление или взятие остатка 
		op = new TreeNode(getNodeFromToken(getCurrent()), getCurrent()); //Создать узел с типом и значением токена
		op->addChild(child); //Добавить к созданному узлу парсинг действия выражения в виде потомка

		getNext(); //Вернуть следующий токен

		op->addChild(parse_unary2()); //Продожить парсинг действий выражения и добавить результаты также в качестве потомка
		child = op; //Хранить все выражение в child
	}

	return child;
}

TreeNode* Parser::parse_unary1() { //Парсинг переменной/функции/числа
	TreeNode* node;
	TreeNode* child;

	node = parse_factor(); //Парсинг идентификатора (переменной, функции) или числа

	while (currentType() == TokenType::T_LPAREN || currentType() == TokenType::T_LBRACKET) { //Пока токен- открывающаяся круглая или квадратная скобка
		if (currentType() == T_LPAREN) { //Если токен- открывающаяся круглая скобка
			child = node;
			node = new TreeNode(NodeType::NODE_FUNCCALL); //Создать узел FUNCCALL
			node->addChild(child->getChild(0)); //Добавить к узлу коэффициента потомка FUNCCALL

			if (currentType() != TokenType::T_RPAREN) { //Если токен- закрывающаяся круглая скобка
				TreeNode* nodeArgs = parse_function_args(); //Создать узел из парсинга аргументов вызываемой функции
				if (nodeArgs->m_children.size() > 0) //Если есть аргументы
					node->addChild(nodeArgs); //Добавить их в качестве потомка к узлу FUNCCALL
			}
		}

		if (currentType() == TokenType::T_LBRACKET) { //Если токен- открывающаяся квадратная скобка
			getNext(); //Вернуть следующий токен

			child = node;
			node = new TreeNode(NodeType::NODE_INDEXER); //Создать узел INDEXER
			node->addChild(child); //Добавить у узлу INDEXER узел с парсингом идентификатора в качестве потомка
			node->addChild(parse_expression());

			if (currentType() != TokenType::T_RBRACKET) { //Если токен- не закрывающаяся квадратная скобка
				std::cout << "Error: Expecting ']' after array index expression." << std::endl;
				exit(0);
			}
			getNext(); //Вернуть следующий токен
		}
	}

	return node;
}

TreeNode* Parser::parse_unary2() { //Парсинг действия выражения

	TreeNode* node = nullptr; // = parse_unary1();

	if (currentType() == TokenType::T_OP_BINARY_ADD || currentType() == TokenType::T_OP_BINARY_SUBSTRACT) { //Если текущий токен- сложение или вычитание
		switch (currentType()) {
		case T_OP_BINARY_ADD: //Сложение
			node = new TreeNode(NodeType::NODE_UNARY_ADD); //Создать узел сложения ADD
			break;
		case T_OP_BINARY_SUBSTRACT: //Вычитание
			node = new TreeNode(NodeType::NODE_UNARY_SUBSTRACT); //Создать узел вычитания SUBSTRACT
			break;
		}

		getNext(); //Вернуть следующий токен
		node->addChild(parse_unary2()); //добавить к созданному узлу потомка (переменную/функцию/число) через рекурсивный вызов парсинга действия выражения
	}
	else
		node = parse_unary1(); //Парсинг переменной/функции/числа

	return node;
}

TreeNode* Parser::parse_factor() { //Парсинг идентификатора (переменной, функции) или числа
	TreeNode* node = new TreeNode(NodeType::NODE_FACTOR); //Создать узел FACTOR

	if (currentType() == TokenType::T_LPAREN) { //Если токен- открывающаяся круглая скобка
		getNext(); //Вернуть следующий токен
		node->addChild(parse_expression());
		getNext(); //Вернуть следующий токен
	}
	else if (currentType() == TokenType::T_IDENTIFIER) { //Если токен- идентификатор
		node->addChild(new TreeNode(NodeType::NODE_IDENTIFIER, getCurrent())); //Добавить к узлу FACTOR узел IDENTIFIER с value токена
		getNext(); //Вернуть следующий токен
	}
	else if (currentType() == TokenType::T_DIGIT ||
		currentType() == TokenType::T_STRING ||
		currentType() == TokenType::T_CHAR) { //Если токен- число, строка или символ
		switch (currentType()) {
		case TokenType::T_DIGIT: //Если токен- число
			node->addChild(new TreeNode(NodeType::NODE_DIGIT, getCurrent())); //Добавить к узлу FACTOR узел DIGIT с value токена
			break;
			//!!!Добавить для char!!!
		}

		getNext(); //Вернуть следующий токен
	}

	if (node->m_children.size() <= 0) //Если к узлу FACTOR не добавлено потомков
		node = nullptr;

	return node;
}