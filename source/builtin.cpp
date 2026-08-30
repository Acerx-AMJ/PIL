#include "builtin.hpp"
#include "pil.hpp"
#include <format>

// helper functions
Value resolveVariable(Executor &executor, Value value, const char *function) {
   if (value.type == VALUE_LOCAL) {
      return executor.stackTrace.top().locals[value.local];
   }

   if (value.type == VALUE_IDENTIFIER) {
      if (!executor.code.values[value.identifier].init || executor.code.values[value.identifier].type != GLOBAL) {
         error(executor.diagnostics, std::format("{}: Variable '{}' does not exist", function, getLexeme(executor.cache, value.identifier)), value.fileLexeme, value.line);
         return value;
      }
      return executor.code.values[value.identifier].global;
   }

   if (value.type != VALUE_REGISTER && value.type != VALUE_RETURN_REGISTER) {
      return value;
   }
   std::vector<Value> &registers = (value.type == VALUE_RETURN_REGISTER ? executor.returnRegisters : executor.registers);

   if (value.reg < 0 || value.reg >= registers.size()) {
      error(executor.diagnostics, std::format("{}: Register {}${} is out of bounds", function, value.type == VALUE_RETURN_REGISTER ? "R" : "", value.reg), value.fileLexeme, value.line);
      return value;
   }
   return registers[value.reg];
}

bool registerOrError(Executor &executor, Value value, const char *function, const char *argument) {
   if (value.type == VALUE_LOCAL) {
      return false;
   }

   if (value.type == VALUE_IDENTIFIER) {
      if (!executor.code.values[value.identifier].init || executor.code.values[value.identifier].type != GLOBAL) {
         error(executor.diagnostics, std::format("{}: Expected Register/Variable for the {} argument, got {} instead", function, argument, getParseValueName(executor.code.values[value.identifier].type)), value.fileLexeme, value.line);
         return true;
      }
      return false;
   }

   if (value.type != VALUE_REGISTER && value.type != VALUE_RETURN_REGISTER) {
      error(executor.diagnostics, std::format("{}: Expected Register/Variable for the {} argument, got {} instead", function, argument, getValueName(value.type)), value.fileLexeme, value.line);
      return true;
   }
   std::vector<Value> &registers = (value.type == VALUE_RETURN_REGISTER ? executor.returnRegisters : executor.registers);

   if (value.reg < 0 || value.reg >= registers.size()) {
      error(executor.diagnostics, std::format("{}: Register {}${} is out of bounds", function, value.type == VALUE_RETURN_REGISTER ? "R" : "", value.reg), value.fileLexeme, value.line);
      return true;
   }
   return false;
}

void storeInRegister(Executor &executor, Value reg, Value value) {
   // jedi mind tricks
   if (reg.type == VALUE_LOCAL) {
      executor.stackTrace.top().locals[reg.local] = value;
   }
   else if (reg.type == VALUE_IDENTIFIER) {
      executor.code.values[reg.identifier].global = value;
   }
   else {
      (reg.type == VALUE_REGISTER ? executor.registers[reg.reg] : executor.returnRegisters[reg.reg]) = value;
   }
}

bool labelOrError(Executor &executor, Value value, const char *function, const char *argument) {
   if (value.type != VALUE_IDENTIFIER || value.identifier >= executor.code.values.size() || !executor.code.values[value.identifier].init || executor.code.values[value.identifier].type != LABEL) {
      error(executor.diagnostics, std::format("{}: Expected Label for the {} argument, got {} instead", function, argument, getValueName(value.type)), value.fileLexeme, value.line);
      return true;
   }
   return false;
}

// misc, temp
void builtinAdd(const Command &command, Executor &executor) {
   Value result;
   result.fileLexeme = command.file;
   result.line = command.line;
   result.type = VALUE_INTEGER;
   result.integer = 0;

   for (size_t i = 0; i < command.args.size() - 1; ++i) {
      Value arg = resolveVariable(executor, command.args[i], "add");
      result.integer += arg.integer;
   }

   if (registerOrError(executor, command.args.back(), "add", "destination")) return;
   storeInRegister(executor, command.args.back(), result);
}

