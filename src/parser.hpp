#pragma once
#include "tokens.hpp"
#include "types.hpp"
#include "utils.hpp"

#include <map>
#include <string>
#include <vector>
#include <boost/variant.hpp>

void parse_parameters(struct Function *);
struct Expression *parse_expression();
struct AST *parse(struct Token *);
struct Block *parse_block();
struct Identifier *parse_identifier();


struct AST {
  std::vector<struct Function *>  functions; // Global functions
  std::vector<struct Statement *> variables; // Global variables
  std::vector<struct Structure *> structures; // Enums & Structs
  struct Block *global;
  struct Token *head;
  struct Token *current;
};
