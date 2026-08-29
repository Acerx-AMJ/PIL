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

struct ByteCode {
   std::vector<Function> functions;
   std::vector<Command> code;
};

void pushBuiltin(LexemeCache &cache, ByteCode &data, const std::string &lexeme, NativeFunction function, size_t paramCount, bool variadic);
void defineStandardBuiltins(LexemeCache &cache, ByteCode &data);
void parsePIL(Diagnostics &diagnostics, LexemeCache &cache, ByteCode &data, std::vector<Token> &tokens);

void callPILFunction(Diagnostics &diagnostics, LexemeCache &cache, ByteCode &data, const std::string &name);