void builtinPrint(const Command &command, Executor &executor) {
   for (size_t i = 0; i < command.args.size(); ++i) {
      Value arg = resolveVariable(executor, command.args[i], "print");
      switch (arg.type) {
      case VALUE_INTEGER:   printf("%ld", arg.integer); break;
      case VALUE_FLOATING:  printf("%F", arg.floating); break;
      case VALUE_CHARACTER: printf("%c", arg.character); break;
      case VALUE_STRING:    printf("%s", getLexeme(executor.cache, arg.string).c_str()); break;
      default: printf("(null)");
      }
   }
   putchar('\n');
}

// comparison
enum Comparison: char {
   COMPARISON_LESS, COMPARISON_GREATER, COMPARISON_EQUAL, COMPARISON_ERROR
};

Comparison compareValues(Executor &executor, Value lhs, Value rhs, const char *function, bool softie) {
   Value a = resolveVariable(executor, lhs, function);
   Value b = resolveVariable(executor, rhs, function);

   if ((a.type == VALUE_INTEGER || a.type == VALUE_FLOATING) && (b.type == VALUE_INTEGER || b.type == VALUE_FLOATING)) {
      double x = (a.type == VALUE_INTEGER) ? (double)a.integer : a.floating;
      double y = (b.type == VALUE_INTEGER) ? (double)b.integer : b.floating;
      return x < y ? COMPARISON_LESS : x > y ? COMPARISON_GREATER : COMPARISON_EQUAL;
   }
   else if (a.type == VALUE_CHARACTER && b.type == VALUE_CHARACTER) {
      return a.character < b.character ? COMPARISON_LESS : a.character > b.character ? COMPARISON_GREATER : COMPARISON_EQUAL;
   }
   else if (a.type == VALUE_STRING && b.type == VALUE_STRING) {
      int c = getLexeme(executor.cache, a.string).compare(getLexeme(executor.cache, b.string));
      return c < 0 ? COMPARISON_LESS : c > 0 ? COMPARISON_GREATER : COMPARISON_EQUAL;
   }

   if (!softie) {
      error(executor.diagnostics, std::format("{}: Cannot compare {} and {}", function, getValueName(a.type), getValueName(b.type)), lhs.fileLexeme, lhs.line);
      return COMPARISON_ERROR;
   }
   return COMPARISON_LESS; // not equal I guess
}

bool isThruthy(Executor &executor, Value value, const char *function, const char *argument, bool &ok) {
   Value v = resolveVariable(executor, value, function);
   ok = true;

   switch (v.type) {
   case VALUE_INTEGER:   return v.integer != 0;
   case VALUE_FLOATING:  return v.floating != 0.0;
   case VALUE_CHARACTER: return v.character != 0;
   case VALUE_STRING:    return !getLexeme(executor.cache, v.string).empty();
   default:
      error(executor.diagnostics, std::format("{}: Expected value for the {} argument, got {}", function, argument, getValueName(v.type)), value.fileLexeme, value.line);
      ok = false;
      return false;
   }
}

void storeBoolean(Executor &executor, Value reg, bool result, const char *function) {
   if (registerOrError(executor, reg, function, "destination")) return;
   Value out = reg;
   out.type = VALUE_INTEGER;
   out.integer = (result ? 1 : 0);
   storeInRegister(executor, reg, out);
}

void builtinLe(const Command &command, Executor &executor) {
   Comparison result = compareValues(executor, command.args[0], command.args[1], "le", false);
   if (result != COMPARISON_ERROR) storeBoolean(executor, command.args[2], result == COMPARISON_LESS, "le");
}

void builtinGr(const Command &command, Executor &executor) {
   Comparison result = compareValues(executor, command.args[0], command.args[1], "gr", false);
   if (result != COMPARISON_ERROR) storeBoolean(executor, command.args[2], result == COMPARISON_GREATER, "gr");
}

