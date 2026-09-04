#include "builtin.hpp"
#include "pil.hpp"
#include <algorithm>
#include <cmath>

// helper functions
Value resolveVariable(Executor &executor, Value value, const char *function) {
   if (value.type == VALUE_LOCAL) {
      return executor.stackTrace.top().locals[value.local];
   }

   if (value.type == VALUE_IDENTIFIER) {
      if (!executor.code.values[value.identifier].init || executor.code.values[value.identifier].type != GLOBAL) {
         error(executor.diagnostics, value.file, value.line, "%s: Variable '%s' does not exist", function, getLexeme(executor.cache, value.identifier).c_str());
         return value;
      }
      return executor.code.values[value.identifier].global;
   }

   if (value.type != VALUE_REGISTER && value.type != VALUE_RETURN_REGISTER) {
      return value;
   }
   std::vector<Value> &registers = (value.type == VALUE_RETURN_REGISTER ? executor.returnRegisters : executor.registers);

   if (value.reg < 0 || value.reg >= registers.size()) {
      error(executor.diagnostics, value.file, value.line, "%s: Register %s$%zu is out of bounds", function, value.type == VALUE_RETURN_REGISTER ? "R" : "", value.reg);
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
         error(executor.diagnostics, value.file, value.line, "%s: Expected Register/Variable for the %s argument, got %s instead", function, argument, getParseValueName(executor.code.values[value.identifier].type));
         return true;
      }
      return false;
   }

   if (value.type != VALUE_REGISTER && value.type != VALUE_RETURN_REGISTER) {
      error(executor.diagnostics, value.file, value.line, "%s: Expected Register/Variable for the %s argument, got %s instead", function, argument, getValueName(value.type));
      return true;
   }
   std::vector<Value> &registers = (value.type == VALUE_RETURN_REGISTER ? executor.returnRegisters : executor.registers);

   if (value.reg < 0 || value.reg >= registers.size()) {
      error(executor.diagnostics, value.file, value.line, "%s: Register %s$%zu is out of bounds", function, value.type == VALUE_RETURN_REGISTER ? "R" : "", value.reg);
      return true;
   }
   return false;
}

void storeInRegister(Executor &executor, Value reg, Value value) {
   switch (reg.type) {
   case VALUE_LOCAL: executor.stackTrace.top().locals[reg.local] = value; break;
   case VALUE_IDENTIFIER: executor.code.values[reg.identifier].global = value; break;
   case VALUE_REGISTER: executor.registers[reg.reg] = value; break;
   case VALUE_RETURN_REGISTER: executor.returnRegisters[reg.reg] = value; break;
   default:
      printf("PIL::storeInRegister: Cannot store into %s.\n", getValueName(value.type));
      exit(EXIT_FAILURE);
   }
}

bool labelOrError(Executor &executor, Value value, const char *function, const char *argument) {
   if (value.type != VALUE_IDENTIFIER || value.identifier >= executor.code.values.size() || !executor.code.values[value.identifier].init || executor.code.values[value.identifier].type != LABEL) {
      error(executor.diagnostics, value.file, value.line, "%s: Expected Label for the %s argument, got %s instead", function, argument, getValueName(value.type));
      return true;
   }
   return false;
}

// output
std::string toString(Executor &executor, Value value, const char *function) {
   value = resolveVariable(executor, value, function);
   switch (value.type) {
   case VALUE_INTEGER: return std::to_string(value.integer);
   case VALUE_FLOATING: return std::to_string(value.floating);
   case VALUE_CHARACTER: return std::string(1, value.character);
   case VALUE_STRING: return getLexeme(executor.cache, value.string);
   default: return "(null)";
   }
}

std::string format(const Command &command, Executor &executor, const char *function, size_t offset) {
   if (command.args[0].type != VALUE_STRING) {
      error(executor.diagnostics, command.file, command.line, "%s: Expected String for the 1st argument, got %s instead", function, getValueName(command.args[0].type));
      return "";
   }
   std::string result = getLexeme(executor.cache, command.args[0].string);
   size_t pos = 0;

   for (size_t i = 1; i < command.args.size() - offset; ++i) {
      pos = result.find("{}", pos);
      result = (pos != std::string::npos ? result.replace(pos, 2, toString(executor, command.args[i], function)) : result);
   }
   return result;
}

void print(const Command &command, Executor &executor, const char *function) {
   for (size_t i = 0; i < command.args.size(); ++i) {
      Value arg = resolveVariable(executor, command.args[i], function);
      switch (arg.type) {
      case VALUE_INTEGER:   printf("%ld", arg.integer); break;
      case VALUE_FLOATING:  printf("%.3F", arg.floating); break;
      case VALUE_CHARACTER: printf("%c", arg.character); break;
      case VALUE_STRING:    printf("%s", getLexeme(executor.cache, arg.string).c_str()); break;
      default: printf("(null)");
      }
   }
}

