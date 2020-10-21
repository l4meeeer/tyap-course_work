#ifndef CODEGEN_H_
#define CODEGEN_H_

#include "scope.h"
#include <vector>

class CodeGen {
public:
	CodeGen() {}

	~CodeGen() {
		delete _globalScope;
	}

	void GenCode(TreeNode* node) {
		rootNodeAST = node;

		_globalScope = new Scope();
		_currentScope = _globalScope;
		compileNode(rootNodeAST);

		_currentEmitIndex = 0;
		_endEmitIndex = 0;
	}

	void Emit(const std::string& code) {
		_emittedCode.push_back(code);
	}

	void BeginEmit() {
		_currentEmitIndex = _emittedCode.size();
	}

	void EndEmit() {
		_endEmitIndex = _emittedCode.size() - 1;
	}

	void pasteEmittedCodeHere() {
		std::vector<std::string> tmpEmit;
		std::vector<std::string> new_emittedCodeList;

		for (unsigned int i = 0; i < _emittedCode.size(); ++i) {
			if (i >= _currentEmitIndex && i <= _endEmitIndex)
				continue;

			new_emittedCodeList.push_back(_emittedCode[i]);
		}

		for (int i = _currentEmitIndex; i < _endEmitIndex + 1; ++i)
			new_emittedCodeList.push_back(_emittedCode.at(i));

		_emittedCode = new_emittedCodeList;
	}

