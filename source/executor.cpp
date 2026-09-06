#include "builtin.hpp"
#include "pil.hpp"

constexpr size_t DEFAULT_REGISTER_COUNT = 16;
constexpr size_t DEFAULT_RETURN_REGISTER_COUNT = 4;

void call(Executor &executor, const Command &command, ParseValue &function, size_t functionPos, size_t returnCount, size_t args) {
   if (function.type == NATIVE_FUNCTION) {
      function.nativeFunction(command, executor);
   }
   else if (function.type == FUNCTION) {
      Trace trace (executor.pointer, command.lexeme, returnCount);
      trace.locals = std::vector<Value>(function.localCount, Value{VALUE_COUNT});

      for (size_t i = functionPos + 1; i < functionPos + 1 + function.params.size(); ++i) {
         Value value = resolveVariable(executor, executor.arguments[i + command.argStart], "call", command.file, command.line);
         moveValue(executor, trace.locals[i - functionPos - 1], value);
      }
      executor.stackTrace.push(trace);
      executor.pointer = function.function - 1;
   }
   else {
      error(executor.diagnostics, command.file, command.line, "Stray %s '%s'", getParseValueName(function.type), getLexeme(executor.cache, command.lexeme).c_str());
   }
}

// execute the function that the pointer is on. return and call logic can be found in the builtin header since they're just
// callable functions.
void callPILFunction(Executor &executor, const std::string &name, ErrorSeverity stopSeverity) {
   size_t lexeme = cacheLexeme(executor.cache, name);
   if (lexeme >= executor.values.size() || !executor.values[lexeme].init || executor.values[lexeme].type != FUNCTION) {
      error(executor.diagnostics, 0, 0, "Function '%s' cannot be called as it is not defined", name.c_str());
      return;
   }

   if (!executor.values[lexeme].params.empty() || executor.values[lexeme].variadic) {
      error(executor.diagnostics, 0, 0, "Attempted to call function '%s' with 0 arguments", name.c_str());
      return;
   }

   if (executor.registers.empty()) {
      executor.registers.resize(DEFAULT_REGISTER_COUNT);
   }

   if (executor.returnRegisters.empty()) {
      executor.returnRegisters.resize(DEFAULT_RETURN_REGISTER_COUNT);
   }
   executor.stackTrace = {};
   executor.pointer = executor.values[lexeme].function;
   executor.returnCount = 0;
   executor.exitCalled = false;

   while (true) {
      Command &command = executor.code[executor.pointer];
      ParseValue &function = executor.values[command.lexeme];
      call(executor, command, function, -1, 0, command.argCount);
      if (executor.exitCalled || shouldError(executor.diagnostics, stopSeverity)) {
         break;
      }
      executor.pointer += 1;
   }
}
