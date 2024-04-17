#include "parser.hpp"
#include "types.hpp"
#include <algorithm>
#include <assert.h>
#include <iostream>
#include <map>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct AST *ast;
struct Statement *parse_statement();

// Flags
int loop_counter;
bool in_global_scope =
    true; // At this time, we don't want nested function definitions, so we use
          // this to check whether or not we're nested.
bool in_defer = false;
std::vector<struct Statement *> defer_stack = std::vector<struct Statement *>();
std::vector<struct Block *> block_stack =
    std::vector<struct Block *>(); // Right now this doesn't make too much sense
                                   // since we don't have nested blocks, but
                                   // we're going to add it since we are going
                                   // to want nested scopes at some point.

std::map<enum TokenType, enum AstType> token_type_to_ast_type = {
    {TokenType_Add, AstType_Add},
    {TokenType_Sub, AstType_Sub},
    {TokenType_Star, AstType_Mul}, // TODO: This is a hack, we need to
                                   // differentiate between * and *deref
    {TokenType_Div, AstType_Div},
    {TokenType_Mod, AstType_Mod},

    {TokenType_Eq, AstType_Eq},
    {TokenType_Neq, AstType_Neq},
    {TokenType_Geq, AstType_Geq},
    {TokenType_Leq, AstType_Leq},
    {TokenType_Great, AstType_Great},
    {TokenType_Less, AstType_Less},
    {TokenType_And, AstType_And},
    {TokenType_Or, AstType_Or},
    {TokenType_Land, AstType_Land},
    {TokenType_Lor, AstType_Lor},

    {TokenType_Bang, AstType_Not},

    {TokenType_Assign, AstType_Assign},
    {TokenType_Return, AstType_Return},
    {TokenType_If, AstType_If},
    {TokenType_While, AstType_While},
    {TokenType_Defer, AstType_Defer},
};

std::map<enum AstType, std::string> ast_type_to_string = {

    {AstType_Int, "AstType_Int"},
    {AstType_Float, "AstType_Float"},
    {AstType_Char, "AstType_Char"},
    {AstType_Str, "AstType_Str"},
    {AstType_Void, "AstType_Void"},
    {AstType_Unknown, "AstType_Unknown"},
    {AstType_Integer, "AstType_Integer"},
    {AstType_Floating, "AstType_Floating"},
    {AstType_Character, "AstType_Character"},
    {AstType_String, "AstType_String"},
    {AstType_Identifier, "AstType_Identifier"},
    {AstType_Add, "AstType_Add"},
    {AstType_Sub, "AstType_Sub"},
    {AstType_Mul, "AstType_Mul"},
    {AstType_Div, "AstType_Div"},
    {AstType_Mod, "AstType_Mod"},
    {AstType_Eq, "AstType_Eq"},
    {AstType_Neq, "AstType_Neq"},
    {AstType_Geq, "AstType_Geq"},
    {AstType_Leq, "AstType_Leq"},
    {AstType_Great, "AstType_Great"},
    {AstType_Less, "AstType_Less"},
    {AstType_And, "AstType_And"},
    {AstType_Or, "AstType_Or"},
    {AstType_Land, "AstType_Land"},
    {AstType_Lor, "AstType_Lor"},
    {AstType_Not, "AstType_Not"},
    {AstType_Assign, "AstType_Assign"},
    {AstType_Return, "AstType_Return"},
    {AstType_If, "AstType_If"},
    {AstType_While, "AstType_While"},
    {AstType_FunctionDefinition, "AstType_FunctionDefinition"},
    {AstType_BinaryOp, "AstType_BinaryOp"},
    {AstType_UnaryOp, "AstType_UnaryOp"},
    {AstType_Literal, "AstType_Literal"},
    {AstType_FunctionCall, "AstType_FunctionCall"},
    {AstType_Interp, "AstType_Interp"},
    {AstType_Defer, "AstType_Defer"},
    {AstType_Switch, "AstType_Switch"},
    {AstType_Break, "AstType_Break"},
    {AstType_Continue, "AstType_Continue"},
    {AstType_Array, "AstType_Array"},
    {AstType_Index, "AstType_Index"},
    {AstType_Dot, "AstType_Dot"},
    {AstType_Expression, "AstType_Expression"},
    {AstType_Enum, "AstType_Enum"},
};