void builtinPrint(const Command &command, Executor &executor) {
   print(command, executor, "print");
}

void builtinPrintn(const Command &command, Executor &executor) {
   print(command, executor, "printn");
   putchar('\n');
}

void builtinPrintf(const Command &command, Executor &executor) {
   printf("%s", format(command, executor, "printf", 0).c_str());
}

void builtinPrintfn(const Command &command, Executor &executor) {
   printf("%s\n", format(command, executor, "printfn", 0).c_str());
}

void builtinStr(const Command &command, Executor &executor) {
   std::string result;
   for (size_t i = 0; i < command.args.size() - 1; ++i) {
      result += toString(executor, command.args[i], "str");
   }
   Value value {VALUE_STRING, command.line, command.file};
   value.string = pushLexeme(executor.cache, result);
   if (registerOrError(executor, command.args.back(), "str", "destination")) return;
   storeInRegister(executor, command.args.back(), value);
}

void builtinFormat(const Command &command, Executor &executor) {
   Value value {VALUE_STRING, command.line, command.file};
   value.string = pushLexeme(executor.cache, format(command, executor, "format", 1));
   if (registerOrError(executor, command.args.back(), "format", "destination")) return;
   storeInRegister(executor, command.args.back(), value);
}

// math
double getNum(Executor &executor, Value value, const char *function, bool *floating = nullptr) {
   value = resolveVariable(executor, value, function);
   if (value.type != VALUE_INTEGER && value.type != VALUE_FLOATING) {
      error(executor.diagnostics, value.file, value.line, "%s: Expected numeral, got %s instead", function, getValueName(value.type));
      return 0.0;
   }
   if (floating && value.type == VALUE_FLOATING) *floating = true;
   return (value.type == VALUE_INTEGER ? (double)value.integer : value.floating);
}

void storeNumber(Executor &executor, Value reg, double number, bool floating) {
   Value value {floating ? VALUE_FLOATING : VALUE_INTEGER, reg.line, reg.file};
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
   for (size_t i = 0; i < command.args.size() - 1; ++i) {
      number += getNum(executor, command.args[i], "add", &floating);
   }
   if (registerOrError(executor, command.args.back(), "add", "destination")) return;
   storeNumber(executor, command.args.back(), number, floating);
}

void builtinSub(const Command &command, Executor &executor) {
   bool floating = false;
   double number = getNum(executor, command.args[0], "sub", &floating);
   for (size_t i = 1; i < command.args.size() - 1; ++i) {
      number -= getNum(executor, command.args[i], "sub", &floating);
   }
   if (registerOrError(executor, command.args.back(), "sub", "destination")) return;
   storeNumber(executor, command.args.back(), number, floating);
}

void builtinMul(const Command &command, Executor &executor) {
   bool floating = false;
   double number = 1.0;
   for (size_t i = 0; i < command.args.size() - 1; ++i) {
      number *= getNum(executor, command.args[i], "mul", &floating);
   }
   if (registerOrError(executor, command.args.back(), "mul", "destination")) return;
   storeNumber(executor, command.args.back(), number, floating);
}

void builtinDiv(const Command &command, Executor &executor) {
   bool floating = false;
   double number = getNum(executor, command.args[0], "div", &floating);
   for (size_t i = 1; i < command.args.size() - 1; ++i) {
      double num = getNum(executor, command.args[i], "div", &floating);
      number = (num == 0.0 ? 0.0 : number / num); // defined behavior
   }
   if (registerOrError(executor, command.args.back(), "div", "destination")) return;
   storeNumber(executor, command.args.back(), number, floating);
}

void builtinMod(const Command &command, Executor &executor) {
   bool floating = false;
   double a = getNum(executor, command.args[0], "mod", &floating);
   double b = getNum(executor, command.args[1], "mod", &floating);
   if (registerOrError(executor, command.args[2], "mod", "destination")) return;
   storeNumber(executor, command.args[2], fmod(a, b), floating);
}

void builtinPow(const Command &command, Executor &executor) {
   bool floating = false;
   double a = getNum(executor, command.args[0], "pow", &floating);
   double b = getNum(executor, command.args[1], "pow", &floating);
   if (registerOrError(executor, command.args[2], "pow", "destination")) return;
   storeNumber(executor, command.args[2], pow(a, b), floating);
}

void builtinNeg(const Command &command, Executor &executor) {
   bool floating = false;
   if (registerOrError(executor, command.args[1], "neg", "destination")) return;
   storeNumber(executor, command.args[1], -getNum(executor, command.args[0], "neg", &floating), floating);
}

