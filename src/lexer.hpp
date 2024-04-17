#pragma once
#include "tokens.hpp"
#include <map>

std::pair<Token *, std::vector<std::string>> lex(char *, int);
void push(struct LexerContext *, enum TokenType ,
          const std::string);

extern std::map<std::string, enum TokenType> keywords;
extern std::map<std::string, enum TokenType> compound_operators;
extern std::map<std::string, enum TokenType> single_operators;