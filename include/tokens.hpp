#pragma once
#include <cstddef>

enum TokenType: char {
   TOKEN_REGISTER, TOKEN_L_PAREN, TOKEN_R_PAREN,
   TOKEN_IDENTIFIER, TOKEN_INTEGER, TOKEN_FLOATING, TOKEN_STRING, TOKEN_CHARACTER,
   TOKEN_EOF, TOKEN_COUNT,
};

constexpr const char *getTokenName(TokenType token) {
   constexpr const char *tokenTypeStrings[TOKEN_COUNT + 1] = {
      "Register", "Left Parentheses", "Right Parentheses",
      "Identifier", "Integer", "Floating", "String", "Character",
      "EOF", "Invalid Token",
   };

   if (token < 0 || token >= TOKEN_COUNT) {
      return tokenTypeStrings[TOKEN_COUNT];
   }
   return tokenTypeStrings[token];
}

struct Token {
   Token(TokenType type, size_t lexeme, size_t fileLexeme, size_t line)
      : type(type), lexeme(lexeme), fileLexeme(fileLexeme), line(line) {}

   bool parsed = false;
   TokenType type;
   size_t lexeme;
   size_t fileLexeme;
   size_t line = 0;
};
