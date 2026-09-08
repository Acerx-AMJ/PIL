#include "builtin.hpp"
#include "pil.hpp"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <random>
#include <thread>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

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
   else if (value.type == VALUE_IDENTIFIER) {
      if (!executor.values[value.identifier].init || executor.values[value.identifier].type != GLOBAL) {
         error(executor.diagnostics, file, line, "%s: Variable '%s' does not exist", function, getLexeme(executor.cache, value.identifier).c_str());
         return value;
      }
      return executor.values[value.identifier].global;
   }
   else if (value.type == VALUE_REGISTER || value.type == VALUE_RETURN_REGISTER) {
      std::vector<Value> &registers = (value.type == VALUE_RETURN_REGISTER ? executor.returnRegisters : executor.registers);
      return registers[value.reg];
   }
   else {
      return value;
   }
}

Value arg(const Executor &executor, const Command &command, size_t i) {
   return executor.arguments[command.argStart + i];
}

Value back(const Executor &executor, const Command &command) {
   return executor.arguments[command.argStart + command.argCount - 1];
}

void storeInRegister(Executor &executor, const Command &command, Value reg, Value value, const char *function) {
   if (reg.type == VALUE_LOCAL) {
      copyValue(executor, executor.locals[executor.stackTrace.top().localStart + reg.local], value);
   }
   else if (reg.type == VALUE_IDENTIFIER) {
      if (!executor.values[reg.identifier].init || executor.values[reg.identifier].type != GLOBAL) {
         error(executor.diagnostics, command.file, command.line, "%s: Expected Register/Variable for the destination argument, got %s instead", function, getParseValueName(executor.values[reg.identifier].type));
         return;
      }
      copyValue(executor, executor.values[reg.identifier].global, value);
   }
   else if (reg.type == VALUE_REGISTER || reg.type == VALUE_RETURN_REGISTER) {
      std::vector<Value> &registers = (reg.type == VALUE_RETURN_REGISTER ? executor.returnRegisters : executor.registers);
      copyValue(executor, registers[reg.reg], value);
   }
   else {
      error(executor.diagnostics, command.file, command.line, "%s: Expected Register/Variable for the destination argument, got %s instead", function, getValueName(reg.type));
   }
}

void storeInRegister(Executor &executor, const Command &command, Value value, const char *function) {
   storeInRegister(executor, command, back(executor, command), value, function);
}

void jumpToLabel(Executor &executor, Value value, const char *function, const char *argument, size_t file, size_t line, bool condition) {
   if (value.type != VALUE_IDENTIFIER || !executor.values[value.identifier].init || executor.values[value.identifier].type != LABEL) {
      error(executor.diagnostics, file, line, "%s: Expected Label for the %s argument, got %s instead", function, argument, getValueName(value.type));
      return;
   }
   if (condition) {
      executor.pointer = executor.values[value.identifier].label - 1;
   }
}

double getNum(Executor &executor, const Command &command, size_t i, const char *function, bool *floating = nullptr) {
   Value value = resolveVariable(executor, arg(executor, command, i), function, command.file, command.line);
   if (value.type != VALUE_INTEGER && value.type != VALUE_FLOATING) {
      error(executor.diagnostics, command.file, command.line, "%s: Expected numeral, got %s instead", function, getValueName(value.type));
      return 0.0;
   }
   if (floating && value.type == VALUE_FLOATING) *floating = true;
   return (value.type == VALUE_INTEGER ? (double)value.integer : value.floating);
}

void storeNumber(Executor &executor, const Command &command, double number, bool floating, const char *function) {
   Value value {floating ? VALUE_FLOATING : VALUE_INTEGER};
   if (floating) {
      value.floating = number;
   }
   else {
      value.integer = number;
   }
   storeInRegister(executor, command, value, function);
}

void storeBoolean(Executor &executor, const Command &command, bool result, const char *function) {
   Value value {VALUE_INTEGER};
   value.integer = (result ? 1 : 0);
   storeInRegister(executor, command, value, function);
}

void unaryBuiltin(Executor &executor, const Command &command, double(*fn)(double), const char *function) {
   storeNumber(executor, command, fn(getNum(executor, command, 0, function)), true, function);
}

void binaryBuiltin(Executor &executor, const Command &command, double(*fn)(double, double), const char *function) {
   storeNumber(executor, command, fn(getNum(executor, command, 0, function), getNum(executor, command, 1, function)), true, function);
}

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

