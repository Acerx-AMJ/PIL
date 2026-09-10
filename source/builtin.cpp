#include "builtin.hpp"
#include "builtinhelpers.hpp"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <random>
#include <thread>

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
   setEcho(getBool(executor, command, 0));
}

// string
void builtinStringNew(const Command &command, Executor &executor) {
   std::string result;
   for (size_t i = 0; i < command.argCount - 1; ++i) {
      result += toString(executor, arg(executor, command, i), "string-new", command.file, command.line);
   }
   storeString(executor, command, result, "string-new");
}

void builtinFormat(const Command &command, Executor &executor) {
   storeString(executor, command, format(command, executor, "format", 1), "format");
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
   bool cond = getBool(executor, command, 0);
   for (size_t i = 1; i < command.argCount - 1; ++i) {
      cond = cond || getBool(executor, command, i);
   }
   storeBoolean(executor, command, cond, "or");
}

void builtinAnd(const Command &command, Executor &executor) {
   bool cond = getBool(executor, command, 0);
   for (size_t i = 1; i < command.argCount - 1; ++i) {
      cond = cond && getBool(executor, command, i);
   }
   storeBoolean(executor, command, cond, "and");
}

void builtinNot(const Command &command, Executor &executor) {
   storeBoolean(executor, command, !getBool(executor, command, 0), "not");
}

// control flow
void builtinGoto(const Command &command, Executor &executor) {
   jumpToLabel(executor, arg(executor, command, 0), "goto", "1st", command.file, command.line, true);
}

void builtinJmp(const Command &command, Executor &executor) {
   jumpToLabel(executor, arg(executor, command, 1), "jmp", "2nd", command.file, command.line, getBool(executor, command, 0));
}

void builtinJmpn(const Command &command, Executor &executor) {
   jumpToLabel(executor, arg(executor, command, 1), "jmpn", "2nd", command.file, command.line, !getBool(executor, command, 0));
}

void builtinJmptable(const Command &command, Executor &executor) {
   // jmptable value, result1, label1, result2, label2, ...
   if (command.argCount % 2 != 1) {
      error(executor.diagnostics, command.file, command.line, "jmptable: Expected odd number of arguments");
      return;
   }
   Value value = resolveVariable(executor, arg(executor, command, 0));
   for (size_t i = 1; i < command.argCount; i += 2) {
      Value result = resolveVariable(executor, arg(executor, command, i));
      jumpToLabel(executor, arg(executor, command, i + 1), "jmptable", "destination", command.file, command.line, valuesEqual(executor, command, value, result));
   }
}

void builtinCall(const Command &command, Executor &executor) {
   Function &function = executor.functions[arg(executor, command, command.callee).function];
   call(executor, command, function, command.callee, command.callee, command.argCount - command.callee - 1);
}

