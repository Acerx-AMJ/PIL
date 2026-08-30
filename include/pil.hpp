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

struct Trace {
   Trace(size_t position, size_t lexeme, size_t callExpectedReturnCount)
      : position(position), lexeme(lexeme), callExpectedReturnCount(callExpectedReturnCount) {}

   size_t position;
   size_t lexeme;
   size_t callExpectedReturnCount;
};

struct Executor {
   Executor(Diagnostics &diagnostics, LexemeCache &cache, ByteCode &code)
      : diagnostics(diagnostics), cache(cache), code(code) {}

   Diagnostics &diagnostics;
   LexemeCache &cache;
   ByteCode &code;

   std::vector<Value> registers;
   std::vector<Value> returnRegisters;
   std::stack<Trace> stackTrace;
   size_t pointer;
   size_t returnCount;
   bool exitCalled;
};

PILFile readPIL(Diagnostics &diagnostics, LexemeCache &cache, const std::string &path);
std::vector<Token> lexPILFile(Diagnostics &diagnostics, LexemeCache &cache, PILFile &file);
void translatePIL(Executor &executor, PILFile &file, std::vector<Token> &tokens);

void pushBuiltin(LexemeCache &cache, ByteCode &data, const std::string &lexeme, NativeFunction function, size_t paramCount, bool variadic);
void pushReservedBuiltin(LexemeCache &cache, ByteCode &data, const std::string &lexeme, NativeFunction function, size_t paramCount, bool variadic);
void defineStandardBuiltins(LexemeCache &cache, ByteCode &data);

void parsePIL(Diagnostics &diagnostics, LexemeCache &cache, ByteCode &data, std::vector<Token> &tokens);
void callPILFunction(Executor &executor, const std::string &name, ErrorSeverity stopSeverity);
