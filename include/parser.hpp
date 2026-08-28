#pragma once
#include "tokens.hpp"
#include "values.hpp"
#include <string>

struct Parser {
   void lex(const std::string &code, size_t fileLexeme);
   void handleIncludes();

   void pushBuiltin(const std::string &identifier, NativeFunction func);
   void defineBuiltins();
   void parse();

   std::vector<Token> tokens;
   size_t tokenLine = 1;
   size_t lexerFileLexeme;

   std::vector<Function> functionMap;
   std::vector<Block> blocks;
   size_t valueLine = 1;
};
