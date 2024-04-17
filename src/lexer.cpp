/* */

// @TODO : We're loosing blocks during calloc, see valgrind.
#pragma once
#include "lexer.hpp"
#include "utils.hpp"
#include <ctype.h>
#include <iostream>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>

static struct Token *head;
static struct Token *tail;
static int current_row;
static int current_col;

static std::vector<std::string> directives;

//@TODO : We don't even use size anymore 
void push(enum TokenType token, const std::string value) {
  logcall();
  struct Token * new_token = new Token();
  new_token->row = current_row;
  new_token->col = current_col;
  new_token->type = token;
  new_token->literal_value = value;

  switch (token) {
  case TokenType_Integer:
    new_token->value = atoi(value.c_str());
    break;
  case TokenType_Floating:
    new_token->value = atof(value.c_str());
    break;
  case TokenType_Character:
  case TokenType_Identifier:
  case TokenType_String: 
    new_token->value = value;
    break;  
  default:
    new_token->value = value;
    break;
  }

  current_col += new_token->literal_value.size();

  // Maintain and update linked-list order
  if (head == nullptr) {
    head = new_token;
    tail = head;
  } else {
    tail->next = new_token;
    new_token->prev = tail;
    tail = new_token;
  }
}

std::map<std::string, enum TokenType> keywords = {

    // Control Flow
    {"if", TokenType_If},
    {"func", TokenType_Func},
    {"else", TokenType_Else},
    {"while", TokenType_While},
    {"for", TokenType_For},

    {"input", TokenType_Input},
    {"print", TokenType_Print},
    {"return", TokenType_Return},
    {"break", TokenType_Break},
    {"continue", TokenType_Continue},
    {"defer", TokenType_Defer},

    // Types
    {"int", TokenType_Int},
    {"str", TokenType_Str},
    {"char", TokenType_Char},
    {"void", TokenType_Void},
    {"float", TokenType_Float},
    {"bool", TokenType_Bool},
    {"enum", TokenType_Enum},
    {"ptr", TokenType_Pointer},

};

std::map<std::string, enum TokenType> compound_operators = {

    // Dual Character Operators
    {"==", TokenType_Eq},   {"!=", TokenType_Neq}, {"<", TokenType_Less},
    {">", TokenType_Great}, {"<=", TokenType_Leq}, {">=", TokenType_Geq},
    {"&&", TokenType_Land}, {"||", TokenType_Lor},
};

std::map<std::string, enum TokenType> single_operators = {
    // Single Character Operators @NOTE : We don't insert '/' here as it needs
    // to be checked for start-of-comment use before being able to assert if its
    // an operator.
    {"+", TokenType_Add},      {"-", TokenType_Sub},
    {"*", TokenType_Star},     {"%", TokenType_Mod},
    {"=", TokenType_Assign},   {"!", TokenType_Bang},
    {"&", TokenType_Amp},      {"<", TokenType_Less},
    {">", TokenType_Great},    {"|", TokenType_Pipe},
    {".", TokenType_Dot},      {",", TokenType_Comma},
    {":", TokenType_Colon},    {"(", TokenType_Lparens},
    {")", TokenType_Rparens},  {"[", TokenType_Lbracket},
    {"]", TokenType_Rbracket}, {"{", TokenType_Lbrace},
    {"}", TokenType_Rbrace},   {";", TokenType_Semicolon},
};

