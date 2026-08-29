#pragma once
#include <string>
#include <vector>
#include <unordered_map>

// lexeme cache
struct LexemeCache {
   std::vector<std::string> lexemes;
   std::unordered_map<std::string, size_t> lexemeCache;
};

size_t pushLexeme(LexemeCache &cache, const std::string &lexeme);
size_t cacheLexeme(LexemeCache &cache, const std::string &lexeme);
std::string &getLexeme(LexemeCache &cache, size_t id);
size_t getLexemeCount(LexemeCache &cache);
