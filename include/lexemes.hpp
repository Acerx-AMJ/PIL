#pragma once
#include <string>

size_t pushLexeme(const std::string &lexeme);
size_t cacheLexeme(const std::string &lexeme);
std::string &getLexeme(size_t lexeme);