std::map<enum TokenType, std::string> token_type_to_string = {
    {TokenType_If, "TokenType_If"},
    {TokenType_Eq, "TokenType_Eq"},
    {TokenType_Or, "TokenType_Or"},
    {TokenType_For, "TokenType_For"},
    {TokenType_Str, "TokenType_Str"},
    {TokenType_Int, "TokenType_Int"},
    {TokenType_Geq, "TokenType_Geq"},
    {TokenType_Leq, "TokenType_Leq"},
    {TokenType_And, "TokenType_And"},
    {TokenType_Amp, "TokenType_Amp"},
    {TokenType_Mod, "TokenType_Mod"},
    {TokenType_Break, "TokenType_Break"},
    {TokenType_Continue, "TokenType_Continue"},
    {TokenType_Add, "TokenType_Add"},
    {TokenType_Sub, "TokenType_Sub"},
    {TokenType_Div, "TokenType_Div"},
    {TokenType_Lor, "TokenType_Lor"},
    {TokenType_Land, "TokenType_Land"},
    {TokenType_Dot, "TokenType_Dot"},
    {TokenType_Neq, "TokenType_Neq"},
    {TokenType_Less, "TokenType_Less"},
    {TokenType_Pipe, "TokenType_Pipe"},
    {TokenType_Bang, "TokenType_Bang"},
    {TokenType_Star, "TokenType_Star"},
    {TokenType_Else, "TokenType_Else"},
    {TokenType_Func, "TokenType_Func"},
    {TokenType_Char, "TokenType_Char"},
    {TokenType_Empty, "TokenType_Empty"},
    {TokenType_While, "TokenType_While"},
    {TokenType_Great, "TokenType_Great"},
    {TokenType_Comma, "TokenType_Comma"},
    {TokenType_Colon, "TokenType_Colon"},
    {TokenType_Input, "TokenType_Input"},
    {TokenType_Print, "TokenType_Print"},
    {TokenType_Assign, "TokenType_Assign"},
    {TokenType_Addressof, "TokenType_Addressof"},
    {TokenType_Deref, "TokenType_Deref"},
    {TokenType_Rbrace, "TokenType_Rbrace"},
    {TokenType_Lbrace, "TokenType_Lbrace"},
    {TokenType_String, "TokenType_String"},
    {TokenType_Return, "TokenType_Return"},
    {TokenType_Integer, "TokenType_Integer"},
    {TokenType_Lparens, "TokenType_Lparens"},
    {TokenType_Rparens, "TokenType_Rparens"},
    {TokenType_Lbracket, "TokenType_Lbracket"},
    {TokenType_Rbracket, "TokenType_Rbracket"},
    {TokenType_Semicolon, "TokenType_Semicolon"},
    {TokenType_Character, "TokenType_Character"},
    {TokenType_Identifier, "TokenType_Identifier"},
    {TokenType_Void, "TokenType_Void"},
    {TokenType_Floating, "TokenType_Floating"},
    {TokenType_Float, "TokenType_Float"},
    {TokenType_EOF, "TokenType_EOF"},
    {TokenType_Defer, "TokenType_Defer"},
};

// This generally shouldn't be used but it's useful sometimes
void backtrack(int n) {
  for (int i = 0; i < n; i++) {
    if (ast->current->prev != nullptr) {

      ast->current = ast->current->prev;
    } else {
      err("Backtrack failed, tried to backtrack %d tokens but only %d tokens "
          "were available\n",
          n, i);
    }
  }
}

void consume() {
  if (ast->current->type == TokenType_EOF) {
    return;
  }
  ast->current = ast->current->next;
}

bool consume_if(enum TokenType type) {
  if (ast->current->type == type) {
    consume();
    return true;
  }
  return false;
}

void consume_assert(enum TokenType type) {
  if (ast->current->type != type) {
    err("Expected token of type %s but got %s instead\n",
        token_type_to_string[type].c_str(),
        token_type_to_string[ast->current->type].c_str());
  }
  consume();
}

struct TypeWrapper *consume_array_type() {
  logcall();

  struct TypeWrapper *type_struct = nullptr;
  while (consume_if(TokenType_Lbracket)) {
    debug("Found the start of an array, consuming...\n");

    struct TypeWrapper *array_type = new TypeWrapper(VarType_Array);

    struct Array *arr = boost::get<struct Array>(&array_type->content);
    arr->content_type = type_struct;

    // Check for expression which would be the size of the array
    arr->capacity = parse_expression();
    if (arr->capacity != nullptr) {
      debug("Found expression which represents the static array allocation "
            "quantity\n");
      arr->is_dynamic_array = false;
    } else {
      debug("Consuming array without explicit allocation size (might still be "
            "allocated via. assignment with literal-array)\n");
      arr->is_dynamic_array = true;
    }

    consume_assert(TokenType_Rbracket);
    arr->content_type = type_struct;
    type_struct = array_type;
  }

