#pragma once
#include "pil.hpp"
#include <string>

enum Comparison: char {
   COMPARISON_LESS, COMPARISON_GREATER, COMPARISON_EQUAL, COMPARISON_NOT_EQUAL
};

// helper functions
inline void deallocate(Executor &executor, Value &value) {
   if (value.type == VALUE_STRING) {
      PILString &string = executor.strings[value.string];
      string.allocations -= 1;
      if (string.allocations <= 0) {
         executor.strings.erase(value.string);
         value = Value{VALUE_COUNT};
      }
   }
   else if (value.type == VALUE_ARRAY) {
      PILArray &array = executor.arrays[value.array];
      array.allocations -= 1;
      if (array.allocations <= 0) {
         executor.arrays.erase(value.array);
         value = Value{VALUE_COUNT};
      }
   }
}

inline void copyValue(Executor &executor, Value &target, Value &copy) {
   deallocate(executor, target);
   target = copy;
   if (target.type == VALUE_STRING) {
      executor.strings[target.string].allocations += 1;
   }
   else if (target.type == VALUE_ARRAY) {
      executor.arrays[target.array].allocations += 1;
   }
}

inline void moveValue(Executor &executor, Value &target, Value &move) {
   deallocate(executor, target);
   target = move;
}

inline Value &resolveVariableByRef(Executor &executor, Value &value) {
   if (value.type == VALUE_LOCAL) {
      return executor.locals[executor.stackTrace.top().localStart + value.local];
   }
   else if (value.type == VALUE_REGISTER || value.type == VALUE_RETURN_REGISTER) {
      std::vector<Value> &registers = (value.type == VALUE_RETURN_REGISTER ? executor.returnRegisters : executor.registers);
      return registers[value.reg];
   }
   else {
      return value;
   }
}

inline Value resolveVariable(Executor &executor, Value value) {
   return resolveVariableByRef(executor, value);
}

inline Value arg(const Executor &executor, const Command &command, size_t i) {
   return executor.arguments[command.argStart + i];
}

inline Value back(const Executor &executor, const Command &command) {
   return executor.arguments[command.argStart + command.argCount - 1];
}

inline void storeInRegister(Executor &executor, const Command &command, Value reg, Value value, const char *function) {
   if (reg.type == VALUE_LOCAL) {
      copyValue(executor, executor.locals[executor.stackTrace.top().localStart + reg.local], value);
   }
   else if (reg.type == VALUE_REGISTER || reg.type == VALUE_RETURN_REGISTER) {
      std::vector<Value> &registers = (reg.type == VALUE_RETURN_REGISTER ? executor.returnRegisters : executor.registers);
      copyValue(executor, registers[reg.reg], value);
   }
   else {
      error(executor.diagnostics, command.file, command.line, "%s: Expected Register/Variable for the destination argument, got %s instead", function, getValueName(reg.type));
   }
}

inline void storeInRegister(Executor &executor, const Command &command, Value value, const char *function) {
   storeInRegister(executor, command, back(executor, command), value, function);
}

inline void jumpToLabel(Executor &executor, Value value, const char *function, const char *argument, size_t file, size_t line, bool condition) {
   if (value.type != VALUE_LABEL) {
      error(executor.diagnostics, file, line, "%s: Expected Label for the %s argument, got %s instead", function, argument, getValueName(value.type));
      return;
   }
   if (condition) {
      executor.pointer = executor.functions[value.label].position - 1;
   }
}

inline double getNum(Executor &executor, const Command &command, size_t i, const char *function, bool *floating = nullptr) {
   Value value = resolveVariable(executor, arg(executor, command, i));
   if (value.type != VALUE_INTEGER && value.type != VALUE_FLOATING) {
      error(executor.diagnostics, command.file, command.line, "%s: Expected numeral, got %s instead", function, getValueName(value.type));
      return 0.0;
   }
   if (floating && value.type == VALUE_FLOATING) *floating = true;
   return (value.type == VALUE_INTEGER ? (double)value.integer : value.floating);
}

inline void storeNumber(Executor &executor, const Command &command, double number, bool floating, const char *function) {
   Value value {floating ? VALUE_FLOATING : VALUE_INTEGER};
   if (floating) {
      value.floating = number;
   }
   else {
      value.integer = number;
   }
   storeInRegister(executor, command, value, function);
}

