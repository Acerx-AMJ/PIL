#pragma once
#include <string>
#include <unordered_map>
#include <vector>

typedef size_t lexemeid_t;

enum TokenType {
   TOKEN_REGISTER, TOKEN_L_PAREN, TOKEN_R_PAREN, TOKEN_REFERENCE, TOKEN_DEREFERENCE, TOKEN_COMMA,
   TOKEN_IDENTIFIER, TOKEN_INTEGER, TOKEN_FLOATING, TOKEN_STRING, TOKEN_CHARACTER,
   TOKEN_EOF, TOKEN_COUNT,
};

struct Token {
   Token(TokenType type, lexemeid_t lexeme, size_t line)
      : type(type), lexeme(lexeme), line(line) {}

   TokenType type;
   lexemeid_t lexeme;
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

// lexemes
inline std::vector<std::string> lexemes;
inline std::unordered_map<std::string, lexemeid_t> lexemeCache;

inline lexemeid_t pushLexeme(const std::string &lexeme) {
   if (auto it = lexemeCache.find(lexeme); it != lexemeCache.end()) {
      return it->second;
   }
   lexemeid_t id = lexemes.size();
   lexemes.push_back(lexeme);
   lexemeCache[lexeme] = id;
   return id;
}

inline const std::string &getLexeme(lexemeid_t id) {
   if (id < 0 || id >= lexemes.size()) {
      printf("Invalid lexeme ID %llu.\n", id);
      exit(EXIT_FAILURE);
   }
   return lexemes[id];
}