  return type_struct;
}

std::map<TokenType, VarType> token_type_to_var_type = {
    {TokenType_Int, VarType_Int},    {TokenType_Float, VarType_Float},
    {TokenType_Char, VarType_Char},  {TokenType_Void, VarType_Void},
    {TokenType_Str, VarType_String},
};

struct TypeWrapper *consume_type() {
  logcall();

  struct TypeWrapper *type_struct = new TypeWrapper(VarType_Unknown);

  switch (ast->current->type) {
  case TokenType_Enum: {
    debug("Found enum type\n");
    type_struct->var_type = VarType_Enum;

    type_struct->content = new struct EnumContainer();
    consume();
    type_struct->type_name = parse_identifier();
    if (type_struct->type_name == nullptr) {
      err("Expected identifier after enum keyword");
    }
    backtrack(1);
    break;
  }
  case TokenType_Pointer: {
    debug("Found pointer type\n");
    type_struct->var_type = VarType_Pointer;
    consume();
    type_struct->content = Pointer{
        .content_type = nullptr,
    };
    struct Pointer *ptr = boost::get<struct Pointer>(&type_struct->content);
    ptr->content_type = consume_type();
    if (ptr->content_type == nullptr ||
        ptr->content_type->var_type == VarType_Unknown) {
      err("Expected type after pointer\n");
    }
    return type_struct;
  }
  default:
    if (token_type_to_var_type.count(ast->current->type) > 0) {
      debug("Found primitive type '%s'\n",
            token_type_to_string[ast->current->type].c_str());
      type_struct->var_type = token_type_to_var_type[ast->current->type];
      consume();
      break; // Continue down the function as we need to check for array types
    }
  }

  if (ast->current->type == TokenType_Lbracket) {
    debug("Found Array Type\n");
    struct TypeWrapper *arr = consume_array_type();
    if (arr != nullptr) {
      arr->content = Array{
          .is_dynamic_array = false,
          .content_type = type_struct,
          .capacity = nullptr,
      };
      return arr;
    }
  }
  debug("Returning type: %s\n", type_to_string[type_struct->var_type].c_str());
  return type_struct;
}

std::map<enum VarType, std::string> type_to_string = {
    {VarType_Int, "int"},         {VarType_Float, "float"},
    {VarType_Char, "char"},       {VarType_Void, "void"},
    {VarType_String, "char *"},   {VarType_Unknown, "unknown"},
    {VarType_Enum, "enum"},       {VarType_Null, "nullptr"},
    {VarType_Pointer, "pointer"}, {VarType_Array, "array"},
};

struct Identifier *parse_identifier() {
  logcall();
  struct Identifier *ident = new Identifier();
  if (ast->current->type != TokenType_Identifier) {
    return nullptr;
  }
  ident->name = ast->current->literal_value;
  consume();
  return ident;
}

struct Function *parse_function() {
  logcall();

  if (!in_global_scope) {
    err("Cannot declare nested functions\n");
  }

  in_global_scope = false;

  struct Function *func = new Function();
  consume();
  func->return_type = consume_type();
  if (func->return_type == nullptr) {
    err("Expected return type in function definition\n");
  }

  func->name = parse_identifier();
  if (func->name == nullptr) {
    err("Expected identifier in function definition\n");
  }
  consume_assert(TokenType_Lparens);

  parse_parameters(func);

  consume_assert(TokenType_Rparens);
  consume_assert(TokenType_Lbrace);

  func->body = parse_block();
  defer_stack.clear();

  consume_assert(TokenType_Rbrace);
  in_global_scope = true;
  return func;
}

void parse_parameters(struct Function *function) {
  logcall();

  while (ast->current->type != TokenType_Rparens) {
    struct Variable *arg = new Variable();
    arg->type_wrapper = new TypeWrapper();
    arg->name = parse_identifier();
    if (arg->name == nullptr) {
      err("Expected identifier in function argument\n");
    }

    consume_assert(TokenType_Colon);

    arg->type_wrapper = consume_type();
    if (arg->type_wrapper == nullptr) {
      err("Expected type in function argument\n");
    } else if (arg->type_wrapper->var_type == VarType_Void) {
      err("Cannot have void type in function argument\n");
    }

    function->params.push_back(arg);
    if (ast->current->type == TokenType_Comma) {
      consume();
    }
  }
}

void parse_arguments(struct FunctionCall *call) {
  logcall();

  while (ast->current->type != TokenType_Rparens) {
    struct Expression *arg = parse_expression();

    if (arg == nullptr) {
      err("Faulty argument in function call\n");
    }

    call->args.push_back(arg);
    if (ast->current->type == TokenType_Comma) {
      consume();
    }
  }

  consume_if(TokenType_Comma);
}

