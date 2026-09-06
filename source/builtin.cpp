#include "builtin.hpp"
#include "pil.hpp"
#include <algorithm>
#include <cmath>

// helper functions
void deallocate(Executor &executor, Value &value) {
   if (value.type == VALUE_STRING) {
      value.allocations -= 1;
      if (value.allocations <= 0) {
         executor.strings.erase(value.string);
         value = Value{VALUE_COUNT};
      }
   }
}

void copyValue(Executor &executor, Value &target, Value &copy) {
   deallocate(executor, target);
   target = copy;
   target.allocations += 1;
}

void moveValue(Executor &executor, Value &target, Value &move) {
   deallocate(executor, target);
   target = move;
}

Value resolveVariable(Executor &executor, Value value, const char *function, size_t file, size_t line) {
   if (value.type == VALUE_LOCAL) {
      return executor.locals[executor.stackTrace.top().localStart + value.local];
   }

   if (value.type == VALUE_IDENTIFIER) {
      if (!executor.values[value.identifier].init || executor.values[value.identifier].type != GLOBAL) {
         error(executor.diagnostics, file, line, "%s: Variable '%s' does not exist", function, getLexeme(executor.cache, value.identifier).c_str());
         return value;
      }
      return executor.values[value.identifier].global;
   }

   if (value.type != VALUE_REGISTER && value.type != VALUE_RETURN_REGISTER) {
      return value;
   }
   std::vector<Value> &registers = (value.type == VALUE_RETURN_REGISTER ? executor.returnRegisters : executor.registers);

   if (value.reg < 0 || value.reg >= registers.size()) {
      error(executor.diagnostics, file, line, "%s: Register %s$%zu is out of bounds", function, value.type == VALUE_RETURN_REGISTER ? "R" : "", value.reg);
      return value;
   }
   return registers[value.reg];
}

Value arg(const Executor &executor, const Command &command, size_t i) {
   return executor.arguments[command.argStart + i];
}

bool registerOrError(Executor &executor, Value value, const char *function, const char *argument, size_t file, size_t line) {
   if (value.type == VALUE_LOCAL) {
      return false;
   }

   if (value.type == VALUE_IDENTIFIER) {
      if (!executor.values[value.identifier].init || executor.values[value.identifier].type != GLOBAL) {
         error(executor.diagnostics, file, line, "%s: Expected Register/Variable for the %s argument, got %s instead", function, argument, getParseValueName(executor.values[value.identifier].type));
         return true;
      }
      return false;
   }

   if (value.type != VALUE_REGISTER && value.type != VALUE_RETURN_REGISTER) {
      error(executor.diagnostics, file, line, "%s: Expected Register/Variable for the %s argument, got %s instead", function, argument, getValueName(value.type));
      return true;
   }
   std::vector<Value> &registers = (value.type == VALUE_RETURN_REGISTER ? executor.returnRegisters : executor.registers);

   if (value.reg < 0 || value.reg >= registers.size()) {
      error(executor.diagnostics, file, line, "%s: Register %s$%zu is out of bounds", function, value.type == VALUE_RETURN_REGISTER ? "R" : "", value.reg);
      return true;
   }
   return false;
}

void storeInRegister(Executor &executor, Value reg, Value value) {
   switch (reg.type) {
   case VALUE_LOCAL: copyValue(executor, executor.locals[executor.stackTrace.top().localStart + reg.local], value); break;
   case VALUE_IDENTIFIER: copyValue(executor, executor.values[reg.identifier].global, value); break;
   case VALUE_REGISTER: copyValue(executor, executor.registers[reg.reg], value); break;
   case VALUE_RETURN_REGISTER: copyValue(executor, executor.returnRegisters[reg.reg], value); break;
   default:
      printf("PIL::storeInRegister: Cannot store into %s.\n", getValueName(value.type));
      exit(EXIT_FAILURE);
   }
}