void builtinLeeq(const Command &command, Executor &executor) {
   Comparison result = compareValues(executor, command.args[0], command.args[1], "leeq", false);
   if (result != COMPARISON_ERROR) storeBoolean(executor, command.args[2], result != COMPARISON_GREATER, "leeq");
}

void builtinGreq(const Command &command, Executor &executor) {
   Comparison result = compareValues(executor, command.args[0], command.args[1], "greq", false);
   if (result != COMPARISON_ERROR) storeBoolean(executor, command.args[2], result != COMPARISON_LESS, "greq");
}

void builtinEq(const Command &command, Executor &executor) {
   Comparison result = compareValues(executor, command.args[0], command.args[1], "eq", true);
   if (result != COMPARISON_ERROR) storeBoolean(executor, command.args[2], result == COMPARISON_EQUAL, "eq");
}

void builtinNeq(const Command &command, Executor &executor) {
   Comparison result = compareValues(executor, command.args[0], command.args[1], "neq", true);
   if (result != COMPARISON_ERROR) storeBoolean(executor, command.args[2], result != COMPARISON_EQUAL, "neq");
}

void builtinNot(const Command &command, Executor &executor) {
   bool ok;
   bool thruthy = isThruthy(executor, command.args[0], "not", "1st", ok);
   if (ok) storeBoolean(executor, command.args[1], thruthy, "not");
}

// control flow
void builtinGoto(const Command &command, Executor &executor) {
   if (labelOrError(executor, command.args[0], "goto", "1st")) return;
   executor.pointer = executor.code.values[command.args[0].identifier].label - 1;
}

void builtinJmp(const Command &command, Executor &executor) {
   if (labelOrError(executor, command.args[1], "jmp", "2nd")) return;
   bool ok;
   bool thruthy = isThruthy(executor, command.args[0], "jmp", "1st", ok);
   if (ok && thruthy) {
      executor.pointer = executor.code.values[command.args[1].identifier].label - 1;
   }
}

void builtinJmpn(const Command &command, Executor &executor) {
   if (labelOrError(executor, command.args[1], "jmpn", "2nd")) return;
   bool ok;
   bool thruthy = isThruthy(executor, command.args[0], "jmpn", "1st", ok);
   if (ok && !thruthy) {
      executor.pointer = executor.code.values[command.args[1].identifier].label - 1;
   }
}

void builtinCall(const Command &command, Executor &executor) {
   size_t returnCount = 0;
   size_t argCount = 0;
   size_t functionPos = std::string::npos;

   for (size_t i = 0; i < command.args.size(); ++i) {
      size_t identifier = command.args[i].identifier; // access before check
      if (command.args[i].type == VALUE_IDENTIFIER && identifier < executor.code.values.size() && executor.code.values[identifier].init && (executor.code.values[identifier].type == FUNCTION || executor.code.values[identifier].type == NATIVE_FUNCTION)) {
         if (functionPos != std::string::npos) {
            error(executor.diagnostics, "call: Cannot call multiple functions in a single call", command.file, command.line);
            return;
         }
         functionPos = i;
      }
      else {
         returnCount += (functionPos == std::string::npos);
         argCount += (functionPos != std::string::npos);
      }
   }

   if (functionPos == std::string::npos) {
      error(executor.diagnostics, "call: Expected function name to call", command.file, command.line);
      return;
   }
   ParseValue &function = executor.code.values[command.args[functionPos].identifier];
   size_t params = function.params.size();
   bool variadic = function.variadic;

   if ((!variadic && argCount != params) || (variadic && argCount < params)) {
      error(executor.diagnostics, std::format("call: Called function expected {}{} parameters, but received {} arguments", (variadic ? ">" : ""), params, argCount), command.file, command.line);
      return;
   }

   if (function.type == NATIVE_FUNCTION) {
      error(executor.diagnostics, "call: Cannot call native function. Remove excess call", command.file, command.line);
      return;
   }

   Trace trace (executor.pointer, command.lexeme, returnCount);
   trace.locals.resize(function.localCount);
   for (size_t i = functionPos + 1; i < functionPos + 1 + params; ++i) {
      trace.locals[i - functionPos - 1] = resolveVariable(executor, command.args[i], "call");
   }

   executor.stackTrace.push(trace);
   executor.pointer = function.function - 1;
}