struct FunctionCall *parse_function_call() {
  logcall();

  struct FunctionCall *call = new FunctionCall();

  call->name = parse_identifier();
  if (call->name == nullptr) {
    err("Expected identifier in function call\n");
  }

  consume_assert(TokenType_Lparens);
  parse_arguments(call);
  consume_assert(TokenType_Rparens);

  return call;
}

struct Expression *parse_factor() {
  logcall();

  struct Expression *expr = new Expression();

  // Prioritized expressions
  if (ast->current->type == TokenType_Lparens) {
    expr = parse_expression();
    consume_assert(TokenType_Rparens);
    return expr;
  }

  // All expressions containing identifiers
  struct Identifier *ident = parse_identifier();
  if (ident != nullptr) {

    if (ast->current->type == TokenType_Lparens) {
      debug("-> FunctionCall\n");
      expr->ast_type = AstType_FunctionCall;
      backtrack(1); // backtrack to the identifier
      free(ident);
      expr->function_call = parse_function_call();
      return expr;

    } else if (ast->current->type == TokenType_Lbracket) {
      debug("-> Index\n");

      consume();
      expr->ast_type = AstType_Index;
      expr->index = new Index();
      expr->index->name = ident;
      expr->index->idx = parse_expression();
      consume_assert(TokenType_Rbracket);

      return expr;

    } else if (ast->current->type == TokenType_Dot) {
      // Member access or method call. We're going to start out by being naive,
      // and not letting users do struct.member.member.func() and thus not
      // allowing this nested stuff.
      consume();
      struct Dot *dot = new Dot();
      expr->ast_type = AstType_Dot;
      expr->dot = dot;
      dot->left = new Expression(AstType_Identifier);
      dot->left->identifier = ident;

      debug("Identifier on the left: %s\n",
            dot->left->identifier->name.c_str());
      dot->right = parse_expression();

      if (dot->right == nullptr) {
        err("Expected expression after dot\n");
      }
      debug("Identifier on the right: %s\n",
            dot->right->identifier->name.c_str());

      if (dot->right->ast_type == AstType_FunctionCall) {
        // We call this 'MethodCall' and not 'FunctionCall'
        // as it's called via. a member, struct, or in the future, an array
        // access (we gotta get functions as first-class citizens), but
        // internally they're handled the same except for some specific
        // type-checking.
        debug("-> MethodCall\n"); //@TODO Not implemented yet really
        return expr;
      } else {
        debug("-> MemberAccess\n");
        // This could be a structure member access or an enum member
        dot->right->ast_type = AstType_Identifier;
        return expr;
      }
    }
    debug("-> Identifier\n");
    expr->ast_type = AstType_Identifier;
    expr->identifier = ident;
    return expr;
  }

  // Others
  debug("Literal '%s'\n", token_type_to_string[ast->current->type].c_str());
  struct Literal *lit = new Literal(new TypeWrapper());
  expr->ast_type = AstType_Literal;

  switch (ast->current->type) {
  case TokenType_Integer:
    lit->type_wrapper->var_type = VarType_Int;
    lit->value = ast->current->value;
    break;
  case TokenType_Floating:
    lit->type_wrapper->var_type = VarType_Float;
    lit->value = ast->current->value;
    break;
  case TokenType_Character:
    lit->type_wrapper->var_type = VarType_Char;
    lit->value = ast->current->value;
    break;
  case TokenType_String:
    lit->type_wrapper->var_type = VarType_String;
    lit->value = ast->current->value;
    break;
  case TokenType_Lbracket: {

    consume();
    expr->ast_type = AstType_Array;
    lit->type_wrapper = new TypeWrapper();
    lit->type_wrapper->var_type = VarType_Array;

    // Enforce that the vector of expressions is initalized
    if (lit->value.which() !=
        3) { // 3 refers to the type "std::vector<struct Expression *>"

      lit->value = std::vector<struct Expression *>();
      debug("Setting the literals value to a vector of expressions as it's "
            "not already initialized\n");
    }

    // Initialize lit->value
    std::vector<struct Expression *> *vec =
        boost::get<std::vector<struct Expression *>>(&lit->value);

    // Gather all the expressions in the array literal
    while (ast->current->type != TokenType_Rbracket) {
      struct Expression *value = parse_expression();
      if (value == nullptr) {
        err("Expected expression in array literal\n");
      }

      debug("Found expression in array literal: %s\n",
            ast_type_to_string[value->ast_type].c_str());
      vec->push_back(value);
      consume_if(TokenType_Comma);
    }

    // The array we created has an implicit size based on the number of
    // elements. So here we set the size of the array.
    struct Array *arr = boost::get<struct Array>(&lit->type_wrapper->content);
    arr->capacity = make_integer_literal_expression();
    arr->capacity->literal->value = static_cast<int>(vec->size());
    debug("Setting the size of the array to %d\n",
          boost::get<int>(arr->capacity->literal->value));
  } break;
  default:
    return nullptr;
  }
  expr->literal = lit;
  consume();
  return expr;
}
struct Expression *parse_term() {
  logcall();