bool labelOrError(Executor &executor, Value value, const char *function, const char *argument, size_t file, size_t line) {
   if (value.type != VALUE_IDENTIFIER || value.identifier >= executor.values.size() || !executor.values[value.identifier].init || executor.values[value.identifier].type != LABEL) {
      error(executor.diagnostics, file, line, "%s: Expected Label for the %s argument, got %s instead", function, argument, getValueName(value.type));
      return true;
   }
   return false;
}

// output
std::string toString(Executor &executor, Value value, const char *function, size_t file, size_t line) {
   value = resolveVariable(executor, value, function, file, line);
   switch (value.type) {
   case VALUE_INTEGER: return std::to_string(value.integer);
   case VALUE_FLOATING: return std::to_string(value.floating);
   case VALUE_CHARACTER: return std::string(1, value.character);
   case VALUE_CSTRING: return getLexeme(executor.cache, value.string);
   case VALUE_STRING: return getString(executor, value.string, file, line);
   default: return "(null)";
   }
}

std::string format(const Command &command, Executor &executor, const char *function, size_t offset) {
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

void print(const Command &command, Executor &executor, const char *function, size_t file, size_t line) {
   for (size_t i = 0; i < command.argCount; ++i) {
      Value a = resolveVariable(executor, arg(executor, command, i), function, file, line);
      switch (a.type) {
      case VALUE_INTEGER: printf("%ld", a.integer); break;
      case VALUE_FLOATING: printf("%.3F", a.floating); break;
      case VALUE_CHARACTER: printf("%c", a.character); break;
      case VALUE_CSTRING: printf("%s", getLexeme(executor.cache, a.string).c_str()); break;
      case VALUE_STRING: printf("%s", getString(executor, a.string, command.file, command.line).c_str()); break;
      default: printf("(null)");
      }
   }
}

void builtinPrint(const Command &command, Executor &executor) {
   print(command, executor, "print", command.file, command.line);
}

void builtinPrintn(const Command &command, Executor &executor) {
   print(command, executor, "printn", command.file, command.line);
   putchar('\n');
}

void builtinPrintf(const Command &command, Executor &executor) {
   printf("%s", format(command, executor, "printf", 0).c_str());
}

void builtinPrintfn(const Command &command, Executor &executor) {
   printf("%s\n", format(command, executor, "printfn", 0).c_str());
}

// string
void builtinStringNew(const Command &command, Executor &executor) {
   std::string result;
   for (size_t i = 0; i < command.argCount - 1; ++i) {
      result += toString(executor, arg(executor, command, i), "string-new", command.file, command.line);
   }
   Value value {VALUE_STRING, 0};
   value.string = allocateString(executor, result);
   if (registerOrError(executor, arg(executor, command, command.argCount-1), "string-new", "destination", command.file, command.line)) return;
   storeInRegister(executor, arg(executor, command, command.argCount-1), value);
}

void builtinFormat(const Command &command, Executor &executor) {
   Value value {VALUE_STRING, 0};
   value.string = allocateString(executor, format(command, executor, "format", 1));
   if (registerOrError(executor, arg(executor, command, command.argCount-1), "format", "destination", command.file, command.line)) return;
   storeInRegister(executor, arg(executor, command, command.argCount-1), value);
}

// math
double getNum(Executor &executor, Value value, const char *function, size_t file, size_t line, bool *floating = nullptr) {
   value = resolveVariable(executor, value, function, file, line);
   if (value.type != VALUE_INTEGER && value.type != VALUE_FLOATING) {
      error(executor.diagnostics, file, line, "%s: Expected numeral, got %s instead", function, getValueName(value.type));
      return 0.0;
   }
   if (floating && value.type == VALUE_FLOATING) *floating = true;
   return (value.type == VALUE_INTEGER ? (double)value.integer : value.floating);
}

void storeNumber(Executor &executor, Value reg, double number, bool floating) {
   Value value {floating ? VALUE_FLOATING : VALUE_INTEGER};
   if (floating) {
      value.floating = number;
   }
   else {
      value.integer = number;
   }
   storeInRegister(executor, reg, value);
}

void builtinAdd(const Command &command, Executor &executor) {
   bool floating = false;
   double number = 0.0;
   for (size_t i = 0; i < command.argCount - 1; ++i) {
      number += getNum(executor, arg(executor, command, i), "add", command.file, command.line, &floating);
   }
   if (registerOrError(executor, arg(executor, command, command.argCount-1), "add", "destination", command.file, command.line)) return;
   storeNumber(executor, arg(executor, command, command.argCount-1), number, floating);
}

void builtinSub(const Command &command, Executor &executor) {
   bool floating = false;
   double number = getNum(executor, arg(executor, command, 0), "sub", command.file, command.line, &floating);
   for (size_t i = 1; i < command.argCount - 1; ++i) {
      number -= getNum(executor, arg(executor, command, i), "sub", command.file, command.line, &floating);
   }
   if (registerOrError(executor, arg(executor, command, command.argCount-1), "sub", "destination", command.file, command.line)) return;
   storeNumber(executor, arg(executor, command, command.argCount-1), number, floating);
}

void builtinMul(const Command &command, Executor &executor) {
   bool floating = false;
   double number = 1.0;
   for (size_t i = 0; i < command.argCount - 1; ++i) {
      number *= getNum(executor, arg(executor, command, i), "mul", command.file, command.line, &floating);
   }
   if (registerOrError(executor, arg(executor, command, command.argCount-1), "mul", "destination", command.file, command.line)) return;
   storeNumber(executor, arg(executor, command, command.argCount-1), number, floating);
}

void builtinDiv(const Command &command, Executor &executor) {
   bool floating = false;
   double number = getNum(executor, arg(executor, command, 0), "div", command.file, command.line, &floating);
   for (size_t i = 1; i < command.argCount - 1; ++i) {
      double num = getNum(executor, arg(executor, command, i), "div", command.file, command.line, &floating);
      number = (num == 0.0 ? 0.0 : number / num); // defined behavior
   }
   if (registerOrError(executor, arg(executor, command, command.argCount-1), "div", "destination", command.file, command.line)) return;
   storeNumber(executor, arg(executor, command, command.argCount-1), number, floating);
}

void builtinMod(const Command &command, Executor &executor) {
   bool floating = false;
   double a = getNum(executor, arg(executor, command, 0), "mod", command.file, command.line, &floating);
   double b = getNum(executor, arg(executor, command, 1), "mod", command.file, command.line, &floating);
   if (registerOrError(executor, arg(executor, command, 2), "mod", "destination", command.file, command.line)) return;
   storeNumber(executor, arg(executor, command, 2), fmod(a, b), floating);
}

void builtinPow(const Command &command, Executor &executor) {
   bool floating = false;
   double a = getNum(executor, arg(executor, command, 0), "pow", command.file, command.line, &floating);
   double b = getNum(executor, arg(executor, command, 1), "pow", command.file, command.line, &floating);
   if (registerOrError(executor, arg(executor, command, 2), "pow", "destination", command.file, command.line)) return;
   storeNumber(executor, arg(executor, command, 2), pow(a, b), floating);
}

void builtinNeg(const Command &command, Executor &executor) {
   bool floating = false;
   if (registerOrError(executor, arg(executor, command, 1), "neg", "destination", command.file, command.line)) return;
   storeNumber(executor, arg(executor, command, 1), -getNum(executor, arg(executor, command, 0), "neg", command.file, command.line, &floating), floating);
}

void builtinSqrt(const Command &command, Executor &executor) {
   if (registerOrError(executor, arg(executor, command, 1), "sqrt", "destination", command.file, command.line)) return;
   storeNumber(executor, arg(executor, command, 1), sqrt(getNum(executor, arg(executor, command, 0), "sqrt", command.file, command.line)), true);
}

void builtinCbrt(const Command &command, Executor &executor) {
   if (registerOrError(executor, arg(executor, command, 1), "cbrt", "destination", command.file, command.line)) return;
   storeNumber(executor, arg(executor, command, 1), cbrt(getNum(executor, arg(executor, command, 0), "cbrt", command.file, command.line)), true);
}

void builtinSin(const Command &command, Executor &executor) {
   if (registerOrError(executor, arg(executor, command, 1), "sin", "destination", command.file, command.line)) return;
   storeNumber(executor, arg(executor, command, 1), sin(getNum(executor, arg(executor, command, 0), "sin", command.file, command.line)), true);
}

void builtinCos(const Command &command, Executor &executor) {
   if (registerOrError(executor, arg(executor, command, 1), "cos", "destination", command.file, command.line)) return;
   storeNumber(executor, arg(executor, command, 1), cos(getNum(executor, arg(executor, command, 0), "cos", command.file, command.line)), true);
}

void builtinTan(const Command &command, Executor &executor) {
   if (registerOrError(executor, arg(executor, command, 1), "tan", "destination", command.file, command.line)) return;
   storeNumber(executor, arg(executor, command, 1), tan(getNum(executor, arg(executor, command, 0), "tan", command.file, command.line)), true);
}

void builtinAsin(const Command &command, Executor &executor) {
   if (registerOrError(executor, arg(executor, command, 1), "asin", "destination", command.file, command.line)) return;
   storeNumber(executor, arg(executor, command, 1), asin(getNum(executor, arg(executor, command, 0), "asin", command.file, command.line)), true);
}

void builtinAcos(const Command &command, Executor &executor) {
   if (registerOrError(executor, arg(executor, command, 1), "acos", "destination", command.file, command.line)) return;
   storeNumber(executor, arg(executor, command, 1), acos(getNum(executor, arg(executor, command, 0), "acos", command.file, command.line)), true);
}

void builtinAtan(const Command &command, Executor &executor) {
   if (registerOrError(executor, arg(executor, command, 1), "atan", "destination", command.file, command.line)) return;
   storeNumber(executor, arg(executor, command, 1), atan(getNum(executor, arg(executor, command, 0), "atan", command.file, command.line)), true);
}

void builtinAtan2(const Command &command, Executor &executor) {
   if (registerOrError(executor, arg(executor, command, 2), "atan2", "destination", command.file, command.line)) return;
   storeNumber(executor, arg(executor, command, 2), atan2(getNum(executor, arg(executor, command, 0), "atan2", command.file, command.line), getNum(executor, arg(executor, command, 1), "atan2", command.file, command.line)), true);
}

void builtinAsinh(const Command &command, Executor &executor) {
   if (registerOrError(executor, arg(executor, command, 1), "asinh", "destination", command.file, command.line)) return;
   storeNumber(executor, arg(executor, command, 1), asinh(getNum(executor, arg(executor, command, 0), "asinh", command.file, command.line)), true);
}

void builtinAcosh(const Command &command, Executor &executor) {
   if (registerOrError(executor, arg(executor, command, 1), "acosh", "destination", command.file, command.line)) return;
   storeNumber(executor, arg(executor, command, 1), acosh(getNum(executor, arg(executor, command, 0), "acosh", command.file, command.line)), true);
}

void builtinAtanh(const Command &command, Executor &executor) {
   if (registerOrError(executor, arg(executor, command, 1), "atanh", "destination", command.file, command.line)) return;
   storeNumber(executor, arg(executor, command, 1), atanh(getNum(executor, arg(executor, command, 0), "atanh", command.file, command.line)), true);
}

void builtinSinh(const Command &command, Executor &executor) {
   if (registerOrError(executor, arg(executor, command, 1), "sinh", "destination", command.file, command.line)) return;
   storeNumber(executor, arg(executor, command, 1), sinh(getNum(executor, arg(executor, command, 0), "sinh", command.file, command.line)), true);
}

void builtinCosh(const Command &command, Executor &executor) {
   if (registerOrError(executor, arg(executor, command, 1), "cosh", "destination", command.file, command.line)) return;
   storeNumber(executor, arg(executor, command, 1), cosh(getNum(executor, arg(executor, command, 0), "cosh", command.file, command.line)), true);
}

void builtinTanh(const Command &command, Executor &executor) {
   if (registerOrError(executor, arg(executor, command, 1), "tanh", "destination", command.file, command.line)) return;
   storeNumber(executor, arg(executor, command, 1), tanh(getNum(executor, arg(executor, command, 0), "tanh", command.file, command.line)), true);
}

void builtinAbs(const Command &command, Executor &executor) {
   if (registerOrError(executor, arg(executor, command, 1), "abs", "destination", command.file, command.line)) return;
   storeNumber(executor, arg(executor, command, 1), fabs(getNum(executor, arg(executor, command, 0), "abs", command.file, command.line)), true);
}

void builtinMin(const Command &command, Executor &executor) {
   bool floating = false;
   double number = std::numeric_limits<double>::max();
   for (size_t i = 0; i < command.argCount - 1; ++i) {
      number = std::min(number, getNum(executor, arg(executor, command, i), "min", command.file, command.line, &floating));
   }
   if (registerOrError(executor, arg(executor, command, command.argCount-1), "min", "destination", command.file, command.line)) return;
   storeNumber(executor, arg(executor, command, command.argCount-1), number, floating);
}

void builtinMax(const Command &command, Executor &executor) {
   bool floating = false;
   double number = std::numeric_limits<double>::min();
   for (size_t i = 0; i < command.argCount - 1; ++i) {
      number = std::max(number, getNum(executor, arg(executor, command, i), "max", command.file, command.line, &floating));
   }
   if (registerOrError(executor, arg(executor, command, command.argCount-1), "max", "destination", command.file, command.line)) return;
   storeNumber(executor, arg(executor, command, command.argCount-1), number, floating);
}

void builtinClamp(const Command &command, Executor &executor) {
   bool floating = false;
   double x = getNum(executor, arg(executor, command, 0), "clamp", command.file, command.line, &floating);
   double lo = getNum(executor, arg(executor, command, 1), "clamp", command.file, command.line, &floating);
   double hi = getNum(executor, arg(executor, command, 2), "clamp", command.file, command.line, &floating);
   if (registerOrError(executor, arg(executor, command, 3), "clamp", "destination", command.file, command.line)) return;
   storeNumber(executor, arg(executor, command, 3), std::clamp(x, lo, hi), floating);
}

void builtinCeil(const Command &command, Executor &executor) {
   if (registerOrError(executor, arg(executor, command, 1), "ceil", "destination", command.file, command.line)) return;
   storeNumber(executor, arg(executor, command, 1), ceil(getNum(executor, arg(executor, command, 0), "ceil", command.file, command.line)), true);
}

void builtinFloor(const Command &command, Executor &executor) {
   if (registerOrError(executor, arg(executor, command, 1), "floor", "destination", command.file, command.line)) return;
   storeNumber(executor, arg(executor, command, 1), floor(getNum(executor, arg(executor, command, 0), "floor", command.file, command.line)), true);
}

void builtinRound(const Command &command, Executor &executor) {
   if (registerOrError(executor, arg(executor, command, 1), "round", "destination", command.file, command.line)) return;
   storeNumber(executor, arg(executor, command, 1), round(getNum(executor, arg(executor, command, 0), "round", command.file, command.line)), true);
}

void builtinExp(const Command &command, Executor &executor) {
   if (registerOrError(executor, arg(executor, command, 1), "exp", "destination", command.file, command.line)) return;
   storeNumber(executor, arg(executor, command, 1), exp(getNum(executor, arg(executor, command, 0), "exp", command.file, command.line)), true);
}

void builtinLn(const Command &command, Executor &executor) {
   if (registerOrError(executor, arg(executor, command, 1), "ln", "destination", command.file, command.line)) return;
   storeNumber(executor, arg(executor, command, 1), log(getNum(executor, arg(executor, command, 0), "ln", command.file, command.line)), true);
}

void builtinLog(const Command &command, Executor &executor) {
   if (registerOrError(executor, arg(executor, command, 2), "log", "destination", command.file, command.line)) return;
   storeNumber(executor, arg(executor, command, 2), log(getNum(executor, arg(executor, command, 0), "log", command.file, command.line)) / log(getNum(executor, arg(executor, command, 1), "log", command.file, command.line)), true);
}

void builtinLog2(const Command &command, Executor &executor) {
   if (registerOrError(executor, arg(executor, command, 1), "log2", "destination", command.file, command.line)) return;
   storeNumber(executor, arg(executor, command, 1), log2(getNum(executor, arg(executor, command, 0), "log2", command.file, command.line)), true);
}

void builtinLog10(const Command &command, Executor &executor) {
   if (registerOrError(executor, arg(executor, command, 1), "log10", "destination", command.file, command.line)) return;
   storeNumber(executor, arg(executor, command, 1), log10(getNum(executor, arg(executor, command, 0), "log10", command.file, command.line)), true);
}

// comparison
enum Comparison: char {
   COMPARISON_LESS, COMPARISON_GREATER, COMPARISON_EQUAL, COMPARISON_ERROR, COMPARISON_NOT_EQUAL
};

Comparison compareValues(Executor &executor, Value lhs, Value rhs, const char *function, bool softie, size_t file, size_t line) {
   Value a = resolveVariable(executor, lhs, function, file, line);
   Value b = resolveVariable(executor, rhs, function, file, line);

   if ((a.type == VALUE_INTEGER || a.type == VALUE_FLOATING) && (b.type == VALUE_INTEGER || b.type == VALUE_FLOATING)) {
      double x = (a.type == VALUE_INTEGER) ? (double)a.integer : a.floating;
      double y = (b.type == VALUE_INTEGER) ? (double)b.integer : b.floating;
      return x < y ? COMPARISON_LESS : x > y ? COMPARISON_GREATER : COMPARISON_EQUAL;
   }
   else if (a.type == VALUE_CHARACTER && b.type == VALUE_CHARACTER) {
      return a.character < b.character ? COMPARISON_LESS : a.character > b.character ? COMPARISON_GREATER : COMPARISON_EQUAL;
   }
   else if ((a.type == VALUE_STRING || a.type == VALUE_CSTRING) && (b.type == VALUE_STRING || b.type == VALUE_CSTRING)) {
      const std::string &as = (a.type == VALUE_STRING ? getString(executor, a.string, file, line) : getLexeme(executor.cache, a.string));
      const std::string &bs = (b.type == VALUE_STRING ? getString(executor, b.string, file, line) : getLexeme(executor.cache, b.string));
      int c = as.compare(bs);
      return c < 0 ? COMPARISON_LESS : c > 0 ? COMPARISON_GREATER : COMPARISON_EQUAL;
   }

   if (!softie) {
      error(executor.diagnostics, file, line, "%s: Cannot compare %s and %s", function, getValueName(a.type), getValueName(b.type));
      return COMPARISON_ERROR;
   }
   return COMPARISON_NOT_EQUAL;
}

bool isThruthy(Executor &executor, Value value, const char *function, const char *argument, bool &ok, size_t file, size_t line) {
   Value v = resolveVariable(executor, value, function, file, line);
   ok = true;

   switch (v.type) {
   case VALUE_INTEGER: return v.integer != 0;
   case VALUE_FLOATING: return v.floating != 0.0;
   case VALUE_CHARACTER: return v.character != 0;
   case VALUE_CSTRING: return !getLexeme(executor.cache, v.string).empty();
   case VALUE_STRING: return !getString(executor, v.string, file, line).empty();
   default:
      error(executor.diagnostics, file, line, "%s: Expected value for the %s argument, got %s", function, argument, getValueName(v.type));
      ok = false;
      return false;
   }
}

void storeBoolean(Executor &executor, Value reg, bool result, const char *function, size_t file, size_t line) {
   if (registerOrError(executor, reg, function, "destination", file, line)) return;
   Value out = reg;
   out.type = VALUE_INTEGER;
   out.integer = (result ? 1 : 0);
   storeInRegister(executor, reg, out);
}

void builtinLe(const Command &command, Executor &executor) {
   Comparison result = compareValues(executor, arg(executor, command, 0), arg(executor, command, 1), "le", false, command.file, command.line);
   if (result != COMPARISON_ERROR) storeBoolean(executor, arg(executor, command, 2), result == COMPARISON_LESS, "le", command.file, command.line);
}

void builtinGr(const Command &command, Executor &executor) {
   Comparison result = compareValues(executor, arg(executor, command, 0), arg(executor, command, 1), "gr", false, command.file, command.line);
   if (result != COMPARISON_ERROR) storeBoolean(executor, arg(executor, command, 2), result == COMPARISON_GREATER, "gr", command.file, command.line);
}

void builtinLeeq(const Command &command, Executor &executor) {
   Comparison result = compareValues(executor, arg(executor, command, 0), arg(executor, command, 1), "leeq", false, command.file, command.line);
   if (result != COMPARISON_ERROR) storeBoolean(executor, arg(executor, command, 2), result != COMPARISON_GREATER, "leeq", command.file, command.line);
}

void builtinGreq(const Command &command, Executor &executor) {
   Comparison result = compareValues(executor, arg(executor, command, 0), arg(executor, command, 1), "greq", false, command.file, command.line);
   if (result != COMPARISON_ERROR) storeBoolean(executor, arg(executor, command, 2), result != COMPARISON_LESS, "greq", command.file, command.line);
}

void builtinEq(const Command &command, Executor &executor) {
   Comparison result = compareValues(executor, arg(executor, command, 0), arg(executor, command, 1), "eq", true, command.file, command.line);
   if (result != COMPARISON_ERROR) storeBoolean(executor, arg(executor, command, 2), result == COMPARISON_EQUAL, "eq", command.file, command.line);
}

void builtinNeq(const Command &command, Executor &executor) {
   Comparison result = compareValues(executor, arg(executor, command, 0), arg(executor, command, 1), "neq", true, command.file, command.line);
   if (result != COMPARISON_ERROR) storeBoolean(executor, arg(executor, command, 2), result != COMPARISON_EQUAL, "neq", command.file, command.line);
}

void builtinNot(const Command &command, Executor &executor) {
   bool ok;
   bool thruthy = isThruthy(executor, arg(executor, command, 0), "not", "1st", ok, command.file, command.line);
   if (ok) storeBoolean(executor, arg(executor, command, 1), thruthy, "not", command.file, command.line);
}

// control flow
void builtinGoto(const Command &command, Executor &executor) {
   if (labelOrError(executor, arg(executor, command, 0), "goto", "1st", command.file, command.line)) return;
   executor.pointer = executor.values[arg(executor, command, 0).identifier].label - 1;
}

void builtinJmp(const Command &command, Executor &executor) {
   if (labelOrError(executor, arg(executor, command, 1), "jmp", "2nd", command.file, command.line)) return;
   bool ok;
   bool thruthy = isThruthy(executor, arg(executor, command, 0), "jmp", "1st", ok, command.file, command.line);
   if (ok && thruthy) {
      executor.pointer = executor.values[arg(executor, command, 1).identifier].label - 1;
   }
}

void builtinJmpn(const Command &command, Executor &executor) {
   if (labelOrError(executor, arg(executor, command, 1), "jmpn", "2nd", command.file, command.line)) return;
   bool ok;
   bool thruthy = isThruthy(executor, arg(executor, command, 0), "jmpn", "1st", ok, command.file, command.line);
   if (ok && !thruthy) {
      executor.pointer = executor.values[arg(executor, command, 1).identifier].label - 1;
   }
}

void builtinCall(const Command &command, Executor &executor) {
   ParseValue &function = executor.values[arg(executor, command, command.callee).identifier];
   call(executor, command, function, command.callee, command.callee, command.argCount - command.callee - 1);
}

void builtinReturn(const Command &command, Executor &executor) {
   if (executor.stackTrace.empty()) {
      executor.exitCalled = true;
      return;
   }
   Trace &trace = executor.stackTrace.top();
   size_t callArgStart = trace.callArgStart;
   size_t callArgCount = trace.callArgCount;
   size_t localStart = trace.localStart;
   executor.pointer = trace.position;
   executor.returnCount = command.argCount;

   if (executor.returnCount > executor.returnRegisters.size()) {
      error(executor.diagnostics, command.file, command.line, "return: Can return at maximum %zu values. Define 'return-register-count %zu' directive to mitigate. Error", executor.returnRegisters.size(), executor.returnCount);
      executor.stackTrace.pop();
      return;
   }

   for (size_t i = 0; i < executor.returnCount; ++i) {
      Value value = resolveVariable(executor, arg(executor, command, i), "return", command.file, command.line);
      moveValue(executor, executor.returnRegisters[i], value);
   }
   executor.locals.resize(localStart);
   executor.stackTrace.pop();

   // call shenanigans
   if (callArgCount != std::string::npos) {
      if (executor.returnCount != callArgCount) {
         warn(executor.diagnostics, command.file, command.line, "call: Expected %zu return values, but got %zu instead", callArgCount, executor.returnCount);
      }

      size_t count = std::min(executor.returnCount, callArgCount);
      for (size_t i = 0; i < count; ++i) {
         Value reg = executor.arguments[callArgStart + i];
         if (registerOrError(executor, reg, "call", "return", command.file, command.line)) return;
         storeInRegister(executor, reg, executor.returnRegisters[i]);
      }
   }
}

// variables. set and move being the same with different order is intentional
void builtinSet(const Command &command, Executor &executor) {
   Value reg = arg(executor, command, 0);
   if (registerOrError(executor, reg, "set", "1st", command.file, command.line)) return;
   storeInRegister(executor, reg, resolveVariable(executor, arg(executor, command, 1), "set", command.file, command.line));
}

void builtinGlobal(const Command &command, Executor &executor) {
   Value value {VALUE_COUNT};
   size_t definitionCount = command.argCount;

   if (command.argCount > 1 && (arg(executor, command, command.argCount-1).type != VALUE_IDENTIFIER || (executor.values[arg(executor, command, command.argCount-1).identifier].init && executor.values[arg(executor, command, command.argCount-1).identifier].type == GLOBAL))) {
      value = resolveVariable(executor, arg(executor, command, command.argCount-1), "global", command.file, command.line);
      definitionCount -= 1;
   }
   for (size_t i = 0; i < definitionCount; ++i) {
      if (arg(executor, command, i).type != VALUE_IDENTIFIER) {
         error(executor.diagnostics, command.file, command.line, "global: Expected Identifier, but got %s instead", getValueName(arg(executor, command, i).type));
         continue;
      }
      size_t lexeme = arg(executor, command, i).identifier;
      if (executor.values[lexeme].init && executor.values[lexeme].type != GLOBAL) {
         error(executor.diagnostics, command.file, command.line, "global: Cannot define global '%s' as a %s with the same name already exists", getLexeme(executor.cache, lexeme).c_str(), getParseValueName(executor.values[lexeme].type));
         continue;
      }

      if (executor.values[lexeme].init) {
         deallocate(executor, executor.values[lexeme].global);
      }
      ParseValue global;
      global.type = GLOBAL;
      global.global = value;
      global.init = true;
      executor.values[lexeme] = global;
   }
}
