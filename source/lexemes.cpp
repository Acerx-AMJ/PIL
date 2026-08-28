#include "lexemes.hpp"
#include <unordered_map>
#include <vector>

inline std::vector<std::string> lexemes;
inline std::unordered_map<std::string, size_t> lexemeCache;

size_t pushLexeme(const std::string &lexeme) {
   size_t id = lexemes.size();
   lexemes.push_back(lexeme);
   return id;
}

size_t cacheLexeme(const std::string &lexeme) {
   if (auto it = lexemeCache.find(lexeme); it != lexemeCache.end()) {
      return it->second;
   }
   size_t id = lexemes.size();
   lexemes.push_back(lexeme);
   lexemeCache[lexeme] = id;
   return id;
}

std::string &getLexeme(size_t id) {
   if (id < 0 || id >= lexemes.size()) {
      printf("Invalid lexeme ID %llu.\n", id);
      exit(EXIT_FAILURE);
   }
   return lexemes[id];
}
