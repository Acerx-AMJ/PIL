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

struct Trace {
   Trace(size_t position, size_t lexeme, size_t callExpectedReturnCount)
      : position(position), lexeme(lexeme), callExpectedReturnCount(callExpectedReturnCount) {}

   size_t position;
   size_t lexeme;
   size_t callExpectedReturnCount;
   std::vector<Value> locals;
};

struct Executor {
   Executor(Diagnostics &diagnostics, LexemeCache &cache)
      : diagnostics(diagnostics), cache(cache) {}

   Diagnostics &diagnostics;
   LexemeCache &cache;

   std::vector<Value> registers;
   std::vector<Value> returnRegisters;
   std::stack<Trace, std::vector<Trace>> stackTrace;

   std::unordered_map<size_t, std::string> strings;
   std::vector<ParseValue> values;
   std::vector<Command> code;

   size_t pointer;
   size_t returnCount;
   bool exitCalled;
};

PILFile readPIL(Diagnostics &diagnostics, LexemeCache &cache, const std::string &path);
std::vector<Token> lexPILFile(Diagnostics &diagnostics, LexemeCache &cache, PILFile &file);
void translatePIL(Executor &executor, PILFile &file, std::vector<Token> &tokens);

void pushBuiltin(Executor &executor, const std::string &lexeme, NativeFunction function, size_t paramCount, bool variadic);
void pushReservedBuiltin(Executor &executor, const std::string &lexeme, NativeFunction function, size_t paramCount, bool variadic);
void defineStandardBuiltins(Executor &executor);

void parsePIL(Executor &executor, std::vector<Token> &tokens);
void callPILFunction(Executor &executor, const std::string &name, ErrorSeverity stopSeverity);

std::string &getString(Executor &executor, size_t ID, size_t file, size_t line);
size_t allocateString(Executor &executor, const std::string &string);
