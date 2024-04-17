
#pragma once
#include "parser.hpp"
#include "types.hpp"
#include <string>
char *format_string(const char *format, ...);
std::string get_string_representation_of_literal(struct Literal *literal);
struct Expression *make_integer_literal_expression();

#ifdef DEBUG
#define debug(str, args...)                                                    \
  printf("%s:%d:\t%s():\t" str, __FILE__, __LINE__, __func__, ##args)
#else
#define debug(str, args...) ;
#endif

#define err(str, args...)                                                      \
  fprintf(stderr, "[ERROR]:%s:%d:\t%s():\t" str, __FILE__, __LINE__, __func__, \
          ##args);                                                             \
  exit(-1)

#ifdef DEBUG
#define logcall() debug("\n")
#else
#define logcall() ;
#endif