#include "builtin.hpp"
#include "pil.hpp"

void call(Executor &executor, const Command &command, Function &function, size_t functionPos, size_t returnCount, size_t args) {
   if (function.native) {
      function.nativeFunction(command, executor);
   }
   else if (!function.isLabel) {
      Trace trace (executor.pointer, command.lexeme, command.argStart, returnCount);
      trace.localStart = executor.locals.size();
      trace.localCount = function.localCount;
      executor.locals.resize(trace.localStart + trace.localCount, Value{VALUE_COUNT});

      for (size_t i = functionPos + 1; i < functionPos + 1 + function.params.size(); ++i) {
         Value value = resolveVariable(executor, executor.arguments[i + command.argStart]);
         moveValue(executor, executor.locals[trace.localStart + (i - functionPos - 1)], value);
      }
      executor.stackTrace.push(trace);
      executor.pointer = function.position - 1;
   }
   else {
      error(executor.diagnostics, command.file, command.line, "Stray label '%s'", getLexeme(executor.cache, command.lexeme).c_str());
   }
}

// execute the function that the pointer is on. return and call logic can be found in the builtin header since they're just
// callable functions.
void callPILFunction(Executor &executor, const std::string &name, ErrorSeverity stopSeverity) {
   size_t lexeme = cacheLexeme(executor.cache, name);
   auto it = executor.constants.find(lexeme);
   if (it == executor.constants.end()) {
      error(executor.diagnostics, 0, 0, "Function '%s' cannot be called as it is not defined", name.c_str());
      return;
   }
   else if (it->second.type != VALUE_FUNCTION) {
      error(executor.diagnostics, 0, 0, "Cannot call '%s' as it is not a function", name.c_str());
      return;
   }

   Function &main = executor.functions[it->second.function];
   if (main.native) {
      error(executor.diagnostics, 0, 0, "Cannot call '%s' as it is a native function", name.c_str());
      return;
   }
   else if (!main.params.empty() || main.variadic) {
      error(executor.diagnostics, 0, 0, "Attempted to call function '%s' with 0 arguments", name.c_str());
      return;
   }

   if (executor.registers.empty()) {
      executor.registers.resize(DEFAULT_REGISTER_COUNT, Value{VALUE_COUNT});
   }

   if (executor.returnRegisters.empty()) {
      executor.returnRegisters.resize(DEFAULT_RETURN_REGISTER_COUNT, Value{VALUE_COUNT});
   }
   executor.locals.reserve(DEFAULT_LOCAL_RESERVE);
   executor.stackTrace = {};
   executor.pointer = main.position;
   executor.returnCount = 0;
   executor.exitCalled = false;

   while (true) {
      Command &command = executor.code[executor.pointer];
      Function &function = executor.functions[command.functionId];
      call(executor, command, function, -1, std::string::npos, command.argCount);
      if (executor.exitCalled || shouldError(executor.diagnostics, stopSeverity)) {
         break;
      }
      executor.pointer += 1;
   }
}