enum Comparison: char {
   COMPARISON_LESS, COMPARISON_GREATER, COMPARISON_EQUAL, COMPARISON_ERROR, COMPARISON_NOT_EQUAL
};

Comparison compareValues(Executor &executor, const Command &command, const char *function, bool softie) {
   Value a = resolveVariable(executor, arg(executor, command, 0), function, command.file, command.line);
   Value b = resolveVariable(executor, arg(executor, command, 1), function, command.file, command.line);

   if ((a.type == VALUE_INTEGER || a.type == VALUE_FLOATING) && (b.type == VALUE_INTEGER || b.type == VALUE_FLOATING)) {
      double x = (a.type == VALUE_INTEGER) ? (double)a.integer : a.floating;
      double y = (b.type == VALUE_INTEGER) ? (double)b.integer : b.floating;
      return x < y ? COMPARISON_LESS : x > y ? COMPARISON_GREATER : COMPARISON_EQUAL;
   }
   else if (a.type == VALUE_CHARACTER && b.type == VALUE_CHARACTER) {
      return a.character < b.character ? COMPARISON_LESS : a.character > b.character ? COMPARISON_GREATER : COMPARISON_EQUAL;
   }
   else if ((a.type == VALUE_STRING || a.type == VALUE_CSTRING) && (b.type == VALUE_STRING || b.type == VALUE_CSTRING)) {
      const std::string &as = (a.type == VALUE_STRING ? getString(executor, a.string, command.file, command.line) : getLexeme(executor.cache, a.string));
      const std::string &bs = (b.type == VALUE_STRING ? getString(executor, b.string, command.file, command.line) : getLexeme(executor.cache, b.string));
      int c = as.compare(bs);
      return c < 0 ? COMPARISON_LESS : c > 0 ? COMPARISON_GREATER : COMPARISON_EQUAL;
   }

   if (!softie) {
      error(executor.diagnostics, command.file, command.line, "%s: Cannot compare %s to %s", function, getValueName(a.type), getValueName(b.type));
      return COMPARISON_ERROR;
   }
   return COMPARISON_NOT_EQUAL;
}