inline void storeBoolean(Executor &executor, const Command &command, bool result, const char *function) {
   Value value {VALUE_INTEGER};
   value.integer = (result ? 1 : 0);
   storeInRegister(executor, command, value, function);
}

inline void unaryBuiltin(Executor &executor, const Command &command, double(*fn)(double), const char *function) {
   storeNumber(executor, command, fn(getNum(executor, command, 0, function)), true, function);
}

inline void binaryBuiltin(Executor &executor, const Command &command, double(*fn)(double, double), const char *function) {
   storeNumber(executor, command, fn(getNum(executor, command, 0, function), getNum(executor, command, 1, function)), true, function);
}

inline std::string toString(Executor &executor, Value value, const char *function, size_t file, size_t line) {
   value = resolveVariable(executor, value);
   switch (value.type) {
   case VALUE_INTEGER: return std::to_string(value.integer);
   case VALUE_FLOATING: return std::to_string(value.floating);
   case VALUE_CHARACTER: return std::string(1, value.character);
   case VALUE_CSTRING: return getLexeme(executor.cache, value.string);
   case VALUE_STRING: return getString(executor, value.string, file, line);
   case VALUE_FUNCTION: return getLexeme(executor.cache, executor.functions[value.function].lexeme) + "()";
   case VALUE_LABEL: return getLexeme(executor.cache, executor.functions[value.label].lexeme) + ":";
   case VALUE_ARRAY: {
      std::string result;
      std::vector<Value> &array = getArray(executor, value.array, file, line);
      result.reserve(3 + 4 * array.size());
      result += "[ ";
      for (Value &arv: array) {
         result += toString(executor, arv, function, file, line) + ", ";
      }
      result += "]";
   }
   default: return "(null)";
   }
}

inline std::string format(const Command &command, Executor &executor, const char *function, size_t offset) {
   Value string = arg(executor, command, 0);
   if (string.type != VALUE_CSTRING && string.type != VALUE_STRING) {
      error(executor.diagnostics, command.file, command.line, "%s: Expected String for the 1st argument, got %s instead", function, getValueName(string.type));
      return "";
   }
   std::string result = string.type == VALUE_CSTRING ? getLexeme(executor.cache, string.string) : getString(executor, string.string, command.file, command.line);
   size_t pos = 0;

   for (size_t i = 1; i < command.argCount - offset; ++i) {
      pos = result.find("{}", pos);
      result = (pos != std::string::npos ? result.replace(pos, 2, toString(executor, arg(executor, command, i), function, command.file, command.line)) : result);
   }
   return result;
}

inline void printValue(Executor &executor, Value a, size_t file, size_t line) {
   switch (a.type) {
   case VALUE_INTEGER: printf("%ld", a.integer); break;
   case VALUE_FLOATING: printf("%.3F", a.floating); break;
   case VALUE_CHARACTER: printf("%c", a.character); break;
   case VALUE_CSTRING: printf("%s", getLexeme(executor.cache, a.string).c_str()); break;
   case VALUE_STRING: printf("%s", getString(executor, a.string, file, line).c_str()); break;
   case VALUE_FUNCTION: printf("%s()", getLexeme(executor.cache, executor.functions[a.function].lexeme).c_str()); break;
   case VALUE_LABEL: printf("%s:", getLexeme(executor.cache, executor.functions[a.label].lexeme).c_str()); break;
   case VALUE_ARRAY: {
      printf("[ ");
      std::vector<Value> &array = getArray(executor, a.array, file, line);
      for (Value &arv: array) {
         printValue(executor, arv, file, line);
         printf(", ");
      }
      putchar(']');
      break;
   }
   default: printf("(null)");
   }
}

inline void print(const Command &command, Executor &executor, const char *function, size_t file, size_t line) {
   for (size_t i = 0; i < command.argCount; ++i) {
      Value a = resolveVariable(executor, arg(executor, command, i));
      printValue(executor, a, file, line);
   }
}

