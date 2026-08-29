#pragma once
#include "tokens.hpp"
#include "values.hpp"
#include <string>
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

// PIL parser
struct PILFile {
   std::string code;
   size_t lexeme;
};

PILFile readPIL(LexemeCache &cache, const std::string &path);
std::vector<Token> lexPILFile(LexemeCache &cache, PILFile &file);
void handlePILFileIncludes(LexemeCache &cache, PILFile &file, std::vector<Token> &tokens);

struct FunctionData {
   std::vector<Function> functionMap;
   std::vector<Block> blocks;
};

void pushBuiltin(LexemeCache &cache, FunctionData &data, const std::string &lexeme, NativeFunction function);
void defineStandardBuiltins(LexemeCache &cache, FunctionData &data);
void parsePIL(LexemeCache &cache, FunctionData &data, std::vector<Token> &tokens);