void comparisonBuiltin(Executor &executor, const Command &command, const char *function, Comparison expected, bool reverse, bool softie) {
   Comparison result = compareValues(executor, command, function, softie);
   if (result != COMPARISON_ERROR) storeBoolean(executor, command, (result == expected) != reverse, function);
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

void storeString(Executor &executor, const Command &command, const std::string &string, const char *function) {
   Value value {VALUE_STRING, 0};
   value.string = allocateString(executor, string);
   storeInRegister(executor, command, value, function);
}

bool getBool(Executor &executor, const Command &command, size_t i, const char *function, const char *argument, bool &ok) {
   return isThruthy(executor, resolveVariable(executor, arg(executor, command, i), function, command.file, command.line), function, argument, ok, command.file, command.line);
}

// output
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

void builtinRead(const Command &command, Executor &executor) {
   std::string input;
   std::cin >> input;
   storeString(executor, command, input, "read");
}

void builtinReadline(const Command &command, Executor &executor) {
   std::string input;
   std::getline(std::cin, input);
   storeString(executor, command, input, "readline");
}

void builtinReadchar(const Command &command, Executor &executor) {
   char ch = getchar();
   Value value {VALUE_CHARACTER};
   value.character = ch;
   storeInRegister(executor, command, value, "readchar");
}

void builtinSetecho(const Command &command, Executor &executor) {
   bool ok;
   bool enable = getBool(executor, command, 0, "setecho", "1st", ok);
   if (!ok) return;
#ifdef _WIN32
   HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
   DWORD mode;
   GetConsoleMode(hStdin, &mode);
   mode = enable ? (mode | ENABLE_ECHO_INPUT) : (mode & ~ENABLE_ECHO_INPUT);
   SetConsoleMode(hStdin, mode);
#else
   termios tty;
   tcgetattr(STDIN_FILENO, &tty);
   if (enable) tty.c_lflag |= ECHO;
   else        tty.c_lflag &= ~ECHO;
   tcsetattr(STDIN_FILENO, TCSANOW, &tty);
#endif
}

// string
void builtinStringNew(const Command &command, Executor &executor) {
   std::string result;
   for (size_t i = 0; i < command.argCount - 1; ++i) {
      result += toString(executor, arg(executor, command, i), "string-new", command.file, command.line);
   }
   Value value {VALUE_STRING, 0};
   value.string = allocateString(executor, result);
   storeInRegister(executor, command, value, "string-new");
}

void builtinFormat(const Command &command, Executor &executor) {
   Value value {VALUE_STRING, 0};
   value.string = allocateString(executor, format(command, executor, "format", 1));
   storeInRegister(executor, command, value, "format");
}

// math
void builtinIncr(const Command &command, Executor &executor) {
   bool floating = false;
   double number = getNum(executor, command, 0, "incr");
   storeNumber(executor, command, number + 1, floating, "incr");
}

void builtinDecr(const Command &command, Executor &executor) {
   bool floating = false;
   double number = getNum(executor, command, 0, "decr");
   storeNumber(executor, command, number - 1, floating, "decr");
}

void builtinAdd(const Command &command, Executor &executor) {
   bool floating = false;
   double number = 0.0;
   for (size_t i = 0; i < command.argCount - 1; ++i) {
      number += getNum(executor, command, i, "add", &floating);
   }
   storeNumber(executor, command, number, floating, "add");
}

void builtinSub(const Command &command, Executor &executor) {
   bool floating = false;
   double number = getNum(executor, command, 0, "sub", &floating);
   for (size_t i = 1; i < command.argCount - 1; ++i) {
      number -= getNum(executor, command, i, "sub", &floating);
   }
   storeNumber(executor, command, number, floating, "sub");
}

void builtinMul(const Command &command, Executor &executor) {
   bool floating = false;
   double number = 1.0;
   for (size_t i = 0; i < command.argCount - 1; ++i) {
      number *= getNum(executor, command, i, "mul", &floating);
   }
   storeNumber(executor, command, number, floating, "mul");
}

void builtinDiv(const Command &command, Executor &executor) {
   bool floating = false;
   double number = getNum(executor, command, 0, "div", &floating);
   for (size_t i = 1; i < command.argCount - 1; ++i) {
      double num = getNum(executor, command, i, "div", &floating);
      number = (num == 0.0 ? 0.0 : number / num); // defined behavior
   }
   storeNumber(executor, command, number, floating, "div");
}

void builtinMod(const Command &command, Executor &executor) {
   bool floating = false;
   double a = getNum(executor, command, 0, "mod", &floating);
   double b = getNum(executor, command, 1, "mod", &floating);
   storeNumber(executor, command, fmod(a, b), floating, "mod");
}

void builtinPow(const Command &command, Executor &executor) {
   bool floating = false;
   double a = getNum(executor, command, 0, "pow", &floating);
   double b = getNum(executor, command, 1, "pow", &floating);
   storeNumber(executor, command, pow(a, b), floating, "pow");
}

void builtinNeg(const Command &command, Executor &executor) {
   bool floating = false;
   double n = getNum(executor,command, 0, "neg", &floating);
   storeNumber(executor, command, -n, floating, "neg");
}

void builtinSqrt(const Command &command, Executor &executor) {
   unaryBuiltin(executor, command, sqrt, "sqrt");
}

void builtinCbrt(const Command &command, Executor &executor) {
   unaryBuiltin(executor, command, cbrt, "cbrt");
}

void builtinSin(const Command &command, Executor &executor) {
   unaryBuiltin(executor, command, sin, "sin");
}

void builtinCos(const Command &command, Executor &executor) {
   unaryBuiltin(executor, command, cos, "cos");
}

void builtinTan(const Command &command, Executor &executor) {
   unaryBuiltin(executor, command, tan, "tan");
}

void builtinAsin(const Command &command, Executor &executor) {
   unaryBuiltin(executor, command, asin, "asin");
}

void builtinAcos(const Command &command, Executor &executor) {
   unaryBuiltin(executor, command, acos, "acos");
}

void builtinAtan(const Command &command, Executor &executor) {
   unaryBuiltin(executor, command, atan, "atan");
}

void builtinAtan2(const Command &command, Executor &executor) {
   binaryBuiltin(executor, command, atan2, "atan2");
}

void builtinAsinh(const Command &command, Executor &executor) {
   unaryBuiltin(executor, command, asinh, "asinh");
}

void builtinAcosh(const Command &command, Executor &executor) {
   unaryBuiltin(executor, command, acosh, "acosh");
}

void builtinAtanh(const Command &command, Executor &executor) {
   unaryBuiltin(executor, command, atanh, "atanh");
}

void builtinSinh(const Command &command, Executor &executor) {
   unaryBuiltin(executor, command, sinh, "sinh");
}

void builtinCosh(const Command &command, Executor &executor) {
   unaryBuiltin(executor, command, cosh, "cosh");
}

void builtinTanh(const Command &command, Executor &executor) {
   unaryBuiltin(executor, command, tanh, "tanh");
}

void builtinAbs(const Command &command, Executor &executor) {
   bool floating = false;
   double n = getNum(executor,command, 0, "abs", &floating);
   storeNumber(executor, command, fabs(n), floating, "abs");
}

void builtinMin(const Command &command, Executor &executor) {
   bool floating = false;
   double number = std::numeric_limits<double>::max();
   for (size_t i = 0; i < command.argCount - 1; ++i) {
      number = std::min(number, getNum(executor, command, i, "min", &floating));
   }
   storeNumber(executor, command, number, floating, "min");
}

void builtinMax(const Command &command, Executor &executor) {
   bool floating = false;
   double number = std::numeric_limits<double>::min();
   for (size_t i = 0; i < command.argCount - 1; ++i) {
      number = std::max(number, getNum(executor, command, i, "max", &floating));
   }
   storeNumber(executor, command, number, floating, "max");
}

void builtinClamp(const Command &command, Executor &executor) {
   bool floating = false;
   double x = getNum(executor, command, 0, "clamp", &floating);
   double lo = getNum(executor, command, 1, "clamp", &floating);
   double hi = getNum(executor, command, 2, "clamp", &floating);
   storeNumber(executor, command, std::clamp(x, lo, hi), floating, "clamp");
}

void builtinSign(const Command &command, Executor &executor) {
   double a = getNum(executor, command, 0, "sign");
   storeNumber(executor, command, (a < 0 ? -1 : a > 0 ? 1 : 0), false, "sign");
}

void builtinTrunc(const Command &command, Executor &executor) {
   unaryBuiltin(executor, command, trunc, "trunc");
}

void builtinCeil(const Command &command, Executor &executor) {
   unaryBuiltin(executor, command, ceil, "ceil");
}

void builtinFloor(const Command &command, Executor &executor) {
   unaryBuiltin(executor, command, floor, "floor");
}

void builtinRound(const Command &command, Executor &executor) {
   unaryBuiltin(executor, command, round, "round");
}

void builtinExp(const Command &command, Executor &executor) {
   unaryBuiltin(executor, command, exp, "exp");
}

void builtinLn(const Command &command, Executor &executor) {
   unaryBuiltin(executor, command, log, "ln");
}

void builtinLog(const Command &command, Executor &executor) {
   binaryBuiltin(executor, command, [](double a, double b){ return log(a) / log(b); }, "log");
}

void builtinLog2(const Command &command, Executor &executor) {
   unaryBuiltin(executor, command, log2, "log2");
}

void builtinLog10(const Command &command, Executor &executor) {
   unaryBuiltin(executor, command, log10, "log10");
}

void builtinLerp(const Command &command, Executor &executor) {
   double a = getNum(executor, command, 0, "lerp");
   double b = getNum(executor, command, 1, "lerp");
   double t = getNum(executor, command, 2, "lerp");
   storeNumber(executor, command, a + (b - a) * t, true, "lerp");
}

void builtinStepTowards(const Command &command, Executor &executor) {
   bool floating = false;
   double a = getNum(executor, command, 0, "step-towards", &floating);
   double b = getNum(executor, command, 1, "step-towards", &floating);
   storeNumber(executor, command, (a < b ? a + 1 : a > b ? a - 1 : a), floating, "step-towards");
}

// comparison
void builtinLe(const Command &command, Executor &executor) {
   comparisonBuiltin(executor, command, "le", COMPARISON_LESS, false, false);
}

void builtinGr(const Command &command, Executor &executor) {
   comparisonBuiltin(executor, command, "gr", COMPARISON_GREATER, false, false);
}

void builtinLeeq(const Command &command, Executor &executor) {
   comparisonBuiltin(executor, command, "leeq", COMPARISON_GREATER, true, false);
}

void builtinGreq(const Command &command, Executor &executor) {
   comparisonBuiltin(executor, command, "greq", COMPARISON_LESS, true, false);
}

void builtinEq(const Command &command, Executor &executor) {
   comparisonBuiltin(executor, command, "eq", COMPARISON_EQUAL, false, true);
}

void builtinNeq(const Command &command, Executor &executor) {
   comparisonBuiltin(executor, command, "neq", COMPARISON_EQUAL, true, true);
}

void builtinOr(const Command &command, Executor &executor) {
   bool ok;
   bool cond = getBool(executor, command, 0, "or", "value", ok);
   for (size_t i = 1; ok && i < command.argCount - 1; ++i) {
      cond = cond || getBool(executor, command, i, "or", "value", ok);
   }
   if (ok) storeBoolean(executor, command, cond, "or");
}

void builtinAnd(const Command &command, Executor &executor) {
   bool ok;
   bool cond = getBool(executor, command, 0, "and", "value", ok);
   for (size_t i = 1; ok && i < command.argCount - 1; ++i) {
      cond = cond && getBool(executor, command, i, "and", "value", ok);
   }
   if (ok) storeBoolean(executor, command, cond, "and");
}

void builtinNot(const Command &command, Executor &executor) {
   bool ok;
   bool thruthy = isThruthy(executor, arg(executor, command, 0), "not", "1st", ok, command.file, command.line);
   if (ok) storeBoolean(executor, command, thruthy, "not");
}

// control flow
void builtinGoto(const Command &command, Executor &executor) {
   jumpToLabel(executor, arg(executor, command, 0), "goto", "1st", command.file, command.line, true);
}

void builtinJmp(const Command &command, Executor &executor) {
   bool ok;
   bool thruthy = isThruthy(executor, arg(executor, command, 0), "jmp", "1st", ok, command.file, command.line);
   jumpToLabel(executor, arg(executor, command, 1), "jmp", "2nd", command.file, command.line, ok && thruthy);
}

void builtinJmpn(const Command &command, Executor &executor) {
   bool ok;
   bool thruthy = isThruthy(executor, arg(executor, command, 0), "jmpn", "1st", ok, command.file, command.line);
   jumpToLabel(executor, arg(executor, command, 1), "jmpn", "2nd", command.file, command.line, ok && !thruthy);
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
   size_t localCount = trace.localCount;
   executor.pointer = trace.position;
   executor.returnCount = command.argCount;

   if (executor.returnCount > executor.returnRegisters.size()) {
      error(executor.diagnostics, command.file, command.line, "return: Can return at maximum %zu values. Define 'return-register-count %zu' directive to mitigate. Error", executor.returnRegisters.size(), executor.returnCount);
      executor.stackTrace.pop();
      return;
   }

   for (size_t i = localStart; i < localStart + localCount; ++i) {
      deallocate(executor, executor.locals[i]);
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
         storeInRegister(executor, command, reg, executor.returnRegisters[i], "call");
      }
   }
}

