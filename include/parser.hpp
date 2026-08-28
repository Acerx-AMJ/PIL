#pragma once
#include "tokens.hpp"
#include <vector>

struct Parser {
   void lex(const std::string &code);
   void insertToken(TokenType type, const std::string &lexeme);
   char handleEscapeCode(const std::string &code, size_t &i);

   void parse();

   std::vector<Token> tokens;
   size_t tokenLine = 1;
};
