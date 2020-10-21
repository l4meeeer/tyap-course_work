#include "lexer.h"
#include <cctype>

Lexer::Lexer() {
	m_inputString = "";
	m_currentCharPos = 0;
}

Lexer::Lexer(const std::string& _input) : m_inputString(_input), m_currentCharPos(0) {}

Lexer::~Lexer() {}

void Lexer::setInput(const std::string& _input) {
	m_inputString = _input;
}

char Lexer::getCurrentChar() { //Возврат символа с текущей позиции
	return m_inputString[m_currentCharPos];
}

void Lexer::next() { //Сдвиг позиции для считывания
	m_currentCharPos++;
}

Token Lexer::getToken() { //Возврат текущего токена (типа и значения)
	m_currentToken = getTokenProto(); //Тип токена
	if (m_currentToken._type == -1) { //Ошибка
		std::cout << "Error: Unknown token" << std::endl;
	}
	return m_currentToken;
}

Token Lexer::getTokenProto() { //Тип токена
	Token token;

	if ((size_t)m_currentCharPos >= m_inputString.size()) { //EOF
		token._type = T_EOF;
		return token;
	}

	while (std::isspace(getCurrentChar())) //'\f','\n','\r','\t','\v' Пробел, таб, перевод строки
		next();

	if (getCurrentChar() == '=' && m_inputString[m_currentCharPos + 1] == '=') { //Равно
		token._type = T_OP_EQUAL;
		next();
		next();
		return token;
	}

	if (getCurrentChar() == '!' && m_inputString[m_currentCharPos + 1] == '=') { //Не равно
		token._type = T_OP_NOTEQUAL;
		next();
		next();
		return token;
	}

	if (getCurrentChar() == '<' && m_inputString[m_currentCharPos + 1] == '=') {//Меньше или равно
		token._type = T_OP_LESSEQUAL;
		next();
		next();
		return token;
	}

	if (getCurrentChar() == '>' && m_inputString[m_currentCharPos + 1] == '=') {//Больше или равно
		token._type = T_OP_GREATEREQUAL;
		next();
		next();
		return token;
	}

	if (std::isalpha(getCurrentChar()) || getCurrentChar() == '_' || getCurrentChar() == '.') { //Буквы и '_'
		while (std::isalnum(getCurrentChar()) || getCurrentChar() == '_' || getCurrentChar() == '.') { //Пока продолжается имя
			token._identifier += getCurrentChar(); //Строка текущего токена
			next(); //Сдвиг позиции для считывания
		}

		token._type = T_IDENTIFIER; //TokenType

		/*if (token._identifier == "var") // !!!УБРАТЬ!!! ?
			token._type = T_VAR;*/
			/*if (token._identifier == "let") // !!!УБРАТЬ!!! ?
				token._type = T_CONST;*/
		if (token._identifier == "int")
			token._type = T_INT;
		if (token._identifier == "char")
			token._type = T_CHAR;
		if (token._identifier == "void")
			token._type = T_VOID;
		if (token._identifier == "return")
			token._type = T_RETURN;
		if (token._identifier == "while")
			token._type = T_WHILE;
		if (token._identifier == "if")
			token._type = T_IF;
		if (token._identifier == "else")
			token._type = T_ELSE;
		/*if (token._identifier == "asm") // !!!УБРАТЬ!!! ?
			token._type = T_ASM;*/
			/*if (token._identifier == "#asm") // !!!УБРАТЬ!!! ?
				token._type = T_ASM;*/
		if (token._identifier == "global")
			token._type = T_GLOBAL;
		if (token._identifier == "System.out.println")
			token._type = T_PRINT;

		return token;
	}

	if (std::isdigit(getCurrentChar())) { //Цифра
		std::string t_digit;

		while (std::isdigit(getCurrentChar())) { //Пока продолжается число
			t_digit += getCurrentChar(); //Строка текущего числа
			next(); //Сдвиг позиции для считывания
		}

		token._value.uintvalue = std::stoi(t_digit); //Перевод string в int
		token._type = T_DIGIT; //Тип- число

		return token;
	}


	if (getCurrentChar() == '"') { //Кавычки (")
		std::string t_string;
		next();

		while (getCurrentChar() != '"') {
			t_string += getCurrentChar();
			next();

			if (getCurrentChar() == '"')
				break;
		}

		next();

		token._identifier = t_string;
		token._type = T_STRING;
		return token;
	}

	if (getCurrentChar() == 0x27) { //Кавычки (')
		std::string t_string;

		next();
		while (getCurrentChar() != 0x27) {
			t_string += getCurrentChar();
			next();

			if (getCurrentChar() == 0x27)
				break;
		}
		next();

		token._identifier = t_string;
		token._type = T_CHAR;
		return token;
	}

	switch (getCurrentChar()) {
	case ';': token._type = T_SEMICOLON; break;
	case ',': token._type = T_COMMA; break;
	case '|': token._type = T_DISJUNCTION; break;
	case '&': token._type = T_CONJUNCTION; break;
	case '+': token._type = T_OP_BINARY_ADD; break;
	case '*': token._type = T_STAR; break;
	case '/': token._type = T_OP_DIVIDE; break;
	case '-': token._type = T_OP_BINARY_SUBSTRACT; break;
	case '(': token._type = T_LPAREN; break;
	case ')': token._type = T_RPAREN; break;
	case '{': token._type = T_LBRACE; break;
	case '}': token._type = T_RBRACE; break;
	case '[': token._type = T_LBRACKET; break;
	case ']': token._type = T_RBRACKET; break;
	case '>': token._type = T_OP_GREATER; break;
	case '<': token._type = T_OP_LESS; break;
	case '=': token._type = T_OP_ASSIGN; break;
	case '%': token._type = T_MOD; break;
	default: token._type = T_UNKNOWN; break;
	}

	next();

	return token;
}

Token Lexer::getCurrentToken() {
	return m_currentToken;
}

std::vector<Token> Lexer::LexicalAnalyisis() { //Лексический анализ (всего входного потока)
	std::vector<Token> tokenList;

	Token t;
	while ((t = getToken()).GetType() != T_EOF) { //Пока не конец файла
		tokenList.push_back(t); //Дополнять список токенов
	}

	return tokenList;
}