// types
void builtinTypeof(const Command &command, Executor &executor) {
   Value value {VALUE_STRING, 0};
   Value v = arg(executor, command, 0);
   if (v.type == VALUE_IDENTIFIER && executor.values[v.identifier].init && (executor.values[v.identifier].type == FUNCTION || executor.values[v.identifier].type == NATIVE_FUNCTION)) {
      value.string = allocateString(executor, "function");
   }
   else if (v.type == VALUE_IDENTIFIER && executor.values[v.identifier].init && executor.values[v.identifier].type == LABEL) {
      value.string = allocateString(executor, "label");
   }
   else {
      ValueType type = resolveVariable(executor, v, "typeof", command.file, command.line).type;
      switch (type) {
      case VALUE_INTEGER: value.string = allocateString(executor, "int"); break;
      case VALUE_FLOATING: value.string = allocateString(executor, "float"); break;
      case VALUE_CHARACTER: value.string = allocateString(executor, "char"); break;
      case VALUE_STRING: case VALUE_CSTRING: value.string = allocateString(executor, "string"); break;
      default: value.string = allocateString(executor, "null"); break;
      }
   }
   storeInRegister(executor, command, value, "typeof");
}

void builtinSizeof(const Command &command, Executor &executor) {
   size_t size = 1;
   Value v = arg(executor, command, 0);
   if (v.type == VALUE_IDENTIFIER && executor.values[v.identifier].init) {
      ParseValueType type = executor.values[v.identifier].type;
      if (type == LABEL || type == FUNCTION || type == NATIVE_FUNCTION) {
         storeNumber(executor, command, 0, false, "sizeof");
         return;
      }
   }

   Value value = resolveVariable(executor, v, "sizeof", command.file, command.line);
   switch (value.type) {
   case VALUE_STRING: size = getString(executor, value.string, command.file, command.line).size(); break;
   case VALUE_CSTRING: size = getLexeme(executor.cache, value.string).size(); break;
   default: break;
   }
   storeNumber(executor, command, size, false, "sizeof");
}