  struct Expression *left = parse_factor();
  if (left == nullptr) {
    return nullptr;
  }

  while (ast->current->type == TokenType_Star ||
         ast->current->type == TokenType_Div) {
    debug("-> Binary %s\n",
          ast->current->type == TokenType_Star ? "Multiplication" : "Divison");
    struct BinaryOp *bop = new BinaryOp();
    struct Expression *expr = new Expression();

    expr->ast_type = AstType_BinaryOp;
    bop->ast_type = token_type_to_ast_type[ast->current->type];
    consume();

    expr->binary_op = bop;
    bop->left = left;
    bop->right = parse_factor();
    left = expr;
  }

  return left;
}

struct Expression *parse_additive() {
  logcall();

  struct Expression *left = parse_term();
  if (left == nullptr) {
    return nullptr;
  }

  while (ast->current->type == TokenType_Add ||
         ast->current->type == TokenType_Sub) {
    debug("-> Binary %s\n",
          ast->current->type == TokenType_Add ? "Addition" : "Subtraction");

    struct BinaryOp *bop = new BinaryOp();
    struct Expression *expr = new Expression();

    expr->ast_type = AstType_BinaryOp;
    bop->ast_type = token_type_to_ast_type[ast->current->type];
    consume();

    expr->binary_op = bop;
    bop->left = left;
    bop->right = parse_term();
    left = expr;
  }

  return left;
}
struct Expression *parse_relational() {
  struct Expression *left = parse_additive();
  if (left == nullptr) {
    return nullptr;
  }

  while (ast->current->type == TokenType_Less ||
         ast->current->type == TokenType_Great ||
         ast->current->type == TokenType_Leq ||
         ast->current->type == TokenType_Geq ||
         ast->current->type == TokenType_Eq ||
         ast->current->type == TokenType_Neq) {
    debug("-> Binary Comparison\n");

    struct BinaryOp *bop = new BinaryOp();
    struct Expression *expr = new Expression();

    expr->ast_type = AstType_BinaryOp;
    bop->ast_type = token_type_to_ast_type[ast->current->type];
    consume();

    expr->binary_op = bop;
    bop->left = left;
    bop->right = parse_additive();
    left = expr;
  }

  return left;
}
struct Expression *parse_logical_and() {
  struct Expression *left = parse_relational();
  if (left == nullptr) {
    return nullptr;
  }

  while (ast->current->type == TokenType_Land) {
    debug("-> Logical And\n");
    struct BinaryOp *bop = new BinaryOp();
    struct Expression *expr = new Expression();

    expr->ast_type = AstType_BinaryOp;
    bop->ast_type = token_type_to_ast_type[ast->current->type];
    consume();

    expr->binary_op = bop;
    bop->left = left;
    bop->right = parse_relational();
    left = expr;
  }

  return left;
}
struct Expression *parse_logical_or() {
  logcall();

  struct Expression *left = parse_logical_and();
  if (left == nullptr) {
    return nullptr;
  }

  while (ast->current->type == TokenType_Lor) {
    debug("-> Logical Or\n");

    struct BinaryOp *bop = new BinaryOp();
    struct Expression *expr = new Expression();

    expr->ast_type = AstType_BinaryOp;
    bop->ast_type = token_type_to_ast_type[ast->current->type];
    consume();

    expr->binary_op = bop;
    bop->left = left;
    bop->right = parse_logical_and();
    left = expr;
  }

  return left;
}
struct Expression *parse_expression() { return parse_logical_or(); }
struct Assign *parse_assignment() {
  /*
      Assignments can take four forms:

          identifier: type;
          identifier = value;
          identifier: type = value;
          identifier[idx] = value;

      ': type' infers that the variable does
      not yet exist and should be declared.

      '= value' means what you'd expect
  */

  logcall();

  struct Assign *assignment = new struct Assign();
  assignment->type_wrapper = new struct TypeWrapper();
  assignment->type_wrapper->var_type =
      VarType_Unknown; // Used to check for no declaration
  assignment->right = nullptr;