void builtinReturn(const Command &command, Executor &executor) {
   if (executor.stackTrace.size() <= 1) {
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

   for (size_t i = localStart; i < localStart + localCount; ++i) {
      deallocate(executor, executor.locals[i]);
   }

   for (size_t i = 0; i < executor.returnCount; ++i) {
      Value value = resolveVariable(executor, arg(executor, command, i));
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
   ValueType type = resolveVariable(executor, arg(executor, command, 0)).type;
   const char *string;
   switch (type) {
   case VALUE_INTEGER: string = "int"; break;
   case VALUE_FLOATING: string = "float"; break;
   case VALUE_CHARACTER: string = "char"; break;
   case VALUE_STRING: case VALUE_CSTRING: string = "string"; break;
   case VALUE_FUNCTION: string = "function"; break;
   case VALUE_LABEL: string = "label"; break;
   case VALUE_COUNT: string = "null"; break;
   default:
      printf("PIL::builtinTypeof: Cannot get the type of value %s.\n", getValueName(type));
      exit(EXIT_FAILURE);
   }
   storeString(executor, command, string, "typeof");
}

void builtinSizeof(const Command &command, Executor &executor) {
   size_t size = 1;
   Value value = resolveVariable(executor, arg(executor, command, 0));
   switch (value.type) {
   case VALUE_STRING: size = getString(executor, value.string, command.file, command.line).size(); break;
   case VALUE_CSTRING: size = getLexeme(executor.cache, value.string).size(); break;
   default: break;
   }
   storeNumber(executor, command, size, false, "sizeof");
}

void builtinIsnum(const Command &command, Executor &executor) {
   Value value = resolveVariable(executor, arg(executor, command, 0));
   storeBoolean(executor, command, value.type == VALUE_INTEGER || value.type == VALUE_FLOATING, "is-num");
}

void builtinIsfloat(const Command &command, Executor &executor) {
   Value value = resolveVariable(executor, arg(executor, command, 0));
   storeBoolean(executor, command, value.type == VALUE_FLOATING, "is-float");
}

void builtinIsint(const Command &command, Executor &executor) {
   Value value = resolveVariable(executor, arg(executor, command, 0));
   storeBoolean(executor, command, value.type == VALUE_INTEGER, "is-int");
}

void builtinIschar(const Command &command, Executor &executor) {
   Value value = resolveVariable(executor, arg(executor, command, 0));
   storeBoolean(executor, command, value.type == VALUE_CHARACTER, "is-char");
}

void builtinIsstring(const Command &command, Executor &executor) {
   Value value = resolveVariable(executor, arg(executor, command, 0));
   storeBoolean(executor, command, value.type == VALUE_STRING || value.type == VALUE_CSTRING, "is-string");
}

void builtinIsreg(const Command &command, Executor &executor) {
   Value value = arg(executor, command, 0);
   storeBoolean(executor, command, value.type == VALUE_LOCAL || value.type == VALUE_REGISTER || value.type == VALUE_RETURN_REGISTER, "is-reg");
}

void builtinIsfunction(const Command &command, Executor &executor) {
   Value value = resolveVariable(executor, arg(executor, command, 0));
   storeBoolean(executor, command, value.type == VALUE_FUNCTION, "is-function");
}

void builtinIslabel(const Command &command, Executor &executor) {
   Value value = resolveVariable(executor, arg(executor, command, 0));
   storeBoolean(executor, command, value.type == VALUE_LABEL, "is-label");
}

void builtinIsnull(const Command &command, Executor &executor) {
   Value value = resolveVariable(executor, arg(executor, command, 0));
   storeBoolean(executor, command, value.type == VALUE_COUNT, "is-null");
}

void builtinIsinf(const Command &command, Executor &executor) {
   Value value = resolveVariable(executor, arg(executor, command, 0));
   storeBoolean(executor, command, value.type == VALUE_FLOATING && std::isinf(value.floating), "is-inf");
}

void builtinIsnan(const Command &command, Executor &executor) {
   Value value = resolveVariable(executor, arg(executor, command, 0));
   storeBoolean(executor, command, value.type == VALUE_FLOATING && std::isnan(value.floating), "is-nan");
}

void builtinToint(const Command &command, Executor &executor) {
   Value value = resolveVariable(executor, arg(executor, command, 0));
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
   Value value = resolveVariable(executor, arg(executor, command, 0));
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
   Value value = resolveVariable(executor, arg(executor, command, 0));
   Value character {VALUE_CHARACTER};
   switch (value.type) {
   case VALUE_INTEGER: character.character = value.integer; break;
   case VALUE_FLOATING: character.character = value.floating; break;
   case VALUE_CHARACTER: character.character = value.character; break;
   default: error(executor.diagnostics, command.file, command.line, "to-char: Cannot convert %s to Character", getValueName(value.type));
   }
   storeInRegister(executor, command, character, "to-char");
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
   storeString(executor, command, stream.str().c_str(), "date");
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
   Value &a = resolveVariableByRef(executor, executor.arguments[command.argStart + 0]);
   Value &b = resolveVariableByRef(executor, executor.arguments[command.argStart + 1]);
   std::swap(a, b);
}

void builtinSet(const Command &command, Executor &executor) {
   storeInRegister(executor, command, resolveVariable(executor, arg(executor, command, 0)), "set");
}
