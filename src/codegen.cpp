
#include "parser.hpp"
#include "tokens.hpp"
#include "types.hpp"
#include "utils.hpp"
#include <iostream>
#include <map>
#include <ostream>
struct GeneratorContext {

  std::vector<std::string> *functions;
  std::vector<std::string> *variables;
  std::string out;
};
static GeneratorContext *context;
static AST *tast;

std::string generate_variable(struct Variable *var);
std::map<enum AstType, std::string> ast_type_to_operator = {

    {AstType_Add, "+"},   {AstType_Sub, "-"},   {AstType_Mul, "*"},
    {AstType_Div, "/"},   {AstType_Mod, "%"},   {AstType_Eq, "=="},
    {AstType_Neq, "!="},  {AstType_Geq, ">="},  {AstType_Leq, "<="},
    {AstType_Great, ">"}, {AstType_Less, "<"},  {AstType_And, "&"},
    {AstType_Or, "|"},    {AstType_Land, "&&"}, {AstType_Lor, "||"},
    {AstType_Not, "!"},

};

std::string generate_literal(struct Literal *literal) {

  std::string out = "";

  switch (literal->type_wrapper->var_type) {
  case VarType_Int:
    debug("Generating int literal with value %d\n",
          boost::get<int>(literal->value));
    out += std::to_string(boost::get<int>(literal->value));
    break;
  case VarType_Float:
    debug("Generating float literal with value %f\n",
          boost::get<double>(literal->value));
    out += std::to_string(boost::get<double>(literal->value));
    break;
  case VarType_Char:
    debug("Generating char literal with value %c\n",
          boost::get<std::string>(literal->value).at(0));
    out += "'" + boost::get<std::string>(literal->value) + "'";
    break;
  case VarType_String:
    debug("Generating string literal with value %s\n",
          boost::get<std::string>(literal->value).c_str());
    out += "\"" + boost::get<std::string>(literal->value) + "\"";
    break;
  case VarType_Enum:
    debug("Generating enum literal with value %s\n",
          boost::get<std::string>(literal->value).c_str());
    out += std::to_string(boost::get<int>(literal->value));
    break;
  case VarType_Null:
    out += "NULL";
    break;
  default:
    return out;
  }

  return out;
}

std::string generate_expression(struct Expression *expr) {

  std::string out = "";

  switch (expr->ast_type) {

  case AstType_BinaryOp:
    out += generate_expression(expr->binary_op->left);
    out += ast_type_to_operator[expr->binary_op->ast_type];
    out += generate_expression(expr->binary_op->right);
    break;
  case AstType_Literal:
    out += generate_literal(expr->literal);
    break;
  case AstType_Identifier:
    out += expr->identifier->name;
    break;
  case AstType_FunctionCall:

    out += expr->function_call->name->name + "(";
    for (auto arg : expr->function_call->args) {
      out += generate_expression(arg) + ",";
    }
    if (expr->function_call->args.size() > 0) {
      out = out.substr(0, out.size() - 1);
    }
    out += ")";
    break;
  case AstType_Index:
    out += expr->index->name->name + "[";
    out += generate_expression(expr->index->idx) + "]";
    break;
  case AstType_Array: {
    out += "{";
    std::vector<struct Expression *> expressions =
        boost::get<std::vector<struct Expression *>>(expr->literal->value);
    // expr->literal->array_literal_values
    for (auto element : expressions) {
      out += generate_expression(element) + ",";
    }
    if (expressions.size() > 0) {
      out = out.substr(0, out.size() - 1);
    }
  }
    out += "}";
    break;
  case AstType_Dot:
    debug("Generating dot expression\n");
    // Now we have to figure out whether or not
    // this is en enum with namespace, or a structure.
    // Loop through all the structures and check if the left hand side is a
    // structure

    //@TODO : man this is not an efficient way to do it lol
    for (auto structure : tast->structures) {
      if (structure->ast_type == AstType_Enum) {
        debug("Generating enum member with name %s_%s\n",
              expr->dot->left->identifier->name.c_str(),
              expr->dot->right->identifier->name.c_str());
        out += expr->dot->left->identifier->name + "_" +
               expr->dot->right->identifier->name;
        return out;
      }
    }

    // If we didn't find a match, we can assume that it's a structure

    break;

  default:
    debug("ERROR: Unknown expression type\n");
    return "";
  }

  return out;
}