void builtinIsnum(const Command &command, Executor &executor) {
   Value v = arg(executor, command, 0);
   if (v.type == VALUE_IDENTIFIER && executor.values[v.identifier].init) {
      ParseValueType type = executor.values[v.identifier].type;
      if (type == LABEL || type == FUNCTION || type == NATIVE_FUNCTION) {
         storeBoolean(executor, command, false, "is-num");
         return;
      }
   }
   Value value = resolveVariable(executor, v, "is-num", command.file, command.line);
   storeBoolean(executor, command, value.type == VALUE_INTEGER || value.type == VALUE_FLOATING, "is-num");
}

void builtinIsfloat(const Command &command, Executor &executor) {
   Value v = arg(executor, command, 0);
   if (v.type == VALUE_IDENTIFIER && executor.values[v.identifier].init) {
      ParseValueType type = executor.values[v.identifier].type;
      if (type == LABEL || type == FUNCTION || type == NATIVE_FUNCTION) {
         storeBoolean(executor, command, false, "is-float");
         return;
      }
   }
   Value value = resolveVariable(executor, v, "is-float", command.file, command.line);
   storeBoolean(executor, command, value.type == VALUE_FLOATING, "is-float");
}

void builtinIsint(const Command &command, Executor &executor) {
   Value v = arg(executor, command, 0);
   if (v.type == VALUE_IDENTIFIER && executor.values[v.identifier].init) {
      ParseValueType type = executor.values[v.identifier].type;
      if (type == LABEL || type == FUNCTION || type == NATIVE_FUNCTION) {
         storeBoolean(executor, command, false, "is-int");
         return;
      }
   }
   Value value = resolveVariable(executor, v, "is-int", command.file, command.line);
   storeBoolean(executor, command, value.type == VALUE_INTEGER, "is-int");
}

