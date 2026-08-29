#include "builtin.hpp"
#include "pil.hpp"
#include <format>

// helper functions
Value resolveRegister(Executor &executor, Value value, const char *function) {
   if (value.type != VALUE_REGISTER) return value;
   if (value.reg < 0 || value.reg >= executor.registers.size()) {
      error(executor.diagnostics, std::format("{}: Register ${} is out of bounds", function, value.reg), value.fileLexeme, value.line);
      return value;
   }
   return executor.registers[value.reg];
}

bool registerOrError(Executor &executor, Value value, const char *function, const char *argument) {
   if (value.type != VALUE_REGISTER) {
      error(executor.diagnostics, std::format("{}: Expected Register for the {} argument, got {} instead", function, argument, getValueName(value.type)), value.fileLexeme, value.line);
      return true;
   }
   if (value.reg < 0 || value.reg >= executor.registers.size()) {
      error(executor.diagnostics, std::format("{}: Register ${} is out of bounds", function, value.reg), value.fileLexeme, value.line);
      return true;
   }
   return false;
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
   if (registerOrError(executor, command.args[1], "move", "second")) return;
   executor.registers[command.args[1].reg] = command.args[0];
}

void builtinAdd(const Command &command, Executor &executor) {
   long result = 0;
   for (size_t i = 0; i < command.args.size() - 1; ++i) {
      const Value &arg = command.args[i];
      result += (arg.type == VALUE_REGISTER ? executor.registers[arg.reg].integer : arg.integer);
   }
   executor.registers[command.args.back().reg].integer = result;
}

void builtinSub(const Command &command, Executor &executor) {
   const Value &arg = command.args[0];
   long result = (arg.type == VALUE_REGISTER ? executor.registers[arg.reg].integer : arg.integer);

   for (size_t i = 1; i < command.args.size() - 1; ++i) {
      const Value &arg = command.args[i];
      result -= (arg.type == VALUE_REGISTER ? executor.registers[arg.reg].integer : arg.integer);
   }
   executor.registers[command.args.back().reg].integer = result;
}

void builtinPrint(const Command &command, Executor &executor) {
   for (size_t i = 0; i < command.args.size(); ++i) {
      const Value &arg = command.args[i];
      printf("%ld", (arg.type == VALUE_REGISTER ? executor.registers[arg.reg].integer : arg.integer));
   }
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
   bool thruthy = isThruthy(executor, command.args[0], "not", "first", ok);
   if (ok) storeBoolean(executor, command.args[1], thruthy, "not");
}

// control flow
void builtinGoto(const Command &command, Executor &executor) {
   if (labelOrError(executor, command.args[0], "goto", "first")) return;
   executor.pointer = executor.code.functions[command.args[0].identifier].label - 1;
}

void builtinJmp(const Command &command, Executor &executor) {
   if (labelOrError(executor, command.args[1], "jmp", "second")) return;
   bool ok;
   bool thruthy = isThruthy(executor, command.args[0], "jmp", "first", ok);
   if (ok && thruthy) {
      executor.pointer = executor.code.functions[command.args[1].identifier].label - 1;
   }
}

void builtinJmpn(const Command &command, Executor &executor) {
   if (labelOrError(executor, command.args[1], "jmpn", "second")) return;
   bool ok;
   bool thruthy = isThruthy(executor, command.args[0], "jmpn", "first", ok);
   if (ok && !thruthy) {
      executor.pointer = executor.code.functions[command.args[1].identifier].label - 1;
   }
}

void builtinReturn(const Command &command, Executor &executor) {
   if (executor.stack.empty()) {
      executor.exitCalled = true;
      return;
   }
   size_t back = executor.stack.top();
   executor.stack.pop();
   executor.pointer = back;
}
