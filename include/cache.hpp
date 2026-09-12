#pragma once
#include <string>
#include <vector>
#include <unordered_map>

struct TransparentHash {
   using is_transparent = void;
   size_t operator()(std::string_view sv) const { return std::hash<std::string_view>{}(sv); }
};

struct TransparentEq {
   using is_transparent = void;
   bool operator()(std::string_view a, std::string_view b) const { return a == b; }
};

// lexeme cache
struct LexemeCache {
   std::vector<std::string> lexemes;
   std::unordered_map<std::string, size_t, TransparentHash, TransparentEq> lexemeCache;
};

size_t pushLexeme(LexemeCache &cache, const std::string &lexeme);
size_t cacheLexeme(LexemeCache &cache, std::string_view sv);
std::string &getLexeme(LexemeCache &cache, size_t id);
size_t getLexemeCount(LexemeCache &cache);
