#pragma once
#include "error.hpp"
#include "tokens.hpp"
#include "values.hpp"
#include <stack>
#include <string>

constexpr size_t DEFAULT_REGISTER_COUNT = 16;
constexpr size_t DEFAULT_RETURN_REGISTER_COUNT = 4;
constexpr size_t DEFAULT_LOCAL_RESERVE = 64;

// PIL parser
struct PILFile {
   std::string code;
   size_t lexeme;
};

struct Trace {
   Trace(size_t position, size_t lexeme, size_t callArgStart, size_t callArgCount)
      : position(position), lexeme(lexeme), callArgStart(callArgStart), callArgCount(callArgCount) {}

   size_t position;
   size_t lexeme;
   size_t callArgStart;
   size_t callArgCount;
   size_t localStart;
   size_t localCount;
};

struct Command {
   Command(size_t lexeme, size_t file, size_t line, size_t argStart, size_t argCount, size_t functionId)
      : lexeme(lexeme), file(file), line(line), argStart(argStart), argCount(argCount), callee(std::string::npos), functionId(functionId) {}

   size_t lexeme;
   size_t file;
   size_t line;
   size_t argStart;
   size_t argCount;
   size_t callee;
   size_t functionId;
};

struct PILString {
   std::string string;
   int allocations = 0;
};

struct Executor {
   Executor(Diagnostics &diagnostics, LexemeCache &cache)
      : diagnostics(diagnostics), cache(cache) {}

   Diagnostics &diagnostics;
   LexemeCache &cache;

   std::vector<Value> registers;
   std::vector<Value> returnRegisters;
   std::stack<Trace, std::vector<Trace>> stackTrace;

   std::unordered_map<size_t, Value> constants;
   std::unordered_map<size_t, PILString> strings;

   std::vector<Function> functions;
   std::vector<Value> locals;
   std::vector<Value> arguments;
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
void defineReservedBuiltins(Executor &executor);

Value parseToken(Executor &executor, Token token, const std::unordered_map<size_t, size_t> &functionParamMap, const std::unordered_map<size_t, Value> &constants);
void parsePIL(Executor &executor, std::vector<Token> &tokens);

void call(Executor &executor, const Command &command, Function &function, size_t functionPos, size_t returnCount, size_t argCount);
void callPILFunction(Executor &executor, const std::string &name, ErrorSeverity stopSeverity);

// allocation
std::string &getString(Executor &executor, size_t ID, size_t file, size_t line);
size_t allocateString(Executor &executor, const std::string &string);

// debug
void measure();
float measureEnd();

void debugTokens(LexemeCache &cache, const std::vector<Token> &tokens);
void debugBytecode(Executor &executor);
void debugExecutionTime(float file, float lexer, float translator, float parser, float runtime);
