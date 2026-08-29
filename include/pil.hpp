#pragma once
#include "error.hpp"
#include "tokens.hpp"
#include "values.hpp"
#include <string>

// PIL parser
struct PILFile {
   std::string code;
   size_t lexeme;
};

PILFile readPIL(Diagnostics &diagnostics, LexemeCache &cache, const std::string &path);
std::vector<Token> lexPILFile(Diagnostics &diagnostics, LexemeCache &cache, PILFile &file);
void handlePILFileIncludes(Diagnostics &diagnostics, LexemeCache &cache, PILFile &file, std::vector<Token> &tokens);

struct FunctionData {
   std::vector<Function> functionMap;
   std::vector<Block> blocks;
};

void pushBuiltin(LexemeCache &cache, FunctionData &data, const std::string &lexeme, NativeFunction function);
void defineStandardBuiltins(LexemeCache &cache, FunctionData &data);
void parsePIL(Diagnostics &diagnostics, LexemeCache &cache, FunctionData &data, std::vector<Token> &tokens);
