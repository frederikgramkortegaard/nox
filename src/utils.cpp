#include "utils.hpp"
#include "types.hpp"
#include <iostream>
#include <stdarg.h>

char *format_string(const char *format, ...) {
  va_list args;
  va_start(args, format);
  char *ret;
  vasprintf(&ret, format, args);
  va_end(args);
  return ret;
}

// We do this a lot, so we have a function for it
struct Expression *make_integer_literal_expression() {
  struct Expression *expr = new Expression();
  expr->ast_type = AstType_Literal;
  expr->literal = new Literal();
  expr->literal->type_wrapper = new TypeWrapper();
  expr->literal->type_wrapper->var_type = VarType_Int;
  return expr;
}

std::string get_string_representation_of_literal(struct Literal *literal) {

  switch (literal->type_wrapper->var_type) {
  case VarType_Int:
    debug("printing int\n");
    return std::to_string(boost::get<int>(literal->value));
  case VarType_Float:
    debug("printing float\n");

    return std::to_string(boost::get<double>(literal->value));
  case VarType_Char:
    debug("printing char\n");

    return boost::get<std::string>(literal->value);
  case VarType_String:
    debug("printing string\n");

    return boost::get<std::string>(literal->value);
  case VarType_Null:
    debug("printing null\n");
    return "null";
  case VarType_Enum:
    debug("printing enum\n");
    return std::to_string(boost::get<int>(literal->value));
  default:
    err("Unsupported type for literal\n");
  }
}