  // the left-hand side can be either an identifier, or an index into an array.
  assignment->index = parse_expression();
  debug("Assignment left type: %s\n",
        ast_type_to_string[assignment->index->ast_type].c_str());
  if (assignment->index->ast_type == AstType_Identifier) {

    // Whups, there was no index, just an identifier! This is a little hacky,
    // but since index expressions also start with an identifier, it's hard to
    // get around tbh. Unless we want to parse the identifier, and then check
    // for brackets. But if we did this, why even have the Index in
    // parse_expression.
    assignment->identifier = assignment->index->identifier;
    debug("Assigning to identifier '%s'\n",
          assignment->identifier->name.c_str());
    assignment->index = nullptr;
  } else {
    debug("Assigning to index of array '%s'\n",
          assignment->index->index->name->name.c_str());
  }

  // Ensure that there's a left-hand side
  if (assignment->index == nullptr && assignment->identifier == nullptr) {
    err("Invalid left-hand side in assignment\n");
  }

  // There is a declaration
  if (ast->current->type == TokenType_Colon) {
    debug("^^ it's also a declaration\n");
    if (assignment->index != nullptr) {
      err("Cannot declare an array index\n");
    }

    consume();
    assignment->type_wrapper = consume_type();

    if (assignment->type_wrapper->var_type == VarType_Unknown) {
      err("Expected type after colon in declaration\n");
    }
  }

  // There is a value assignment
  if (ast->current->type == TokenType_Assign) {
    consume();
    assignment->right = parse_expression();

    if (assignment->right == nullptr) {
      err("Expected expression on the right-hand side of assignment\n");
    }
    debug("Assignment right type: %s\n",
          ast_type_to_string[assignment->right->ast_type].c_str());
  }

  // If we've found an array declaration (in consume_type) of a dynamic array,
  // we have to insert a default allocation value, which is 1. We have to do
  // this since in e.g. C it's required to initialize an array with a size.
  // print the string of the type
  if (assignment->type_wrapper->var_type == VarType_Array) {
    struct Array *arr =
        boost::get<struct Array>(&assignment->type_wrapper->content);
    if (arr->is_dynamic_array) {
      debug("Found dynamic array declaration\n");
      // If there is an array literal on the right hand side, we can just use
      // the count of that array.
      if (assignment->right != nullptr &&
          assignment->right->ast_type == AstType_Array) {
        arr->capacity = make_integer_literal_expression();
        arr->capacity->literal->value =
            boost::get<int>(assignment->right->literal->value);
        debug(
            "Using the array capacity of the right-hand side array literal for "
            "default allocation of dynamic array declaration\n");
      } else {
        // Otherwise we just create a dummy one with a value of 1
        debug("Inserted dummy default allocation into dynamic array "
              "declaration (size = 1)\n");
        arr->capacity = make_integer_literal_expression();
        arr->capacity->literal->value = 1;
      }
    }
  }

  return assignment;
}

struct If *parse_if() {
  logcall();

  struct If *if_ = new struct If();
  if_->body = nullptr;
  if_->else_ = nullptr;

  consume();
  consume_if(TokenType_Lparens); // consume_if() since we won't enforce parens

  if_->condition = parse_expression();

  if (if_->condition == nullptr) {
    err("Expected expression in if statement conditional\n");
  }

  consume_if(TokenType_Rparens); // consume_if() since we won't enforce parens
  consume_assert(TokenType_Lbrace);
  if_->body = parse_block();

  consume_assert(TokenType_Rbrace);
  if (ast->current->type == TokenType_Else) {
    consume();
    consume_assert(TokenType_Lbrace);
    if_->else_ = parse_block();
    consume_assert(TokenType_Rbrace);
  }

  return if_;
}

struct For *parse_for() {
  logcall();

  loop_counter++;

  struct For *for_ = new struct For();
  consume();
  consume_assert(TokenType_Lparens);

  for_->init = parse_statement();
  if (for_->init == nullptr) {
    err("Expected statement in for loop init\n");
  }

  for_->condition = parse_expression();
  if (for_->condition == nullptr) {
    err("Expected expression in for loop conditional\n");
  }
  consume_assert(TokenType_Semicolon);

  for_->post = parse_statement();
  if (for_->post == nullptr) {
    err("Expected statement in for loop post\n");
  }

  consume_assert(TokenType_Rparens);
  consume_assert(TokenType_Lbrace);

  for_->body = parse_block();

  consume_assert(TokenType_Rbrace);
  loop_counter--;
  return for_;
}

struct While *parse_while() {
  logcall();

  loop_counter++;

  struct While *while_ = new struct While();