void builtinIschar(const Command &command, Executor &executor) {
   Value v = arg(executor, command, 0);
   if (v.type == VALUE_IDENTIFIER && executor.values[v.identifier].init) {
      ParseValueType type = executor.values[v.identifier].type;
      if (type == LABEL || type == FUNCTION || type == NATIVE_FUNCTION) {
         storeBoolean(executor, command, false, "is-char");
         return;
      }
   }
   Value value = resolveVariable(executor, v, "is-char", command.file, command.line);
   storeBoolean(executor, command, value.type == VALUE_CHARACTER, "is-char");
}

void builtinIsstring(const Command &command, Executor &executor) {
   Value v = arg(executor, command, 0);
   if (v.type == VALUE_IDENTIFIER && executor.values[v.identifier].init) {
      ParseValueType type = executor.values[v.identifier].type;
      if (type == LABEL || type == FUNCTION || type == NATIVE_FUNCTION) {
         storeBoolean(executor, command, false, "is-string");
         return;
      }
   }
   Value value = resolveVariable(executor, v, "is-string", command.file, command.line);
   storeBoolean(executor, command, value.type == VALUE_STRING || value.type == VALUE_CSTRING, "is-string");
}

void builtinIsreg(const Command &command, Executor &executor) {
   Value value = arg(executor, command, 0);
   storeBoolean(executor, command, value.type == VALUE_LOCAL || value.type == VALUE_REGISTER || value.type == VALUE_RETURN_REGISTER || (value.type == VALUE_IDENTIFIER && executor.values[value.identifier].init && executor.values[value.identifier].type == GLOBAL), "is-reg");
}

void builtinIsfunction(const Command &command, Executor &executor) {
   Value value = arg(executor, command, 0);
   storeBoolean(executor, command, value.type == VALUE_IDENTIFIER && executor.values[value.identifier].init && (executor.values[value.identifier].type == FUNCTION || executor.values[value.identifier].type == NATIVE_FUNCTION), "is-function");
}