void builtinSqrt(const Command &command, Executor &executor) {
   if (registerOrError(executor, command.args[1], "sqrt", "destination")) return;
   storeNumber(executor, command.args[1], sqrt(getNum(executor, command.args[0], "sqrt")), true);
}

void builtinCbrt(const Command &command, Executor &executor) {
   if (registerOrError(executor, command.args[1], "cbrt", "destination")) return;
   storeNumber(executor, command.args[1], cbrt(getNum(executor, command.args[0], "cbrt")), true);
}

void builtinSin(const Command &command, Executor &executor) {
   if (registerOrError(executor, command.args[1], "sin", "destination")) return;
   storeNumber(executor, command.args[1], sin(getNum(executor, command.args[0], "sin")), true);
}

void builtinCos(const Command &command, Executor &executor) {
   if (registerOrError(executor, command.args[1], "cos", "destination")) return;
   storeNumber(executor, command.args[1], cos(getNum(executor, command.args[0], "cos")), true);
}

void builtinTan(const Command &command, Executor &executor) {
   if (registerOrError(executor, command.args[1], "tan", "destination")) return;
   storeNumber(executor, command.args[1], tan(getNum(executor, command.args[0], "tan")), true);
}

void builtinAsin(const Command &command, Executor &executor) {
   if (registerOrError(executor, command.args[1], "asin", "destination")) return;
   storeNumber(executor, command.args[1], asin(getNum(executor, command.args[0], "asin")), true);
}

void builtinAcos(const Command &command, Executor &executor) {
   if (registerOrError(executor, command.args[1], "acos", "destination")) return;
   storeNumber(executor, command.args[1], acos(getNum(executor, command.args[0], "acos")), true);
}

void builtinAtan(const Command &command, Executor &executor) {
   if (registerOrError(executor, command.args[1], "atan", "destination")) return;
   storeNumber(executor, command.args[1], atan(getNum(executor, command.args[0], "atan")), true);
}

void builtinAtan2(const Command &command, Executor &executor) {
   if (registerOrError(executor, command.args[2], "atan2", "destination")) return;
   storeNumber(executor, command.args[2], atan2(getNum(executor, command.args[0], "atan2"), getNum(executor, command.args[1], "atan2")), true);
}

void builtinAsinh(const Command &command, Executor &executor) {
   if (registerOrError(executor, command.args[1], "asinh", "destination")) return;
   storeNumber(executor, command.args[1], asinh(getNum(executor, command.args[0], "asinh")), true);
}

void builtinAcosh(const Command &command, Executor &executor) {
   if (registerOrError(executor, command.args[1], "acosh", "destination")) return;
   storeNumber(executor, command.args[1], acosh(getNum(executor, command.args[0], "acosh")), true);
}

void builtinAtanh(const Command &command, Executor &executor) {
   if (registerOrError(executor, command.args[1], "atanh", "destination")) return;
   storeNumber(executor, command.args[1], atanh(getNum(executor, command.args[0], "atanh")), true);
}

void builtinSinh(const Command &command, Executor &executor) {
   if (registerOrError(executor, command.args[1], "sinh", "destination")) return;
   storeNumber(executor, command.args[1], sinh(getNum(executor, command.args[0], "sinh")), true);
}

void builtinCosh(const Command &command, Executor &executor) {
   if (registerOrError(executor, command.args[1], "cosh", "destination")) return;
   storeNumber(executor, command.args[1], cosh(getNum(executor, command.args[0], "cosh")), true);
}

void builtinTanh(const Command &command, Executor &executor) {
   if (registerOrError(executor, command.args[1], "tanh", "destination")) return;
   storeNumber(executor, command.args[1], tanh(getNum(executor, command.args[0], "tanh")), true);
}

void builtinAbs(const Command &command, Executor &executor) {
   if (registerOrError(executor, command.args[1], "abs", "destination")) return;
   storeNumber(executor, command.args[1], fabs(getNum(executor, command.args[0], "abs")), true);
}

void builtinMin(const Command &command, Executor &executor) {
   bool floating = false;
   double number = std::numeric_limits<double>::max();
   for (size_t i = 0; i < command.args.size() - 1; ++i) {
      number = std::min(number, getNum(executor, command.args[i], "min", &floating));
   }
   if (registerOrError(executor, command.args.back(), "min", "destination")) return;
   storeNumber(executor, command.args.back(), number, floating);
}

void builtinMax(const Command &command, Executor &executor) {
   bool floating = false;
   double number = std::numeric_limits<double>::min();
   for (size_t i = 0; i < command.args.size() - 1; ++i) {
      number = std::max(number, getNum(executor, command.args[i], "max", &floating));
   }
   if (registerOrError(executor, command.args.back(), "max", "destination")) return;
   storeNumber(executor, command.args.back(), number, floating);
}

