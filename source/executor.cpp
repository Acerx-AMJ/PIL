#include "builtinhelpers.hpp"
#include "pil.hpp"

void call(Executor &executor, const Command &command, Function &function, size_t functionPos, size_t returnCount, size_t args) {
   if (function.native) {
      // printf("called native %s. Stack trace: %zu.\n", getLexeme(executor.cache, command.lexeme).c_str(), executor.stackTrace.size());
      function.nativeFunction(command, executor);
   }
   else if (!function.isLabel) {
      // printf("called %s @ %s:%zu. Stack trace: %zu.\n", getLexeme(executor.cache, function.lexeme).c_str(), getLexeme(executor.cache, executor.code[function.position].file).c_str(), executor.code[function.position].line, executor.stackTrace.size());
      size_t params = function.params.size();
      size_t offset = command.argStart + functionPos + 1;

      Trace trace (executor.pointer, command.argStart, returnCount);
      trace.localStart = executor.locals.size();
      trace.localCount = function.localCount;
      trace.variadicCount = command.argCount - params;
      executor.locals.resize(trace.localStart + trace.localCount + trace.variadicCount, Value{VALUE_COUNT});

      for (size_t i = 0; i < params; ++i) {
         Value value = resolveVariable(executor, executor.arguments[i + offset]);
         executor.locals[trace.localStart + i] = value;
      }
      if (function.variadic) {
         size_t defines = function.localCount - params;
         for (size_t i = trace.localCount; i < trace.localCount + trace.variadicCount; ++i) {
            Value value = resolveVariable(executor, executor.arguments[i + offset - defines]);
            executor.locals[trace.localStart + i] = value;
         }
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
   executor.locals.resize(main.localCount, Value{VALUE_COUNT});

   executor.stackTrace = {};
   executor.stackTrace.push(Trace(main.position, std::string::npos, std::string::npos));
   executor.stackTrace.top().localStart = 0;
   executor.stackTrace.top().localCount = main.localCount;
   executor.stackTrace.top().variadicCount = 0;

   executor.pointer = main.position;
   executor.returnCount = 0;
   executor.exitCalled = false;

   while (true) {
      // printf("Pointer @ %zu.\n", executor.pointer);
      Command &command = executor.code[executor.pointer];
      Function &function = executor.functions[command.functionId];
      call(executor, command, function, -1, std::string::npos, command.argCount);
      if (executor.exitCalled || shouldError(executor.diagnostics, stopSeverity)) {
         break;
      }
      executor.pointer += 1;
   }
   executor.locals.resize(0);
   executor.stackTrace.pop();
}