std::string generate_type(struct TypeWrapper *type) {

  std::string out = "";

  switch (type->var_type) {
  case VarType_Enum:
    out += "enum " + type->type_name->name;
    break;
  case VarType_Array: {
    //@TODO : this is a copy not a pointer extraction so its inefficient but
    // boost didnt want to work properly
    struct Array array = boost::get<struct Array>(type->content);
    out += generate_type(array.content_type);
    if (array.capacity != NULL) {
      out += "[" + generate_expression(array.capacity) + "]";
    } else {
      out += "[]";
    }
  }

  break;
  default:
    out += type_to_string[type->var_type];
  }

  return out;
}

std::string generate_structure(struct Structure *structure) {

  std::string out = "";
  switch (structure->ast_type) {

  case AstType_Enum:
    debug("Generating enum: '%s'\n", structure->enum_->name->name.c_str());

    out += "enum " + structure->enum_->name->name + "{\n";
    for (const auto &kv : structure->enum_->mapping) {
      debug("Generating enum '%s'\n", kv.first.c_str());

      // Here we should handle the case where no enum value was specified
      //@NOTE We don't need to have an if-else control thing here we could just
      // add it to the string
      if (kv.second == NULL) {
        out += "  " + structure->enum_->name->name + "_" + kv.first + ",\n";
        debug("Generated enum member without value\n");
        continue;
      } else {
        out += structure->enum_->name->name + "_" + kv.first + "=" +
               generate_expression(kv.second) + ",\n";
        debug("Generated enum member with value\n");
      }
    }

    out += "};\n\n";
    break;
  default:
    err("ERROR: Unknown structure type\n");
  }
  return out;
}

std::string generate_array_assignment(struct Statement *stmt) {

  assert(0 && "Array assignment not implemented yet");
}
std::string generate_statement(struct Statement *stmt) {

  std::string out = "";

  switch (stmt->ast_type) {
  case AstType_Assign:

    // Is a declaration
    if (stmt->assign->type_wrapper->var_type != VarType_Unknown) {
      if (stmt->assign->type_wrapper->var_type == VarType_Array) {
        out += generate_array_assignment(stmt);
        return out;
      }
      out += generate_type(stmt->assign->type_wrapper) + " ";
    }

    out += stmt->assign->identifier->name;
    debug("Generating assignment for %s\n",
          stmt->assign->identifier->name.c_str());
    debug("Type: %s\n",
          type_to_string[stmt->assign->type_wrapper->var_type].c_str());
    // check the right hand side

    if (stmt->assign->right != NULL) {
      debug("Right hand side: %s\n",
            generate_expression(stmt->assign->right).c_str());

      debug("Generating assignment\n");
      out += " = " + generate_expression(stmt->assign->right);
    }

    out += ";\n";
    break;

  case AstType_If:
    debug("generating code for if\n");
    out += "if (" + generate_expression(stmt->if_->condition) + "){\n";
    for (auto s : stmt->if_->body->statements) {
      out += generate_statement(s);
    }

    out += "}";

    if (stmt->if_->else_ != NULL) {
      out += "else{\n";
      for (auto s : stmt->if_->else_->statements) {
        out += generate_statement(s);
      }
      out += "}";
    }
    break;
  case AstType_While:
    debug("generating code for while\n");
    out += "while (" + generate_expression(stmt->while_->condition) + "){\n";
    for (auto s : stmt->while_->body->statements) {
      out += generate_statement(s);
    }
    out += "}";
    break;
  case AstType_Return:
    out += "return " + generate_expression(stmt->return_->expression) + ";\n";
    break;
  case AstType_Expression:
    out += generate_expression(stmt->expression) + ";\n";
    break;
  case AstType_Print:
    out += "printf(";
    out += generate_expression(stmt->print->format);
    if (stmt->print->args.size() > 0) {
      out += ",";
    }
    for (auto arg : stmt->print->args) {
      out += generate_expression(arg) + ",";
    }
    if (stmt->print->args.size() > 0) {
      out = out.substr(0, out.size() - 1);
    }
    out += ");\n";
    break;
  case AstType_Input:
    out += "FILE *fp;\n";
    out += "long lSize;\n";
    out +=
        "fp = fopen(" + generate_expression(stmt->input->path) + ", \"rb\");\n";
    out += "if (!fp) perror(" + generate_expression(stmt->input->path) +
           "), exit(1);\n";
    out += "fseek(fp, 0L, SEEK_END);\n";
    out += "lSize = ftell(fp);\n";
    out += "rewind(fp);\n";
    out += generate_expression(stmt->input->destination) +
           " = (char *)calloc(1, lSize + 1);\n";
    out += "if (!" + generate_expression(stmt->input->destination) +
           ") fclose(fp), fputs(\"memory alloc fails\", stderr), "
           "exit(1);\n";
    out += "if (1 != fread(" + generate_expression(stmt->input->destination) +
           ", lSize, 1, fp)) fclose(fp), free(" +
           generate_expression(stmt->input->destination) +
           "), "
           "fputs(\"entire read fails\", stderr), exit(1);\n";
    out += "fclose(fp);\n";

    break;
  case AstType_Break:
    out += "break;\n";
    break;
  case AstType_Continue:
    out += "continue;\n";
    break;
  case AstType_Defer:
    // Defer does nothing directly, so we just skip it here
    break;
  case AstType_Switch: {

    out +=
        "  switch (" + generate_expression(stmt->switch_->expression) + ") {\n";
    for (int i = 0; i < stmt->switch_->keys.size(); i++) {
      // Get the name of the enum
      out += "    case " + generate_expression(stmt->switch_->keys[i]) + ": ";
      out += "return " + generate_expression(stmt->switch_->values[i]) + ";\n";
    }

    if (stmt->switch_->default_ != NULL) {
      out += "    default: return " +
             generate_expression(stmt->switch_->default_) + ";\n";
    } else {
      out += "   default: break;\n";
    }

    out += "  }\n";

    break;
  }
  default:
    debug("Unknown statement type %s\n",
          ast_type_to_string[stmt->ast_type].c_str());
    break;
  }

  return out;
}