  consume();
  consume_if(TokenType_Lparens); // consume_if() since we won't enforce parens
  while_->condition = parse_expression();

  if (while_->condition == nullptr) {
    err("Expected expression in while statement conditional\n");
  }
  consume_if(TokenType_Rparens); // consume_if() since we won't enforce parens
  consume_assert(TokenType_Lbrace);
  while_->body = parse_block();
  consume_assert(TokenType_Rbrace);
  loop_counter--;

  return while_;
}

struct EnumContainer *parse_enum() {
  /*
  enum MyEnum {
    A,
    B,
    C
  }
  */
  debug("-> Parse Enum\n");

  if (!in_global_scope) {
    err("Cannot make nested enum definitions\n");
  }

  consume();

  struct EnumContainer *enum_ = new struct EnumContainer();
  enum_->name = parse_identifier();
  if (enum_->name == nullptr) {
    err("Expected identifier in enum declaration, got '%s' instead\n",
        token_type_to_string[ast->current->type].c_str());
  }

  debug("Enum name: %s\n", enum_->name->name.c_str());
  consume_assert(TokenType_Lbrace);

  while (ast->current->type != TokenType_Rbrace) {

    struct Identifier *name = nullptr;
    struct Expression *value = nullptr;

    name = parse_identifier();
    if (name == nullptr) {
      err("Expected identifier in enum declaration, got '%s' instead\n",
          token_type_to_string[ast->current->type].c_str());
    }

    debug("Enum Member: %s\n", name->name.c_str());

    // Here we also have to assert that the identifier doesn't already exist in
    // the map, this could be done in typechecking but since we're using a map
    // and not a vector, we cannot check for uniqueness later, as we could
    // overwrite a map value here.

    if (enum_->mapping.count(name->name) > 0) {
      err("Duplicate identifier '%s' in enum declaration\n",
          name->name.c_str());
    }

    if (ast->current->type == TokenType_Assign) {
      consume();

      value = parse_expression();
      if (value == nullptr) {
        err("Expected expression in enum declaration when using '='\n");
      }
    } else {
      // We handle default-values for enum members in code-generation, we can
      // just send them without values directly to C, which then handles it
      debug("No value for enum member '%s' using default\n",
            name->name.c_str());
    }

    enum_->mapping[name->name] = value;

    if (ast->current->type == TokenType_Comma) {
      consume();
    }
  }
  consume_assert(TokenType_Rbrace);

  // print the enum
  for (auto const &x : enum_->mapping) {
    debug("Enum Member: %s\n", x.first.c_str());
  }

  return enum_;
}

struct Print *parse_print() {
  /*
    print("Hello World");
    print("Hello World %d, %d, %d", 1, 2, 3);
  */

  // We basically just forward the arguments to printf

  consume_assert(TokenType_Print);
  consume_assert(TokenType_Lparens);

  struct Print *print = new Print();

  print->format = parse_expression();

  if (consume_if(TokenType_Comma)) {
    while (ast->current->type != TokenType_Rparens) {
      print->args.push_back(parse_expression());
      consume_if(TokenType_Comma);
    }
  }
  consume_assert(TokenType_Rparens);

  if (print->format == nullptr) {
    err("Expected expression in print statement\n");
  }

  return print;
}

// These are small enough that they could be done in-line in parser_statement
// but I like the decoupling, while these funtion calls are relatively
// inefficient from a compiler run-time perspective, that's an okay tradeoff
// imho
struct Return *parse_return() {
  consume();

  for (int i = defer_stack.size() - 1; i >= 0; i--) {
    debug("Adding Defer stack\n");
    block_stack.back()->statements.push_back(defer_stack[i]);
  }

  struct Return *ret = new Return();
  ret->expression = parse_expression();
  return ret;
}
struct Break *parse_break() {
  if (loop_counter == 0) {
    err("Cannot break outside of a loop\n");
  }
  consume();

  return new Break();
}
struct Continue *parse_continue() {
  if (loop_counter == 0) {
    err("Cannot continue outside of a loop\n");
  }
  consume();
  return new Continue();
}

struct Input *parse_input() {
  consume_assert(TokenType_Input);
  consume_assert(TokenType_Lparens);
  struct Input *input = new Input();

  input->destination = parse_expression();
  if (input->destination == nullptr) {
    err("Expected destination expression as first argument in function "
        "input()\n");
  }

  consume_assert(TokenType_Comma);

  input->path = parse_expression();

  if (input->path == nullptr) {
    err("Expected file-path expression as second argument in function "
        "input()\n");
  }

  consume_assert(TokenType_Comma);