void builtinIslabel(const Command &command, Executor &executor) {
   Value value = arg(executor, command, 0);
   storeBoolean(executor, command, value.type == VALUE_IDENTIFIER && executor.values[value.identifier].init && executor.values[value.identifier].type == LABEL, "is-label");
}

void builtinIsnull(const Command &command, Executor &executor) {
   Value v = arg(executor, command, 0);
   if (v.type == VALUE_IDENTIFIER && executor.values[v.identifier].init) {
      ParseValueType type = executor.values[v.identifier].type;
      if (type == LABEL || type == FUNCTION || type == NATIVE_FUNCTION) {
         storeBoolean(executor, command, false, "is-null");
         return;
      }
   }
   Value value = resolveVariable(executor, v, "is-null", command.file, command.line);
   storeBoolean(executor, command, value.type == VALUE_COUNT, "is-null");
}

void builtinIsinf(const Command &command, Executor &executor) {
   Value value = resolveVariable(executor, arg(executor, command, 0), "is-inf", command.file, command.line);
   storeBoolean(executor, command, value.type == VALUE_FLOATING && std::isinf(value.floating), "is-inf");
}

void builtinIsnan(const Command &command, Executor &executor) {
   Value value = resolveVariable(executor, arg(executor, command, 0), "is-nan", command.file, command.line);
   storeBoolean(executor, command, value.type == VALUE_FLOATING && std::isnan(value.floating), "is-nan");
}

void builtinToint(const Command &command, Executor &executor) {
   Value value = resolveVariable(executor, arg(executor, command, 0), "to-int", command.file, command.line);
   Value integer {VALUE_INTEGER};
   switch (value.type) {
   case VALUE_INTEGER: integer.integer = value.integer; break;
   case VALUE_FLOATING: integer.integer = value.floating; break;
   case VALUE_CHARACTER: integer.integer = value.character; break;
   case VALUE_STRING: try { integer.integer = std::stol(getString(executor, value.string, command.file, command.line)); } catch (...) { integer.type = VALUE_COUNT; }; break;
   case VALUE_CSTRING: try { integer.integer = std::stol(getLexeme(executor.cache, value.string)); } catch (...) { integer.type = VALUE_COUNT; }; break;
   default: error(executor.diagnostics, command.file, command.line, "to-int: Cannot convert %s to Integer", getValueName(value.type));
   }
   storeInRegister(executor, command, integer, "to-int");
}

void builtinTofloat(const Command &command, Executor &executor) {
   Value value = resolveVariable(executor, arg(executor, command, 0), "to-float", command.file, command.line);
   Value floating {VALUE_FLOATING};
   switch (value.type) {
   case VALUE_INTEGER: floating.floating = value.integer; break;
   case VALUE_FLOATING: floating.floating = value.floating; break;
   case VALUE_CHARACTER: floating.floating = value.character; break;
   case VALUE_STRING: try { floating.floating = std::stod(getString(executor, value.string, command.file, command.line)); } catch (...) { floating.type = VALUE_COUNT; }; break;
   case VALUE_CSTRING: try { floating.floating = std::stod(getLexeme(executor.cache, value.string)); } catch (...) { floating.type = VALUE_COUNT; }; break;
   default: error(executor.diagnostics, command.file, command.line, "to-float: Cannot convert %s to Floating", getValueName(value.type));
   }
   storeInRegister(executor, command, floating, "to-float");
}

void builtinTochar(const Command &command, Executor &executor) {
   Value value = resolveVariable(executor, arg(executor, command, 0), "to-char", command.file, command.line);
   Value character {VALUE_CHARACTER};
   switch (value.type) {
   case VALUE_INTEGER: character.character = value.integer; break;
   case VALUE_FLOATING: character.character = value.floating; break;
   case VALUE_CHARACTER: character.character = value.character; break;
   default: error(executor.diagnostics, command.file, command.line, "to-char: Cannot convert %s to Character", getValueName(value.type));
   }
   storeInRegister(executor, command, character, "to-char");
}

void builtinExists(const Command &command, Executor &executor) {
   Value v = arg(executor, command, 0);
   if (v.type == VALUE_IDENTIFIER) {
      storeBoolean(executor, command, executor.values[v.identifier].init, "exists");
   }
   else {
      storeBoolean(executor, command, true, "exists");
   }
}

