#include "cache.hpp"

// lexeme cache
size_t pushLexeme(LexemeCache &cache, const std::string &lexeme) {
   size_t id = cache.lexemes.size();
   cache.lexemes.push_back(lexeme);
   return id;
}

size_t cacheLexeme(LexemeCache &cache, std::string_view sv) {
   if (auto it = cache.lexemeCache.find(sv); it != cache.lexemeCache.end()) {
      return it->second;
   }
   std::string lexeme = std::string(sv);
   size_t id = cache.lexemes.size();
   cache.lexemes.push_back(lexeme);
   cache.lexemeCache[lexeme] = id;
   return id;
}

std::string &getLexeme(LexemeCache &cache, size_t id) {
   if (id < 0 || id >= cache.lexemes.size()) {
      printf("PIL::getLexeme: Invalid lexeme ID %zu.\n", id);
      exit(EXIT_FAILURE);
   }
   return cache.lexemes[id];
}

size_t getLexemeCount(LexemeCache &cache) {
   return cache.lexemes.size();
}
