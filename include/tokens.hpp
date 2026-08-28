#pragma once
#include <cstddef>

enum TokenType {
   TOKEN_REGISTER, TOKEN_L_PAREN, TOKEN_R_PAREN, TOKEN_REFERENCE, TOKEN_DEREFERENCE, TOKEN_COMMA,
   TOKEN_IDENTIFIER, TOKEN_INTEGER, TOKEN_FLOATING, TOKEN_STRING, TOKEN_CHARACTER,
   TOKEN_EOF, TOKEN_COUNT,
};

struct Token {
   Token(TokenType type, size_t lexeme, size_t line)
      : type(type), lexeme(lexeme), line(line) {}

   TokenType type;
   size_t lexeme;
   size_t line = 0;
};

constexpr const char *tokenTypeStrings[TOKEN_COUNT + 1] = {
   "Register", "Left Parentheses", "Right Parentheses", "Reference", "Dereference", "Comma",
   "Identifier", "Integer", "Floating", "String", "Character",
   "EOF", "Invalid Token",
};

constexpr const char *getTokenName(TokenType token) {
   if (token < 0 || token >= TOKEN_COUNT) {
      return tokenTypeStrings[TOKEN_COUNT];
   }
   return tokenTypeStrings[token];
}
