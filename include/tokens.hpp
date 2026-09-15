#pragma once
#include <cstddef>

enum TokenType: char {
   TOKEN_RETURN_REGISTER, TOKEN_REGISTER, TOKEN_L_PAREN, TOKEN_R_PAREN, TOKEN_VARIADIC, TOKEN_LABEL,
   TOKEN_L_BRACKET, TOKEN_R_BRACKET, TOKEN_PLUS, TOKEN_MINUS, TOKEN_STAR, TOKEN_SLASH, TOKEN_PERCENT,
   TOKEN_LOR, TOKEN_LAND, TOKEN_LNOT, TOKEN_BOR, TOKEN_BXOR, TOKEN_BAND, TOKEN_BNOT, TOKEN_EQUAL, TOKEN_INEQUAL,
   TOKEN_LESSER, TOKEN_LESSER_EQUAL, TOKEN_GREATER, TOKEN_GREATER_EQUAL, TOKEN_BSHL, TOKEN_BSHR, TOKEN_STAR_STAR,
   TOKEN_DIRECTIVE, TOKEN_IDENTIFIER, TOKEN_INTEGER, TOKEN_FLOATING, TOKEN_STRING, TOKEN_CHARACTER,
   TOKEN_NEWLINE, TOKEN_EOF, TOKEN_COUNT,
};

constexpr const char *tokenTypeStrings[TOKEN_COUNT + 1] = {
   "Return Register", "Register", "Left Parentheses", "Right Parentheses", "Three Dots", "Colon",
   "Left Bracket", "Right Bracket", "Add", "Subtract", "Multiply", "Divide", "Modulus",
   "Logical Or", "Logical And", "Logical Not", "Binary Or", "Binary Xor", "Binary And", "Binary Not", "Equal", "Inequal",
   "Lesser", "Lesser Equal", "Greater", "Greater Equal", "Bit Shift Left", "Bit Shift Right", "Exponentiate",
   "Directive", "Identifier", "Integer", "Floating", "String", "Character",
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
