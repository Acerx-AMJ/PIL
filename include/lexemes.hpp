#pragma once
#include <string>
#include <unordered_map>
#include <vector>

size_t pushLexeme(const std::string &lexeme);
size_t cacheLexeme(const std::string &lexeme);
std::string &getLexeme(size_t lexeme);
size_t getLexemeCount();

extern std::vector<std::string> lexemes;
extern std::unordered_map<std::string, size_t> lexemeCache;