  input->format = parse_expression();
  if (input->format == nullptr) {
    err("Expected format expression as third argument in function input()\n");
  }

  consume_assert(TokenType_Rparens);
  return input;
}

struct Statement *parse_statement();
struct Defer *parse_defer() {

  /**
   * Defer is a statement that defers the execution of a statement until the end
   * of the scope. > defer(print("Hello World")); It might be a good idea to
   * allow for blocks or at least the {} syntax instead of a 'floating'
   * statement in parens.
   */

  if (in_defer) {
    err("Cannot defer a defer\n");
  }
  consume();

  consume_assert(TokenType_Lparens);
  struct Defer *defer = new Defer();
  in_defer = true;
  defer->statement = parse_statement();
  consume_assert(TokenType_Rparens);

  if (defer->statement == nullptr) {
    err("Expected statement after defer\n");
  }
  in_defer = false;
  debug("Adding statement to defer stack\n");

  defer_stack.push_back(defer->statement);
  return defer;
}

struct Statement *parse_statement() {
  struct Statement *stmt = new Statement();

  debug("Statement type '%s'\n",
        token_type_to_string[ast->current->type].c_str());

  switch (ast->current->type) {

  case TokenType_Func: // Function Definitions
    stmt->ast_type = AstType_FunctionDefinition;
    stmt->function = parse_function();
    break;
  case TokenType_If:
    stmt->ast_type = AstType_If;
    stmt->if_ = parse_if();
    break;
  case TokenType_While:
    stmt->ast_type = AstType_While;
    stmt->while_ = parse_while();
    break;
  case TokenType_For:
    stmt->ast_type = AstType_For;
    stmt->for_ = parse_for();
    break;
  case TokenType_Return:
    stmt->ast_type = AstType_Return;
    stmt->return_ = parse_return();
    break;
  case TokenType_Break:
    stmt->ast_type = AstType_Break;
    stmt->break_ = parse_break();
    break;
  case TokenType_Continue:
    stmt->ast_type = AstType_Continue;
    stmt->continue_ = parse_continue();
    break;
  case TokenType_Print:
    stmt->ast_type = AstType_Print;
    stmt->print = parse_print();
    break;
  case TokenType_Input:
    stmt->ast_type = AstType_Input;
    stmt->input = parse_input();
    break;
  case TokenType_Defer:
    stmt->ast_type = AstType_Defer;
    stmt->defer = parse_defer();
    break;
  case TokenType_Enum:
    stmt->ast_type = AstType_Enum;
    stmt->enum_ = parse_enum();
    break;
  default:
    // Check for assignment
    if (ast->current->type == TokenType_Identifier &&
        ((ast->current->next->type == TokenType_Colon ||
          ast->current->next->type == TokenType_Assign) ||
         ast->current->next->type == TokenType_Lbracket)) {
      stmt->ast_type = AstType_Assign;
      stmt->assign = parse_assignment();
      break;
    }

    // Check for expression
    stmt->ast_type = AstType_Expression;
    stmt->expression = parse_expression();

    // Maybe do some error handling for wrong variable declaration syntax
    if (stmt->expression == nullptr) {
      return nullptr;
    }
  }

  consume_if(TokenType_Semicolon);
  return stmt;
}

struct Block *parse_block() {
  logcall();
  struct Block *block = new struct Block();
  block_stack.push_back(block);
  struct Statement *stmt;

  while ((stmt = parse_statement()) != nullptr) {

    if (stmt->ast_type == AstType_FunctionDefinition) {
      ast->functions.push_back(stmt->function);
      continue;
    }
    block->statements.push_back(stmt);
  }

  block_stack.pop_back();

  return block;
}

struct Block *parse_global() {
  logcall();

  struct Block *block = new struct Block();
  block_stack.push_back(block);
  struct Statement *stmt;

  while ((stmt = parse_statement()) != nullptr) {

    switch (stmt->ast_type) {
    case AstType_FunctionDefinition:
      ast->functions.push_back(stmt->function);
      break;
    case AstType_Assign:
      ast->variables.push_back(stmt);
      break;
    case AstType_Enum: {
      debug("Adding enum to global scope\n");
      struct Structure *structure = new Structure();
      structure->ast_type = AstType_Enum;
      structure->enum_ = stmt->enum_;
      ast->structures.push_back(structure);
      free(stmt);
    } break;
    default:
      block->statements.push_back(stmt);
      break;
    }
  }
  return block;
}

struct AST *parse(struct Token *head) {
  logcall();
  ast = new AST();
  ast->head = head;
  ast->current = head;
  ast->global = parse_global();
  debug("Finished Parsing\n");
  return ast;
}
