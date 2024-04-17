#pragma once

#include "codegen.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "utils.hpp"
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

using namespace std;

int main(int argc, char *argv[]) {

  debug("Starting the compiler\n");
  FILE *fp;
  long lSize;
  char *buffer;

  fp = fopen(argv[1], "rb");
  if (!fp)
    perror(argv[1]), exit(1);

  fseek(fp, 0L, SEEK_END);
  lSize = ftell(fp);
  rewind(fp);
  debug("Filesize: %ld\n", lSize);

  /* allocate memory for entire content */
  buffer = (char *)calloc(1, lSize + 1);
  if (!buffer)
    fclose(fp), fputs("memory alloc fails", stderr), exit(1);

  /* copy the file into the buffer */
  if (1 != fread(buffer, lSize, 1, fp))
    fclose(fp), free(buffer), fputs("entire read fails", stderr), exit(1);

  auto [head, directives] = lex(buffer, lSize);
  fclose(fp);
  free(buffer);
  if (head == nullptr) {
    return 0;
  }
  // Parsing
  struct AST *ast = parse(head);
  // Codegen
  std::string code = generate(ast);

  // Write to file
  std::ofstream out("out.c");
  out << code;
  out.close();
}
