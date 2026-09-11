#pragma once
#include <cstddef>

enum TokenType: char {
   TOKEN_RETURN_REGISTER, TOKEN_REGISTER, TOKEN_L_PAREN, TOKEN_R_PAREN, TOKEN_VARIADIC, TOKEN_LABEL,
   TOKEN_L_BRACKET, TOKEN_R_BRACKET, TOKEN_PLUS, TOKEN_MINUS, TOKEN_STAR, TOKEN_SLASH, TOKEN_PERCENT, TOKEN_CARET,
   TOKEN_IDENTIFIER, TOKEN_INTEGER, TOKEN_FLOATING, TOKEN_STRING, TOKEN_CHARACTER,
   TOKEN_NEWLINE, TOKEN_EOF, TOKEN_COUNT,
};

constexpr const char *tokenTypeStrings[TOKEN_COUNT + 1] = {
   "Return Register", "Register", "Left Parentheses", "Right Parentheses", "Three Dots", "Colon",
   "Left Bracket", "Right Bracket", "Plus", "Minus", "Star", "Slash", "Percent", "Caret",
   "Identifier", "Integer", "Floating", "String", "Character",
   "Newline", "EOF", "Invalid Token",
};

constexpr const char *getTokenName(TokenType token) {
   if (token < 0 || token >= TOKEN_COUNT) {
      return tokenTypeStrings[TOKEN_COUNT];
   }
   return tokenTypeStrings[token];
}

struct Token {
   Token(TokenType type, size_t lexeme, size_t file, size_t line)
      : parsed(false), type(type), lexeme(lexeme), file(file), line(line) {}

   bool parsed;
   TokenType type;
   size_t lexeme;
   size_t file;
   size_t line;
};