// misc. (time, random)
void builtinTime(const Command &command, Executor &executor) {
   static const auto start = std::chrono::steady_clock::now();
   double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
   storeNumber(executor, command, ms, true, "time");
}

void builtinUnixTime(const Command &command, Executor &executor) {
   double epoch = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
   storeNumber(executor, command, epoch, false, "unix-time");
}

void builtinDate(const Command &command, Executor &executor) {
   Value string = arg(executor, command, 0);
   if (string.type != VALUE_CSTRING && string.type != VALUE_STRING) {
      error(executor.diagnostics, command.file, command.line, "date: Expected String for the 1st argument, got %s instead", getValueName(string.type));
      return;
   }
   std::string &str = (string.type == VALUE_CSTRING ? getLexeme(executor.cache, string.string) : getString(executor, string.string, command.file, command.line));
   long long t = std::time(nullptr);
   tm lt = *std::localtime(&t);
   std::ostringstream stream;
   stream << std::put_time(&lt, str.c_str());
   std::string result = stream.str();

   Value value {VALUE_STRING, 0};
   value.string = allocateString(executor, result);
   storeInRegister(executor, command, value, "date");
}

void builtinSleep(const Command &command, Executor &executor) {
   double s = getNum(executor, command, 0, "sleep");
   std::this_thread::sleep_for(std::chrono::duration<double>(s));
}

std::mt19937 &RNG() {
   static std::mt19937 rng {std::random_device{}()};
   return rng;
}

void builtinSeedRandom(const Command &command, Executor &executor) {
   double seed = getNum(executor, command, 0, "seed-random");
   RNG().seed(seed);
}

void builtinRandom(const Command &command, Executor &executor) {
   double r = std::uniform_real_distribution<double>{}(RNG());
   storeNumber(executor, command, r, true, "random");
}

void builtinRandfRange(const Command &command, Executor &executor) {
   double min = getNum(executor, command, 0, "randf-range");
   double max = getNum(executor, command, 1, "randf-range");
   if (min > max) {
      error(executor.diagnostics, command.file, command.line, "randf-range: Min %F is bigger than Max %F. Flip the arguments", min, max);
      return;
   }
   double r = std::uniform_real_distribution<double>{min, max}(RNG());
   storeNumber(executor, command, r, true, "randf-range");
}

void builtinRandiRange(const Command &command, Executor &executor) {
   long min = getNum(executor, command, 0, "randi-range");
   long max = getNum(executor, command, 1, "randi-range");
   if (min > max) {
      error(executor.diagnostics, command.file, command.line, "randi-range: Min %ld is bigger than Max %ld. Flip the arguments", min, max);
      return;
   }
   double r = std::uniform_int_distribution<long>{min, max}(RNG());
   storeNumber(executor, command, r, false, "randi-range");
}

// variables, registers
void builtinSwap(const Command &command, Executor &executor) {
   Value a = resolveVariable(executor, arg(executor, command, 0), "swap", command.file, command.line);
   Value b = resolveVariable(executor, arg(executor, command, 1), "swap", command.file, command.line);
   // avoid accidental deallocation
   a.allocations += 1;
   b.allocations += 1;
   storeInRegister(executor, command, arg(executor, command, 1), a, "swap");
   storeInRegister(executor, command, arg(executor, command, 0), b, "swap");
   a.allocations -= 1;
   b.allocations -= 1;
}

void builtinSet(const Command &command, Executor &executor) {
   storeInRegister(executor, command, resolveVariable(executor, arg(executor, command, 0), "set", command.file, command.line), "set");
}

void builtinGlobal(const Command &command, Executor &executor) {
   Value value {VALUE_COUNT};
   Value last = back(executor, command);
   size_t definitionCount = command.argCount;

   if (command.argCount > 1 && (last.type != VALUE_IDENTIFIER || (executor.values[last.identifier].init && executor.values[last.identifier].type == GLOBAL))) {
      value = resolveVariable(executor, last, "global", command.file, command.line);
      definitionCount -= 1;
   }
   for (size_t i = 0; i < definitionCount; ++i) {
      Value a = arg(executor, command, i);
      if (a.type != VALUE_IDENTIFIER) {
         error(executor.diagnostics, command.file, command.line, "global: Expected Identifier, but got %s instead", getValueName(a.type));
         continue;
      }
      size_t lexeme = a.identifier;
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
