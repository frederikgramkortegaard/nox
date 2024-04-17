#pragma once
#include "parser.hpp"
#include "tokens.hpp"
#include <vector>

enum VarType {
  VarType_Int,
  VarType_Float,
  VarType_Char,
  VarType_String,
  VarType_Void,
  VarType_Unknown,
  VarType_Enum,
  VarType_Null,
  VarType_Identifier,
  VarType_Function,
  VarType_Array,
  VarType_Any, // Used internally for e.g. create_dynamic_array_add()
  VarType_Pointer,
};

extern std::map<enum VarType, std::string> type_to_string;
struct Statement;

enum AstType {
  // Types
  AstType_Int,
  AstType_Float,
  AstType_Char,
  AstType_Str,
  AstType_Void,
  AstType_Unknown,
  AstType_Enum,
  AstType_Array,

  // Literals
  AstType_Integer,
  AstType_Floating,
  AstType_Character,
  AstType_String,
  AstType_Identifier,
  AstType_Pointer,

  // Binary Operators
  AstType_Add,
  AstType_Sub,
  AstType_Mul,
  AstType_Div,
  AstType_Mod,

  AstType_Eq,
  AstType_Neq,
  AstType_Geq,
  AstType_Leq,
  AstType_Great,
  AstType_Less,
  AstType_And,
  AstType_Or,
  AstType_Land,
  AstType_Lor,

  // Unary Operators
  AstType_Not,

  // Statements
  AstType_Assign,
  AstType_Return,
  AstType_If,
  AstType_While,
  AstType_For,
  AstType_FunctionDefinition,

  // Intrinsics
  AstType_Print,
  AstType_Defer,
  AstType_Switch,
  AstType_Input,

  // Control Flow
  AstType_Break,
  AstType_Continue,

  // Meta
  AstType_BinaryOp,
  AstType_UnaryOp,
  AstType_Literal,
  AstType_FunctionCall,
  AstType_Interp,
  AstType_Expression,
  AstType_Index,
  AstType_Dot,

};
extern std::map<enum AstType, std::string> ast_type_to_string;

struct Variable {
  struct Identifier *name;
  struct TypeWrapper *type_wrapper;
  int size;
};

struct Block {
  std::vector<struct Statement *> statements;
  struct Scope *scope;
};

struct Literal {
  struct TypeWrapper *type_wrapper;
  boost::variant<int, double, std::string, std::vector<struct Expression *>>
      value;

  Literal() {}
  Literal(struct TypeWrapper *type_wrapper) : type_wrapper(type_wrapper) {}
};

struct Identifier {
  std::string name;
};

struct Function {
  struct Identifier *name;
  std::vector<struct Variable *> params;
  struct TypeWrapper *return_type;
  struct Block *body;
  std::string raw_c;
};

struct BinaryOp {
  enum AstType ast_type;
  struct Expression *left;
  struct Expression *right;
};

struct UnaryOp {
  enum AstType ast_type;
  struct Expression *child;
};

struct FunctionCall {
  struct Identifier *name;
  std::vector<struct Expression *> args;
};

struct Dot {
  struct Expression *left;
  struct Expression *right;
};

struct Index {
  struct Identifier *name;
  struct Expression *idx;
};

struct Expression {
  enum AstType ast_type;
  union {
    struct BinaryOp *binary_op;
    struct UnaryOp *unary_op;
    struct Identifier *identifier;
    struct Literal *literal;
    struct FunctionCall *function_call;
    struct Index *index;
    struct Dot *dot;
  };

  Expression() {}
  Expression(enum AstType ast_type) : ast_type(ast_type) {}
};

struct If {
  struct Expression *condition;
  struct Block *body;
  struct Block *else_;
};

struct Assign {
  struct Identifier *identifier;
  struct TypeWrapper
      *type_wrapper; // Used when the assignment is also a declaration

  struct Expression *index; // If we want to assign into an f
  struct Expression *right;
};

struct While {
  struct Expression *condition;
  struct Block *body;
};

struct For {
  struct Statement *init;
  struct Expression *condition;
  struct Statement *post;
  struct Block *body;

  For() : init(nullptr), condition(nullptr), post(nullptr), body(nullptr) {}
};

struct Return {
  struct Expression *expression;
};

struct Print {
  struct Expression *format;
  std::vector<struct Expression *> args;
};

struct Input {
  struct Expression *path;
  struct Expression *format;
  struct Expression *destination;
};

struct Defer {
  struct Statement *statement;
  struct Block *block;
};

struct Break {};
struct Continue {};

struct Switch {
  struct Expression *expression;
  std::vector<struct Expression *> keys;
  std::vector<struct Expression *> values;
  struct Expression *default_;
};

struct Structure {

  enum AstType ast_type;
  union {
    struct EnumContainer *enum_;
    struct StructContainer *struct_;
  };
};

struct Statement {
  enum AstType ast_type;
  union {
    struct Function *function;
    struct If *if_;
    struct While *while_;
    struct For *for_;
    struct Assign *assign;
    struct Return *return_;
    struct Expression *expression;
    struct Print *print;
    struct Defer *defer;
    struct Break *break_;
    struct Continue *continue_;
    struct EnumContainer *enum_;
    struct Switch *switch_;
    struct Input *input;
  };
};

struct StructContainer {
  struct Identifier *name;
  std::vector<struct TypeWrapper *> types;
  std::vector<struct Identifier *> names;
  std::vector<struct Expression *>
      defaults; // In the output C code we perform assignments at run-time to
                // these members, which isn't great performance wise but it
                // probably wont matter if we don't have insane amounts of
                // structs
};

struct EnumContainer {
  struct Identifier *name;
  std::map<std::string, struct Expression *> mapping;
};

struct Array {
  bool is_dynamic_array = false;
  struct TypeWrapper *content_type;
  struct Expression *capacity;
};

struct Pointer {
  struct TypeWrapper *content_type;
};

struct TypeWrapper {

  struct Identifier *type_name;
  enum VarType var_type;

  boost::variant<struct Array, struct Pointer, struct EnumContainer *,
                 struct StructContainer *, struct Function *>
      content;

  TypeWrapper() {}
  TypeWrapper(enum VarType var_type) : var_type(var_type) {}
};