inline void comparisonBuiltin(Executor &executor, const Command &command, const char *function, Comparison expected, bool reverse, bool softie) {
   Comparison result;
   Value a = resolveVariable(executor, arg(executor, command, 0));
   Value b = resolveVariable(executor, arg(executor, command, 1));

   if ((a.type == VALUE_INTEGER || a.type == VALUE_FLOATING) && (b.type == VALUE_INTEGER || b.type == VALUE_FLOATING)) {
      double x = (a.type == VALUE_INTEGER) ? (double)a.integer : a.floating;
      double y = (b.type == VALUE_INTEGER) ? (double)b.integer : b.floating;
      result = (x < y ? COMPARISON_LESS : x > y ? COMPARISON_GREATER : COMPARISON_EQUAL);
   }
   else if (a.type == VALUE_CHARACTER && b.type == VALUE_CHARACTER) {
      result = (a.character < b.character ? COMPARISON_LESS : a.character > b.character ? COMPARISON_GREATER : COMPARISON_EQUAL);
   }
   else if ((a.type == VALUE_STRING || a.type == VALUE_CSTRING) && (b.type == VALUE_STRING || b.type == VALUE_CSTRING)) {
      const std::string &as = (a.type == VALUE_STRING ? getString(executor, a.string, command.file, command.line) : getLexeme(executor.cache, a.string));
      const std::string &bs = (b.type == VALUE_STRING ? getString(executor, b.string, command.file, command.line) : getLexeme(executor.cache, b.string));
      int c = as.compare(bs);
      result = (c < 0 ? COMPARISON_LESS : c > 0 ? COMPARISON_GREATER : COMPARISON_EQUAL);
   }
   else if (!softie) {
      error(executor.diagnostics, command.file, command.line, "%s: Cannot compare %s to %s", function, getValueName(a.type), getValueName(b.type));
      return;
   }
   else {
      result = COMPARISON_NOT_EQUAL;
   }
   storeBoolean(executor, command, (result == expected) != reverse, function);
}

inline bool valuesEqual(Executor &executor, const Command &command, Value a, Value b) {
   if ((a.type == VALUE_INTEGER || a.type == VALUE_FLOATING) && (b.type == VALUE_INTEGER || b.type == VALUE_INTEGER)) {
      double x = (a.type == VALUE_INTEGER) ? (double)a.integer : a.floating;
      double y = (b.type == VALUE_INTEGER) ? (double)b.integer : b.floating;
      return x == y;
   }
   else if (a.type == VALUE_CHARACTER && b.type == VALUE_CHARACTER) {
      return a.character == b.character;
   }
   else if ((a.type == VALUE_STRING || a.type == VALUE_CSTRING) && (b.type == VALUE_STRING || b.type == VALUE_CSTRING)) {
      const std::string &as = (a.type == VALUE_STRING ? getString(executor, a.string, command.file, command.line) : getLexeme(executor.cache, a.string));
      const std::string &bs = (b.type == VALUE_STRING ? getString(executor, b.string, command.file, command.line) : getLexeme(executor.cache, b.string));
      return as == bs;
   }
   return false;
}

inline bool getBool(Executor &executor, const Command &command, size_t i) {
   Value v = resolveVariable(executor, arg(executor, command, i));
   switch (v.type) {
   case VALUE_INTEGER: return v.integer != 0;
   case VALUE_FLOATING: return v.floating != 0.0;
   case VALUE_CHARACTER: return v.character != 0;
   case VALUE_CSTRING: return !getLexeme(executor.cache, v.string).empty();
   case VALUE_STRING: return !getString(executor, v.string, command.file, command.line).empty();
   case VALUE_ARRAY: return !getArray(executor, v.array, command.file, command.line).empty();
   case VALUE_FUNCTION: return true;
   case VALUE_LABEL: return true;
   case VALUE_COUNT: return false;
   default: // should not happen
      printf("PIL::isThruthy: Value %s cannot be checked for thruthiness.\n", getValueName(v.type));
      exit(EXIT_FAILURE);
   }
}

inline void storeString(Executor &executor, const Command &command, const std::string &string, const char *function) {
   Value value {VALUE_STRING};
   value.string = allocateString(executor, string);
   storeInRegister(executor, command, value, function);
}

inline void storeArray(Executor &executor, const Command &command, const std::vector<Value> &array, const char *function) {
   Value value {VALUE_ARRAY};
   value.array = allocateArray(executor, array);
   storeInRegister(executor, command, value, function);
}

void setEcho(bool on);