std::pair<Token *, std::vector<std::string>> lex( char *input, int size) {

  debug("Starting lexer\n");

  int cursor = 0;

  while (cursor < size) {
    std::string buffer = "";
    // Check for both single-character and compound operators such as '==', and
    // '&&'
    if (cursor < size - 1) {
      for (auto it = compound_operators.begin(); it != compound_operators.end();
           it++) {
        if (cursor < size - 1 && input[cursor] == it->first[0] &&
            input[cursor + 1] == it->first[1]) {
            debug("Found a compound operator '%s'\n", it->first.c_str());
          push(it->second, it->first.c_str());
          cursor += 2;
          break;
        }
      }
    }

    // Check for single-character operators such as '+', '-', '(', and ')'
    for (auto it = single_operators.begin(); it != single_operators.end();
         it++) {
      if (input[cursor] == it->first[0]) {
        debug("Found a single operator '%s'\n", it->first.c_str());
        push(it->second, it->first.c_str());
        cursor++;
        break;
      }
    }

    // Whitespaces, tabs, newlines, and comments.
    switch (input[cursor]) {

    // @TODO : Gives the wrong column and row number, might be caused in
    // utils.cpp
    case '\n':
      current_row++;
      current_col = 0;
      cursor++;
      break;
    case '\t':
      current_col++;
      cursor++;
      break;
    case '\v':
    case '\r':
    case ' ':
      cursor++;
      current_col++;
      break;

    // Start of comment
    case '/':
      if (cursor + 1 <= size && input[cursor + 1] == '/') {
        while (input[++cursor] != '\n') {
          current_row++;
        }
        current_col = 0;
      }
      // Division operator
      else {
        push(TokenType_Div, "/");
        cursor++;
      }
      break;

    // Compiler directive
    case '#':
      cursor++;
    // until any type of whitespace or newline is found
      while (cursor + 1 <= size && input[cursor] != '\n') {
        buffer += input[cursor++];
      }
      debug("Found a compiler directive '%s'\n", buffer.c_str());
      directives.push_back(buffer);
      break;

    // Start of a string literal
    case '"': {
      cursor++;

    // Handle if it's an empty string
      if (input[cursor] == '"') {
        debug("Found an empty string literal\n");
        push(TokenType_String, std::string(""));
        cursor++;
        break;
      }

      while (cursor + 1 <= size &&
             (input[cursor] != '"' || input[cursor] == '\\')) {
        buffer += input[cursor++];
      }
      cursor++;
      if (input[cursor-1] != '"') {
        err("Unterminated string literal\n");
      }
      debug("Found a string literal '%s'\n", buffer.c_str());
      push(TokenType_String, buffer);
      break;
    }

    // Character Literal
    case '\'': {

      if (input[cursor + 2] != '\'') {
        err("Character literals can only contain one character\n");
      } 
      push(TokenType_Character, std::string(1, input[cursor + 1]));
      debug("Found a character literal %c\n", input[cursor + 1]);
      cursor += 3;
      break;
    }
    default:

      // Check for numeric literals
      while (isdigit(input[cursor]) == 1) {
        buffer.push_back(input[cursor++]);
      }

      if (buffer.size() > 0 && input[cursor] == '.') {
        // Handle the radix
        buffer.push_back(input[++cursor]);

        while (isdigit(input[cursor]) == 1) {
          buffer.push_back(input[++cursor]);
        }
      }

      // Attempt to push an integer literal if one is found
      if (buffer.size() > 0 && buffer.find('.') == std::string::npos){
        debug("Found an integer literal '%s'\n", buffer.c_str());
        push(TokenType_Integer, buffer);
        break;
      }

      // Attempt to push a float literal if one is found
      if (buffer.size() > 0 && buffer.find('.') != std::string::npos){
        debug("Found a float literal '%s'\n", buffer.c_str());
        push(TokenType_Floating, buffer);
        break;
      }

      // Check for Identifiers and Keywords
      while ((buffer.size() > 0 && input[cursor] >= 48 && input[cursor] <= 57) ||
             (input[cursor] >= 65 && input[cursor] <= 90) ||
             (input[cursor] >= 97 && input[cursor] <= 122) ||
             input[cursor] == 95) {
        buffer.push_back(input[cursor++]);
      }

      // Found an Identifier or Keyword
      if (buffer.size() > 0) {
        // Check if it's a keyword
        if (keywords.find(buffer) != keywords.end()) {
          debug("Found a keyword '%s'\n", buffer.c_str());
          push(keywords[buffer], buffer);

        }
        // It's an identifier
        else {
          // check if it starts with an underscore
          if (buffer.at(0) == '_' && buffer.size() > 1){
            err("Identifiers cannot start with and underscore\n");
          }
          debug("Found an identifier '%s'\n", buffer.c_str());
          push(TokenType_Identifier, buffer);
        }
      }
      break;
          }
  }

  push(TokenType_EOF, "EOF");
  return std::pair<Token *, std::vector<std::string>>(head, directives);
};
