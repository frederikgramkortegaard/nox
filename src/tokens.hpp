#pragma once
#include <map>
#include <string>
#include <boost/variant.hpp>

enum TokenType {
  TokenType_If,
  TokenType_Eq,
  TokenType_Or,
  TokenType_For,
  TokenType_Str,
  TokenType_Int,
  TokenType_Geq,
  TokenType_Leq,
  TokenType_And,
  TokenType_Amp,
  TokenType_Mod,
  TokenType_Break,
  TokenType_Continue,
  TokenType_Add,
  TokenType_Sub,
  TokenType_Div,
  TokenType_Lor,
  TokenType_Land,
  TokenType_Dot,
  TokenType_Neq,
  TokenType_Less,
  TokenType_Pipe,
  TokenType_Bang,
  TokenType_Star,
  TokenType_Else,
  TokenType_Func,
  TokenType_Char,
  TokenType_Defer,
  TokenType_Empty,
  TokenType_While,
  TokenType_Great,
  TokenType_Comma,
  TokenType_Colon,
  TokenType_Input,
  TokenType_Print,
  TokenType_Assign,
  TokenType_Addressof,
  TokenType_Deref,
  TokenType_Rbrace,
  TokenType_Lbrace,
  TokenType_String,
  TokenType_Return,
  TokenType_Integer,
  TokenType_Lparens,
  TokenType_Rparens,
  TokenType_Lbracket,
  TokenType_Rbracket,
  TokenType_Semicolon,
  TokenType_Character,
  TokenType_Identifier,
  TokenType_Void,
  TokenType_Floating,
  TokenType_Float,
  TokenType_EOF,
  TokenType_Enum,
  TokenType_Bool,
  TokenType_Pointer,
};

struct Token {
  int row, col;
  enum TokenType type;
  boost::variant<int, double, std::string> value;
  struct Token *next;
  struct Token *prev;
  std::string literal_value;

};