#pragma once
#include "error.hpp"
#include "tokens.hpp"
#include "values.hpp"
#include <stack>
#include <string>

// PIL parser
struct PILFile {
   std::string code;
   size_t lexeme;
};

struct ByteCode {
   std::vector<Function> functions;
   std::vector<Command> code;
};

struct Executor {
   Executor(Diagnostics &diagnostics, LexemeCache &cache, ByteCode &code, size_t pointer)
      : diagnostics(diagnostics), cache(cache), code(code), pointer(pointer) {}

   Diagnostics &diagnostics;
   LexemeCache &cache;
   ByteCode &code;

   std::vector<Value> registers {16, Value{}};
   std::stack<size_t> stack;
   size_t pointer;
   bool exitCalled = false;
};

PILFile readPIL(Diagnostics &diagnostics, LexemeCache &cache, const std::string &path);
std::vector<Token> lexPILFile(Diagnostics &diagnostics, LexemeCache &cache, PILFile &file);
void handlePILFileIncludes(Diagnostics &diagnostics, LexemeCache &cache, PILFile &file, std::vector<Token> &tokens);

void pushBuiltin(LexemeCache &cache, ByteCode &data, const std::string &lexeme, NativeFunction function, size_t paramCount, bool variadic);
void pushReservedBuiltin(LexemeCache &cache, ByteCode &data, const std::string &lexeme, NativeFunction function, size_t paramCount, bool variadic);
void defineStandardBuiltins(LexemeCache &cache, ByteCode &data);

void parsePIL(Diagnostics &diagnostics, LexemeCache &cache, ByteCode &data, std::vector<Token> &tokens);
void callPILFunction(Diagnostics &diagnostics, LexemeCache &cache, ByteCode &data, const std::string &name, ErrorSeverity stopSeverity);
