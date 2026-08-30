#include "builtin.hpp"
#include "pil.hpp"
#include <format>

// helper functions
const char *getOrdinalSuffix(size_t n) {
   constexpr const char *ordinalSuffixes[] {"st", "nd", "rd", "th"};
   size_t digit = n % 10;
   return digit > 0 && digit < 4 ? ordinalSuffixes[digit - 1] : ordinalSuffixes[3];
}

Value resolveRegister(Executor &executor, Value value, const char *function) {
   if (value.type != VALUE_REGISTER && value.type != VALUE_RETURN_REGISTER) {
      return value;
   }
   if (value.reg < 0 || value.reg >= executor.registers.size()) {
      error(executor.diagnostics, std::format("{}: Register {}${} is out of bounds", function, value.type == VALUE_RETURN_REGISTER ? "R" : "", value.reg), value.fileLexeme, value.line);
      return value;
   }
   return value.type == VALUE_RETURN_REGISTER ? executor.returnRegisters[value.reg] : executor.registers[value.reg];
}

bool registerOrError(Executor &executor, Value value, const char *function, const char *argument) {
   if (value.type != VALUE_REGISTER && value.type != VALUE_RETURN_REGISTER) {
      error(executor.diagnostics, std::format("{}: Expected Register for the {} argument, got {} instead", function, argument, getValueName(value.type)), value.fileLexeme, value.line);
      return true;
   }
   if (value.reg < 0 || value.reg >= executor.registers.size()) {
      error(executor.diagnostics, std::format("{}: Register {}${} is out of bounds", function, value.type == VALUE_RETURN_REGISTER ? "R" : "", value.reg), value.fileLexeme, value.line);
      return true;
   }
   return false;
}

void storeInRegister(Executor &executor, Value reg, Value value) {
   (reg.type == VALUE_REGISTER ? executor.registers[reg.reg] : executor.returnRegisters[reg.reg]) = value;
}

bool labelOrError(Executor &executor, Value value, const char *function, const char *argument) {
   if (value.type != VALUE_IDENTIFIER || value.identifier >= executor.code.functions.size() || !executor.code.functions[value.identifier].init || executor.code.functions[value.identifier].type != LABEL) {
      error(executor.diagnostics, std::format("{}: Expected Label for the {} argument, got {} instead", function, argument, getValueName(value.type)), value.fileLexeme, value.line);
      return true;
   }
   return false;
}

// misc, temp
void builtinMove(const Command &command, Executor &executor) {
   Value reg = command.args[1];
   if (registerOrError(executor, reg, "move", "2nd")) return;
   storeInRegister(executor, reg, resolveRegister(executor, command.args[0], "move"));
}

void builtinAdd(const Command &command, Executor &executor) {
   Value result;
   result.fileLexeme = command.file;
   result.line = command.line;
   result.type = VALUE_INTEGER;
   result.integer = 0;

   for (size_t i = 0; i < command.args.size() - 1; ++i) {
      Value arg = resolveRegister(executor, command.args[i], "add");
      result.integer += arg.integer;
   }

   if (registerOrError(executor, command.args.back(), "add", "destination")) return;
   storeInRegister(executor, command.args.back(), result);
}

void builtinPrint(const Command &command, Executor &executor) {
   for (size_t i = 0; i < command.args.size(); ++i) {
      Value arg = resolveRegister(executor, command.args[i], "print");
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
   Value a = resolveRegister(executor, lhs, function);
   Value b = resolveRegister(executor, rhs, function);

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
   Value v = resolveRegister(executor, value, function);
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
   executor.registers[reg.reg] = out;
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
   executor.pointer = executor.code.functions[command.args[0].identifier].label - 1;
}

void builtinJmp(const Command &command, Executor &executor) {
   if (labelOrError(executor, command.args[1], "jmp", "2nd")) return;
   bool ok;
   bool thruthy = isThruthy(executor, command.args[0], "jmp", "1st", ok);
   if (ok && thruthy) {
      executor.pointer = executor.code.functions[command.args[1].identifier].label - 1;
   }
}

void builtinJmpn(const Command &command, Executor &executor) {
   if (labelOrError(executor, command.args[1], "jmpn", "2nd")) return;
   bool ok;
   bool thruthy = isThruthy(executor, command.args[0], "jmpn", "1st", ok);
   if (ok && !thruthy) {
      executor.pointer = executor.code.functions[command.args[1].identifier].label - 1;
   }
}

void builtinCall(const Command &command, Executor &executor) {
   size_t returnCount = 0;
   size_t argCount = 0;
   size_t functionPos = std::string::npos;

   for (size_t i = 0; i < command.args.size(); ++i) {
      if (command.args[i].type == VALUE_IDENTIFIER && command.args[i].identifier < executor.code.functions.size() && executor.code.functions[command.args[i].identifier].init && executor.code.functions[command.args[i].identifier].type != LABEL) {
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
   Function &function = executor.code.functions[command.args[functionPos].identifier];
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
   executor.stackTrace.emplace(executor.pointer, command.lexeme, returnCount);
   executor.pointer = function.function - 1;
}

void builtinReturn(const Command &command, Executor &executor) {
   if (executor.stackTrace.empty()) {
      executor.exitCalled = true;
      return;
   }
   Trace trace = executor.stackTrace.top();
   executor.stackTrace.pop();
   executor.pointer = trace.position;

   executor.returnCount = command.args.size();
   if (executor.returnCount > executor.returnRegisters.size()) {
      error(executor.diagnostics, std::format("return: Register R${} is out of bounds; too many return values", executor.returnCount), command.file, command.line);
      return;
   }

   for (size_t i = 0; i < executor.returnCount; ++i) {
      executor.returnRegisters[i] = resolveRegister(executor, command.args[i], "return");
   }

   // call shenanigans
   static size_t callLexeme = cacheLexeme(executor.cache, "call");
   if (trace.lexeme == callLexeme) {
      const Command &call = executor.code.code[trace.position];
      if (executor.returnCount != trace.callExpectedReturnCount) {
         warn(executor.diagnostics, std::format("call: Expected {} return values, but got {} instead", trace.callExpectedReturnCount, executor.returnCount), call.file, call.line);
      }

      size_t count = std::min(executor.returnCount, trace.callExpectedReturnCount);
      for (size_t i = 0; i < count; ++i) {
         Value reg = call.args[i];
         if (registerOrError(executor, reg, "call", std::format("{}{}", i + 1, getOrdinalSuffix(i + 1)).c_str())) return;
         (reg.type == VALUE_REGISTER ? executor.registers[reg.reg] : executor.returnRegisters[reg.reg]) = executor.returnRegisters[i];
      }
   }
}