	void printGeneratedCode() {
		for (std::string s : _emittedCode)
			std::cout << s << std::endl;
	}

private:
	void compileNode(TreeNode* node) {
		int i = 0;
		switch (node->m_nodeType) {
		case NodeType::NODE_COMPILATION_UNIT:
			Emit(".data ;--------------------------");
			Emit("string db 12 dup(0)");
			Emit("len_string=$-string/n");
			Emit("ten dd 10");
			Emit(".code ;--------------------------");
			Emit("print_eax: ;---------------------");
			Emit("lea si,string");
			Emit("cld");
			Emit("cmp eax,0");
			Emit("jge cont");
			Emit("neg eax");
			Emit("push eax");
			Emit("mov ah,2");
			Emit("mov dl,'-'");
			Emit("int 21h");
			Emit("pop eax");
			Emit("cont: ;--------------------------");
			Emit("xor edx,edx");
			Emit("div ten");
			Emit("or dl,30h");
			Emit("mov [si],dl");
			Emit("inc si");
			Emit("inc ecx");
			Emit("cmp eax,0");
			Emit("jne cont");
			Emit("mov ah,2");
			Emit("std");
			Emit("dec si");
			Emit("m3: ;----------------------------");
			Emit("lodsb");
			Emit("mov dl,al");
			Emit("int 21h");
			Emit("loop m3");
			Emit("ret");
			for (TreeNode* child : node->m_children)
				compileNode(child);
			break;

		case NodeType::NODE_FUNCTIONDECL:

			
			//Извлечь имя и тип функции
			_currentScope->addSymbol(new Symbol(node->getChild(0)->getChildIdentifier(1), SymbolType::FUNCTION, node->getChild(0)->getChild(0)->getChild(0)->m_nodeType));
			Emit("_" + _currentScope->symbolTable->getSymbolbyName(node->getChild(0)->getChildIdentifier(1))->_symbolName + ":");


			//Комфиляция блока кода
			BeginEmit();
			_currentScope = _currentScope->createScope();
			compileNode(node->getChild(0));
			compileNode(node->getChild(1));
			EndEmit();

			Emit("push ebp");																	//Сохранить указатель стека
			Emit("mov ebp, esp");																//ebp теперь указывает на вершину стека
			Emit("sub esp, " + std::to_string(_currentScope->symbolTable->getSize() * 4));    //Пространство выделенное в стеке для локальных переменных

			pasteEmittedCodeHere();

			Emit("mov esp, ebp");
			Emit("pop ebp");
			Emit("ret");

			_currentScope = _currentScope->leaveScope();

			break;

		case NodeType::NODE_BLOCK:
			for (TreeNode* child : node->m_children)
				compileNode(child);
			break;

		case NodeType::NODE_STATEMENT:
			for (TreeNode* child : node->m_children)
				compileNode(child);
			break;


		case NodeType::NODE_DECLARATION:
			_currentScope->addSymbol(new Symbol(node->getChild(1)->m_token._identifier));

			for (TreeNode* child : node->m_children)
				compileNode(child);

			//Emit("mov [ebp-" + std::to_string(_currentScope->symbolTable->getSymbolbyName(node->getChild(1)->m_token._identifier)->_address) + "], eax");

			break;

		case NodeType::NODE_OP_ASSIGN:
			if (node->getParent()->m_nodeType == NodeType::NODE_INITIALIZER) {
				if (node->getChild(0)->m_nodeType == NodeType::NODE_FACTOR) {
					if (node->getChild(0)->getChild(0)->m_nodeType == NodeType::NODE_DIGIT) {
						Emit("mov [ebp-" + std::to_string(_currentScope->symbolTable->getSymbolbyName(node->getParent()->getParent()->getChild(1)->m_token._identifier)->_address) + "], " + node->getChild(0)->getChild(0)->m_token.getStringValue());
					}
					else if (node->getChild(0)->getChild(0)->m_nodeType == NodeType::NODE_IDENTIFIER) {
						Emit("mov eax, [ebp-" + std::to_string(_currentScope->symbolTable->getSymbolbyName(node->getChild(0)->getChild(0)->m_token._identifier)->_address) + "]");
						Emit("mov [ebp-" + std::to_string(_currentScope->symbolTable->getSymbolbyName(node->getParent()->getParent()->getChild(1)->m_token._identifier)->_address) + "], eax");
					}
				}
				else {
					compileNode(node->getChild(0));
					Emit("mov [ebp-" + std::to_string(_currentScope->symbolTable->getSymbolbyName(node->getParent()->getParent()->getChild(1)->m_token._identifier)->_address) + "], eax");
				}
			}
			else if (node->getParent()->m_nodeType == NodeType::NODE_STATEMENT) {
				if (node->getChild(1)->m_nodeType == NodeType::NODE_FACTOR) {
					if (node->getChild(1)->getChild(0)->m_nodeType == NodeType::NODE_DIGIT) {
						Emit("mov [ebp-" + std::to_string(_currentScope->symbolTable->getSymbolbyName(node->getChild(0)->getChild(0)->m_token._identifier)->_address) + "], " + node->getChild(1)->getChild(0)->m_token.getStringValue());
					}
					else if (node->getChild(1)->getChild(0)->m_nodeType == NodeType::NODE_IDENTIFIER) {
						Emit("mov eax, [ebp-" + std::to_string(_currentScope->symbolTable->getSymbolbyName(node->getChild(1)->getChild(0)->m_token._identifier)->_address) + "]");
						Emit("mov [ebp-" + std::to_string(_currentScope->symbolTable->getSymbolbyName(node->getChild(0)->getChild(0)->m_token._identifier)->_address) + "], eax");
					}
				}
				else {
					compileNode(node->getChild(1));
					Emit("mov [ebp-" + std::to_string(_currentScope->symbolTable->getSymbolbyName(node->getChild(0)->getChild(0)->m_token._identifier)->_address) + "], eax");
				}
			}

			
			break;

		case NodeType::NODE_INITIALIZER:
			compileNode(node->getChild(0));
			break;

		case NODE_ASSEMBLY_LINE:
			Emit(node->m_token._identifier);
			break;


		case NodeType::NODE_BINARY_ADD:

			if (node->getChild(0)->m_nodeType == NodeType::NODE_FACTOR) {
				if (node->getChild(0)->getChild(0)->m_nodeType == NodeType::NODE_DIGIT) {
					Emit("mov eax, " + node->getChild(0)->getChild(0)->m_token.getStringValue());
				}
				else if (node->getChild(0)->getChild(0)->m_nodeType == NodeType::NODE_IDENTIFIER) {
					Emit("mov eax, [ebp-" + std::to_string(_currentScope->symbolTable->getSymbolbyName(node->getChild(0)->getChild(0)->m_token._identifier)->_address) + "]");
				}
			}
			else {
				compileNode(node->getChild(0));
			}

			if (node->getChild(1)->m_nodeType == NodeType::NODE_FACTOR) {
				if (node->getChild(1)->getChild(0)->m_nodeType == NodeType::NODE_DIGIT) {
					Emit("mov ebx, " + node->getChild(1)->getChild(0)->m_token.getStringValue());
				}
				else if (node->getChild(1)->getChild(0)->m_nodeType == NodeType::NODE_IDENTIFIER) {
					Emit("mov ebx, [ebp-" + std::to_string(_currentScope->symbolTable->getSymbolbyName(node->getChild(1)->getChild(0)->m_token._identifier)->_address) + "]");
				}
			}
			else {
				compileNode(node->getChild(1));
			}

			Emit("add eax, ebx");

			break;

		case NodeType::NODE_PRINT:
			if (node->getChild(0)->m_nodeType == NodeType::NODE_STRING) {
				Emit("mov eax, " + node->getChild(0)->m_token.getStringValue());
				Emit("call print_eax");
			}
			else if (node->getChild(0)->getChild(0)->m_nodeType == NodeType::NODE_DIGIT) {
				Emit("mov eax, " + node->getChild(0)->getChild(0)->m_token.getStringValue());
				Emit("call print_eax");
			}
			else if (node->getChild(0)->getChild(0)->m_nodeType == NodeType::NODE_IDENTIFIER) {
				Emit("mov eax, [ebp-" + std::to_string(_currentScope->symbolTable->getSymbolbyName(node->getChild(0)->getChild(0)->m_token._identifier)->_address) + "]");
				Emit("call print_eax");
			}
			else {
				compileNode(node->getChild(0));
				Emit("call print_eax");
			}
			break;

		case NodeType::NODE_IF:
			if (node->getChild(0)->getChild(0)->getChild(0)->m_nodeType == NodeType::NODE_IDENTIFIER && node->getChild(0)->getChild(1)->getChild(0)->m_nodeType == NodeType::NODE_IDENTIFIER) {
				Emit("mov eax, [ebp-" + std::to_string(_currentScope->symbolTable->getSymbolbyName(node->getChild(0)->getChild(0)->getChild(0)->m_token._identifier)->_address) + "]");
				Emit("cmp eax, [ebp-" + std::to_string(_currentScope->symbolTable->getSymbolbyName(node->getChild(0)->getChild(1)->getChild(0)->m_token._identifier)->_address) + "]");
			}
			else if (node->getChild(0)->getChild(0)->getChild(0)->m_nodeType == NodeType::NODE_IDENTIFIER && node->getChild(0)->getChild(1)->getChild(0)->m_nodeType == NodeType::NODE_DIGIT) {
				Emit("cmp [ebp-" + std::to_string(_currentScope->symbolTable->getSymbolbyName(node->getChild(0)->getChild(0)->getChild(0)->m_token._identifier)->_address) + "], " + node->getChild(0)->getChild(1)->getChild(0)->m_token.getStringValue());
			}
			else if (node->getChild(0)->getChild(0)->getChild(0)->m_nodeType == NodeType::NODE_DIGIT && node->getChild(0)->getChild(1)->getChild(0)->m_nodeType == NodeType::NODE_IDENTIFIER) {
				Emit("cmp [ebp-" + std::to_string(_currentScope->symbolTable->getSymbolbyName(node->getChild(0)->getChild(0)->getChild(0)->m_token._identifier)->_address) + "], " + node->getChild(0)->getChild(1)->getChild(0)->m_token.getStringValue());
			}

			if (node->getChild(0)->m_nodeType == NodeType::NODE_GREATER) {
				Emit("jle .L2");
			}
			else if (node->getChild(0)->m_nodeType == NodeType::NODE_GREATEREQUAL) {
				Emit("jl .L2");
			}
			else if (node->getChild(0)->m_nodeType == NodeType::NODE_LESSEQUAL) {
				Emit("jg .L2");
			}
			else if (node->getChild(0)->m_nodeType == NodeType::NODE_LESS) {
				Emit("jge .L2");
			}
			else if (node->getChild(0)->m_nodeType == NodeType::NODE_EQUALITY) {
				Emit("jne .L2");
			}
			else if (node->getChild(0)->m_nodeType == NodeType::NODE_INEQUALITY) {
				Emit("je .L2");
			}

			if (node->getChild(1)->m_nodeType == NodeType::NODE_BLOCK) {
				compileNode(node->getChild(1));
			}

			if (node->getChild(2) != nullptr) {
				Emit("jmp .L3");
				Emit(".L2: ");
				compileNode(node->getChild(2)->getChild(0));
				Emit(".L3: ");
			}
			else {
				Emit(".L2: ");
			}

			break;

		case NodeType::NODE_PARAMLIST:
			//for (TreeNode* child : node->m_children) {
			if (node->getChild(0)->m_nodeType != NodeType::NODE_VOID) {
				i = 0;
				while (node->getChild(i) != nullptr)
					if (node->getChild(i)->getChild(1)->m_nodeType == NodeType::NODE_IDENTIFIER) {
						_currentScope->addSymbol(new Symbol(node->getChild(i)->getChild(1)->m_token._identifier));
						i++;
					}
			}
			//}
			break;

		case NodeType::NODE_MULTIPLICATION:
			if (node->getChild(0)->m_nodeType == NodeType::NODE_FACTOR) {
				if (node->getChild(0)->getChild(0)->m_nodeType == NodeType::NODE_DIGIT) {
					Emit("mov eax, " + node->getChild(0)->getChild(0)->m_token.getStringValue());
				}
				else if (node->getChild(0)->getChild(0)->m_nodeType == NodeType::NODE_IDENTIFIER) {
					Emit("mov eax, [ebp-" + std::to_string(_currentScope->symbolTable->getSymbolbyName(node->getChild(0)->getChild(0)->m_token._identifier)->_address) + "]");
				}
			}
			else {
				compileNode(node->getChild(0));
			}

			if (node->getChild(1)->m_nodeType == NodeType::NODE_FACTOR) {
				if (node->getChild(1)->getChild(0)->m_nodeType == NodeType::NODE_DIGIT) {
					Emit("mov ebx, " + node->getChild(1)->getChild(0)->m_token.getStringValue());
				}
				else if (node->getChild(1)->getChild(0)->m_nodeType == NodeType::NODE_IDENTIFIER) {
					Emit("mov ebx, [ebp-" + std::to_string(_currentScope->symbolTable->getSymbolbyName(node->getChild(1)->getChild(0)->m_token._identifier)->_address) + "]");
				}
			}
			else {
				compileNode(node->getChild(1));
			}

			Emit("mul ebx");
			break;

		case NodeType::NODE_FUNCARGS:
			for (TreeNode* child : node->m_children) {
				if (child->m_nodeType == NodeType::NODE_IDENTIFIER) {
					Symbol* s = _currentScope->symbolTable->getSymbolbyName(child->m_token._identifier);
					Emit("mov eax, [ebp-" + std::to_string(s->_address) + "]");
					Emit("push eax");
				}
				else if (child->m_nodeType == NodeType::NODE_DIGIT) {
					Emit("push " + std::to_string(child->m_token._value.uintvalue));
				}
			}
			break;

		case NodeType::NODE_FUNCCALL: {
			Symbol* s = _currentScope->parentScope->symbolTable->getSymbolbyName(node->getChildIdentifier(0));

			//Рекурсивный вызов, если s сейчас не null
			if (s == nullptr)
				s = _currentScope->symbolTable->getSymbolbyName(node->getChildIdentifier(0));

			if (node->getChild(1)->m_nodeType == NodeType::NODE_FUNCARGS) {
				compileNode(node->getChild(1));
			}

			//Функция не найдена
			if (s == nullptr) {
				std::cout << "Function '" + node->getChildIdentifier(0) + "' not declared!" << std::endl;
				return;
			}

			Emit("call _" + s->_symbolName);
		}
									break;

		default:
			for (TreeNode* child : node->m_children)
				compileNode(child);
			break;
		}
	}

	TreeNode* rootNodeAST;
	Scope* _globalScope;
	Scope* _currentScope;

	std::vector<std::string> _emittedCode;
	unsigned int _currentEmitIndex, _endEmitIndex;
};

#endif