std::string generate_parameters(struct Function *function) {
  std::string out = "";

  for (auto parameter : function->params) {
    out += generate_variable(parameter) + ",";
  }

  debug("Generated parameters: %s\n", out.c_str());

  // Remove the last two characters
  out = out.substr(0, out.size() - 2);

  return out;
}

std::string generate_function(struct Function *function) {

  debug("Generating function %s\n", function->name->name.c_str());
  // Function is not codegen invariant, and just contains pure C, so we can just
  // print it.
  if (function->raw_c.size() != NULL) {
    return function->raw_c;
  }

  std::string out =
      generate_type(function->return_type) + " " + function->name->name + "(";
  out += generate_parameters(function);
  out += "){\n";
  for (auto statement : function->body->statements) {
    out += generate_statement(statement);
  }
  out += "}\n\n";
  return out;
}

std::string generate_variable(struct Variable *var) {
  debug("Generating variable %s\n", var->name->name.c_str());
  if (var->type_wrapper->var_type == VarType_Enum) {

    struct EnumContainer *enum_ =
        boost::get<struct EnumContainer *>(var->type_wrapper->content);
    debug(",&&%s\n", enum_->name->name.c_str());

    return "enum " + enum_->name->name + " " + var->name->name + ";";
  }
  return type_to_string[var->type_wrapper->var_type] + " " + var->name->name +
         ";";
}

std::string generate(struct AST *ast) {

  debug("Starting codegen\n");
  tast = ast;
  context = new GeneratorContext();
  context->functions = new std::vector<std::string>();
  context->variables = new std::vector<std::string>();
  context->out = "#include <stdio.h>\n#include <stdlib.h>\n#include "
                 "<string.h>\n#include <assert.h>\n\n";

  // Structures
  debug("Generating structures\n");
  context->out += "/*STRUCTURES*/\n";
  for (auto structure : ast->structures) {
    context->out += generate_structure(structure);
  }

  // Global Variables.
  debug("Generating global variables\n");
  context->out += "/*GLOBALS*/\n";
  for (auto variable : ast->variables) {
    context->out += generate_statement(variable);
  }
  context->out += "\n";

  // Functions
  debug("Generating functions\n");
  context->out += "/*FUNCTIONS*/\n";
  for (auto function : ast->functions) {
    context->out += generate_function(function);
  }

  return context->out;
}