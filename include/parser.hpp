#pragma once
#include "tokens.hpp"
#include <string>
#include <vector>

struct Parser {
   void lex(const std::string &code);
   char handleEscapeCode(const std::string &code, size_t &i);

   void parse();

   std::vector<Token> tokens;
   size_t tokenLine = 1;
};