void builtinReturn(const Command &command, Executor &executor) {
   if (executor.stackTrace.empty()) {
      executor.exitCalled = true;
      return;
   }
   Trace &trace = executor.stackTrace.top();
   size_t position = trace.position;
   size_t callExpectedReturnCount = trace.callExpectedReturnCount;
   size_t lexeme = trace.lexeme;
   executor.pointer = trace.position;

   executor.returnCount = command.args.size();
   if (executor.returnCount > executor.returnRegisters.size()) {
      error(executor.diagnostics, std::format("return: Can return at maximum {} values. Define 'return-register-count {}' directive to mitigate. Error", executor.returnRegisters.size(), executor.returnCount), command.file, command.line);
      executor.stackTrace.pop();
      return;
   }

   for (size_t i = 0; i < executor.returnCount; ++i) {
      executor.returnRegisters[i] = resolveVariable(executor, command.args[i], "return");
   }
   executor.stackTrace.pop();

   // call shenanigans
   static size_t callLexeme = cacheLexeme(executor.cache, "call");
   if (lexeme == callLexeme) {
      const Command &call = executor.code.code[position];
      if (executor.returnCount != callExpectedReturnCount) {
         warn(executor.diagnostics, std::format("call: Expected {} return values, but got {} instead", callExpectedReturnCount, executor.returnCount), call.file, call.line);
      }

      size_t count = std::min(executor.returnCount, callExpectedReturnCount);
      for (size_t i = 0; i < count; ++i) {
         Value reg = call.args[i];
         if (registerOrError(executor, reg, "call", "return")) return;
         storeInRegister(executor, reg, executor.returnRegisters[i]);
      }
   }
}

// variables. set and move being the same with different order is intentional
void builtinSet(const Command &command, Executor &executor) {
   Value reg = command.args[0];
   if (registerOrError(executor, reg, "set", "1st")) return;
   storeInRegister(executor, reg, resolveVariable(executor, command.args[1], "set"));
}

void builtinMove(const Command &command, Executor &executor) {
   Value reg = command.args[1];
   if (registerOrError(executor, reg, "move", "2nd")) return;
   storeInRegister(executor, reg, resolveVariable(executor, command.args[0], "move"));
}

void builtinGlobal(const Command &command, Executor &executor) {
   Value value {VALUE_COUNT};
   size_t definitionCount = command.args.size();

   if (command.args.size() > 1 && (command.args.back().type != VALUE_IDENTIFIER || (executor.code.values[command.args.back().identifier].init && executor.code.values[command.args.back().identifier].type == GLOBAL))) {
      value = resolveVariable(executor, command.args.back(), "global");
      definitionCount -= 1;
   }
   for (size_t i = 0; i < definitionCount; ++i) {
      if (command.args[i].type != VALUE_IDENTIFIER) {
         error(executor.diagnostics, std::format("global: Expected Identifier, but got {} instead", getValueName(command.args[i].type)), command.file, command.line);
         continue;
      }
      size_t lexeme = command.args[i].identifier;
      if (executor.code.values[lexeme].init && executor.code.values[lexeme].type != GLOBAL) {
         error(executor.diagnostics, std::format("global: Cannot define global '{}' as a {} with the same name already exists", getLexeme(executor.cache, lexeme), getParseValueName(executor.code.values[lexeme].type)), command.file, command.line);
         continue;
      }
      ParseValue global;
      global.type = GLOBAL;
      global.global = value;
      global.init = true;
      executor.code.values[lexeme] = global;
   }
}