void builtinClamp(const Command &command, Executor &executor) {
   bool floating = false;
   double x = getNum(executor, command.args[0], "clamp", &floating);
   double lo = getNum(executor, command.args[1], "clamp", &floating);
   double hi = getNum(executor, command.args[2], "clamp", &floating);
   if (registerOrError(executor, command.args[3], "clamp", "destination")) return;
   storeNumber(executor, command.args[3], std::clamp(x, lo, hi), floating);
}

void builtinCeil(const Command &command, Executor &executor) {
   if (registerOrError(executor, command.args[1], "ceil", "destination")) return;
   storeNumber(executor, command.args[1], ceil(getNum(executor, command.args[0], "ceil")), true);
}

void builtinFloor(const Command &command, Executor &executor) {
   if (registerOrError(executor, command.args[1], "floor", "destination")) return;
   storeNumber(executor, command.args[1], floor(getNum(executor, command.args[0], "floor")), true);
}

void builtinRound(const Command &command, Executor &executor) {
   if (registerOrError(executor, command.args[1], "round", "destination")) return;
   storeNumber(executor, command.args[1], round(getNum(executor, command.args[0], "round")), true);
}

void builtinExp(const Command &command, Executor &executor) {
   if (registerOrError(executor, command.args[1], "exp", "destination")) return;
   storeNumber(executor, command.args[1], exp(getNum(executor, command.args[0], "exp")), true);
}

void builtinLn(const Command &command, Executor &executor) {
   if (registerOrError(executor, command.args[1], "ln", "destination")) return;
   storeNumber(executor, command.args[1], log(getNum(executor, command.args[0], "ln")), true);
}

void builtinLog(const Command &command, Executor &executor) {
   if (registerOrError(executor, command.args[2], "log", "destination")) return;
   storeNumber(executor, command.args[2], log(getNum(executor, command.args[0], "log")) / log(getNum(executor, command.args[1], "log")), true);
}

void builtinLog2(const Command &command, Executor &executor) {
   if (registerOrError(executor, command.args[1], "log2", "destination")) return;
   storeNumber(executor, command.args[1], log2(getNum(executor, command.args[0], "log2")), true);
}

void builtinLog10(const Command &command, Executor &executor) {
   if (registerOrError(executor, command.args[1], "log10", "destination")) return;
   storeNumber(executor, command.args[1], log10(getNum(executor, command.args[0], "log10")), true);
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
      error(executor.diagnostics, lhs.file, lhs.line, "%s: Cannot compare %s and %s", function, getValueName(a.type), getValueName(b.type));
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
      error(executor.diagnostics, value.file, value.line, "%s: Expected value for the %s argument, got %s", function, argument, getValueName(v.type));
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
            error(executor.diagnostics, command.file, command.line, "call: Cannot call multiple functions in a single call");
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
      error(executor.diagnostics, command.file, command.line, "call: Expected function name to call");
      return;
   }
   ParseValue &function = executor.code.values[command.args[functionPos].identifier];
   size_t params = function.params.size();
   bool variadic = function.variadic;

   if ((!variadic && argCount != params) || (variadic && argCount < params)) {
      error(executor.diagnostics, command.file, command.line, "call: Called function expected %s%zu parameters, but received %zu arguments", (variadic ? ">" : ""), params, argCount);
      return;
   }

   if (function.type == NATIVE_FUNCTION) {
      error(executor.diagnostics, command.file, command.line, "call: Cannot call native function. Remove excess call");
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
      error(executor.diagnostics, command.file, command.line, "return: Can return at maximum %zu values. Define 'return-register-count %zu' directive to mitigate. Error", executor.returnRegisters.size(), executor.returnCount);
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
         warn(executor.diagnostics, call.file, call.line, "call: Expected %zu return values, but got %zu instead", callExpectedReturnCount, executor.returnCount);
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
         error(executor.diagnostics, command.file, command.line, "global: Expected Identifier, but got %s instead", getValueName(command.args[i].type));
         continue;
      }
      size_t lexeme = command.args[i].identifier;
      if (executor.code.values[lexeme].init && executor.code.values[lexeme].type != GLOBAL) {
         error(executor.diagnostics, command.file, command.line, "global: Cannot define global '5s' as a 5s with the same name already exists", getLexeme(executor.cache, lexeme).c_str(), getParseValueName(executor.code.values[lexeme].type));
         continue;
      }
      ParseValue global;
      global.type = GLOBAL;
      global.global = value;
      global.init = true;
      executor.code.values[lexeme] = global;
   }
}
