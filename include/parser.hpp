#pragma once
#include "tokens.hpp"
#include "values.hpp"
#include <string>
#include <unordered_map>

struct Parser {
   void lex(const std::string &code, size_t fileLexeme);
   void handleIncludes();
   void parse();

   std::vector<Token> tokens;
   size_t tokenLine = 1;

   std::unordered_map<size_t, size_t> functionDefinitions;
   std::vector<Block> functions;
   size_t valueLine = 1;
};
