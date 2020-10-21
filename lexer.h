#ifndef __LEXER_H_
#define __LEXER_H_

#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include "tokens.h"

class Lexer {
public:
	Lexer();
	Lexer(const std::string& _input);
	virtual ~Lexer();

	std::vector<Token> LexicalAnalyisis(); //Лексический анализ (всего входного потока)

	void		setInput(const std::string& _input);
	void		next(); //Сдвиг позиции для считывания
	char		getCurrentChar(); //Возврат символа с текущей позиции

	Token		getTokenProto(); //Определение и возврат токена
	Token		getToken(); //Возврат текущего токена (типа и значения)
	Token		getCurrentToken();

private:
	int			m_currentCharPos;
	std::string m_inputString;
	Token		m_currentToken;
};

#endif
