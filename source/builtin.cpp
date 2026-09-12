#include "builtin.hpp"
#include "builtinhelpers.hpp"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
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

// string ops
void builtinStringNew(const Command &command, Executor &executor) {
   std::string result;
   for (size_t i = 0; i < command.argCount - 1; ++i) {
      result += toString(executor, arg(executor, command, i), "string-new", command.file, command.line);
   }
   storeString(executor, command, result, "string-new");
}

void builtinStringFmt(const Command &command, Executor &executor) {
   storeString(executor, command, format(command, executor, "string-fmt", 1), "string-fmt");
}

// array ops
void builtinArrayNew(const Command &command, Executor &executor) {
   std::vector<Value> values (command.argCount - 1);
   for (size_t i = 1; i < command.argCount; ++i) {
      values[i-1] = resolveVariable(executor, arg(executor, command, i));
   }
   storeArray(executor, command, values, arg(executor, command, 0), "array-new");
}

void builtinArrayFill(const Command &command, Executor &executor) {
   size_t count = getNum(executor, command, 1, "array-fill");
   std::vector<Value> values (count, resolveVariable(executor, arg(executor, command, 2)));
   storeArray(executor, command, values, arg(executor, command, 0), "array-fill");
}

void builtinArrayIota(const Command &command, Executor &executor) {
   size_t count = getNum(executor, command, 1, "array-fill");
   size_t start = getNum(executor, command, 2, "array-fill");
   std::vector<Value> values (count);
   for (size_t i = 0; i < count; ++i) {
      values[i].type = VALUE_INTEGER;
      values[i].integer = start + i;
   }
   storeArray(executor, command, values, arg(executor, command, 0), "array-fill");
}

void builtinArrayClear(const Command &command, Executor &executor) {
   std::vector<Value> *array;
   if (!arrayOrError(command, executor, "array-clear", array)) return;
   array->clear();
}

void builtinArrayMemFree(const Command &command, Executor &executor) {
   std::vector<Value> *array;
   if (!arrayOrError(command, executor, "array-memfree", array)) return;
   array->clear();
   array->shrink_to_fit();
}

void builtinArrayEmpty(const Command &command, Executor &executor) {
   std::vector<Value> *array;
   if (!arrayOrError(command, executor, "array-empty", array)) return;
   storeBoolean(executor, command, array->empty(), "array-empty");
}

void builtinArraySize(const Command &command, Executor &executor) {
   std::vector<Value> *array;
   if (!arrayOrError(command, executor, "array-size", array)) return;
   storeNumber(executor, command, array->size(), false, "array-size");
}

void builtinArrayCapacity(const Command &command, Executor &executor) {
   std::vector<Value> *array;
   if (!arrayOrError(command, executor, "array-capacity", array)) return;
   storeNumber(executor, command, array->capacity(), false, "array-capacity");
}

void builtinArrayReserve(const Command &command, Executor &executor) {
   std::vector<Value> *array;
   if (!arrayOrError(command, executor, "array-reserve", array)) return;
   array->reserve(getNum(executor, command, 1, "array-reserve"));
}

void builtinArrayResize(const Command &command, Executor &executor) {
   std::vector<Value> *array;
   if (!arrayOrError(command, executor, "array-resize", array)) return;
   array->resize(getNum(executor, command, 1, "array-resize"), resolveVariable(executor, arg(executor, command, 2)));
}

void builtinArraySet(const Command &command, Executor &executor) {
   std::vector<Value> *array;
   if (!arrayOrError(command, executor, "array-set", array)) return;
   size_t id = getNum(executor, command, 1, "array-set");
   if (id < 0 || id >= array->size()) {
      error(executor.diagnostics, command.file, command.line, "array-set: Index %zu is out of bounds", id);
      return;
   }
   (*array)[id] = resolveVariable(executor, arg(executor, command, 2));
}

void builtinArrayIdx(const Command &command, Executor &executor) {
   std::vector<Value> *array;
   if (!arrayOrError(command, executor, "array-idx", array)) return;
   size_t id = getNum(executor, command, 1, "array-idx");
   if (id < 0 || id >= array->size()) {
      error(executor.diagnostics, command.file, command.line, "array-idx: Index %zu is out of bounds", id);
      return;
   }
   storeInRegister(executor, command, (*array)[id], "array-idx");
}

void builtinArrayBack(const Command &command, Executor &executor) {
   std::vector<Value> *array;
   if (!arrayOrError(command, executor, "array-back", array)) return;
   if (array->empty()) {
      error(executor.diagnostics, command.file, command.line, "array-back: Cannot get the back element of array since the array is empty");
      return;
   }
   storeInRegister(executor, command, array->back(), "array-back");
}

void builtinArrayFront(const Command &command, Executor &executor) {
   std::vector<Value> *array;
   if (!arrayOrError(command, executor, "array-front", array)) return;
   if (array->empty()) {
      error(executor.diagnostics, command.file, command.line, "array-front: Cannot get the front element of array since the array is empty");
      return;
   }
   storeInRegister(executor, command, array->front(), "array-front");
}

void builtinArrayPush(const Command &command, Executor &executor) {
   std::vector<Value> *array;
   if (!arrayOrError(command, executor, "array-push", array)) return;
   array->push_back(resolveVariable(executor, arg(executor, command, 1)));
}

void builtinArrayInsert(const Command &command, Executor &executor) {
   std::vector<Value> *array;
   if (!arrayOrError(command, executor, "array-insert", array)) return;
   size_t id = getNum(executor, command, 1, "array-insert");
   if (id < 0 || id > array->size()) {
      error(executor.diagnostics, command.file, command.line, "array-insert: Index %zu is out of bounds", id);
      return;
   }
   array->insert(array->begin() + id, resolveVariable(executor, arg(executor, command, 2)));
}

void builtinArrayPop(const Command &command, Executor &executor) {
   std::vector<Value> *array;
   if (!arrayOrError(command, executor, "array-pop", array)) return;
   if (array->empty()) {
      error(executor.diagnostics, command.file, command.line, "array-pop: Cannot pop from an empty array");
      return;
   }
   array->pop_back();
}

void builtinArrayErase(const Command &command, Executor &executor) {
   std::vector<Value> *array;
   if (!arrayOrError(command, executor, "array-erase", array)) return;
   size_t id = getNum(executor, command, 1, "array-erase");
   if (id < 0 || id >= array->size()) {
      error(executor.diagnostics, command.file, command.line, "array-erase: Index %zu is out of bounds", id);
      return;
   }
   array->erase(array->begin() + id);
}

void builtinArrayFree(const Command &command, Executor &executor) {
   for (size_t i = 0; i < command.argCount; ++i) {
      Value a = arg(executor, command, i);
      Value &array = resolveVariableByRef(executor, a);
      if (array.type != VALUE_ARRAY) {
         error(executor.diagnostics, command.file, command.line, "array-free: Expected array, got %s instead", getValueName(array.type));
         return;
      }
      executor.arrays.erase(array.array);
      array = NULL_VALUE;
   }
}

void builtinArrayDeepFree(const Command &command, Executor &executor) {
   std::unordered_set<size_t> visited;
   for (size_t i = 0; i < command.argCount; ++i) {
      Value a = arg(executor, command, i);
      Value &array = resolveVariableByRef(executor, a);
      if (array.type != VALUE_ARRAY) {
         error(executor.diagnostics, command.file, command.line, "array-free: Expected array, got %s instead", getValueName(array.type));
         return;
      }
      visited.clear();
      deepFree(command, executor, array, visited);
   }
}

void builtinArrayMark(const Command &command, Executor &executor) {
   Value array = resolveVariable(executor, arg(executor, command, 0));
   if (array.type != VALUE_ARRAY) {
      error(executor.diagnostics, command.file, command.line, "array-mark: Expected array, got %s instead", getValueName(array.type));
      return;
   }
   auto it = executor.arrays.find(array.array);
   if (it == executor.arrays.end()) {
      error(executor.diagnostics, command.file, command.line, "Invalid array ID %zu. Use after free", array.array);
      return;
   }
   it->second.mark = getNum(executor, command, 1, "array-mark");
}

void builtinArrayFreeMarked(const Command &command, Executor &executor) {
   int mark = getNum(executor, command, 0, "array-free-marked");
   for (auto &[id, array]: executor.arrays) {
      if (array.mark == mark) {
         executor.arrays.erase(id);
      }
   }
}

void builtinArrayJoin(const Command &command, Executor &executor) {
   std::vector<Value> *array;
   if (!arrayOrError(command, executor, "array-join", array)) return;
   size_t size = array->size();
   std::string connector = toString(executor, arg(executor, command, 1), "array-join", command.file, command.line);
   std::string result;
   result.reserve((2 + connector.size()) * size);
   for (size_t i = 0; i < size; ++i) {
      result += toString(executor, (*array)[i], "array-join", command.file, command.line);
      if (i + 1 < size) result += connector;
   }
   storeString(executor, command, result, "array-join");
}

void builtinArrayConcat(const Command &command, Executor &executor) {
   std::vector<Value> *array1, *array2;
   if (!arrayOrError(command, executor, "array-concat", array1) || !arrayOrError(command, executor, "array-concat", array2, 1)) return;
   std::vector<Value> result = *array1;
   result.insert(result.end(), array2->begin(), array2->end());
   storeArray(executor, command, result, back(executor, command), "array-concat");
}

void builtinArraySlice(const Command &command, Executor &executor) {
   std::vector<Value> *array;
   if (!arrayOrError(command, executor, "array-slice", array)) return;
   size_t start = getNum(executor, command, 1, "array-slice");
   size_t end = getNum(executor, command, 2, "array-slice");
   if (start < 0 || start >= array->size() || end < 0 || end > array->size() || start >= end) {
      error(executor.diagnostics, command.file, command.line, "array-slice: Invalid slice range %zu-%zu", start, end);
      return;
   }
   std::vector<Value> copy (array->begin() + start, array->begin() + end);
   storeArray(executor, command, copy, back(executor, command), "array-slice");
}

void builtinArrayShuffle(const Command &command, Executor &executor) {
   std::vector<Value> *array;
   if (!arrayOrError(command, executor, "array-shuffle", array)) return;
   std::shuffle(array->begin(), array->end(), RNG());
}

void builtinArraySort(const Command &command, Executor &executor) {
   std::vector<Value> *array;
   if (!arrayOrError(command, executor, "array-sort", array)) return;
   bool ascending = getBool(executor, command, 1);
   std::sort(array->begin(), array->end(), [&](const Value &a, const Value &b) {
      Comparison c = compareTwoValues(executor, a, b, command.file, command.line, false, "array-sort");
      return ascending ? c == COMPARISON_LESS : c == COMPARISON_GREATER;
   });
}

void builtinArrayCount(const Command &command, Executor &executor) {
   std::vector<Value> *array;
   if (!arrayOrError(command, executor, "array-count", array)) return;
   Value target = resolveVariable(executor, arg(executor, command, 1));
   size_t count = 0;
   for (Value &v : *array) count += valuesEqual(executor, command, v, target);
   storeNumber(executor, command, count, false, "array-count");
}

void builtinArrayReverse(const Command &command, Executor &executor) {
   std::vector<Value> *array;
   if (!arrayOrError(command, executor, "array-reverse", array)) return;
   std::reverse(array->begin(), array->end());
}

void builtinArrayFind(const Command &command, Executor &executor) {
   std::vector<Value> *array;
   if (!arrayOrError(command, executor, "array-find", array)) return;
   Value target = resolveVariable(executor, arg(executor, command, 1));
   for (size_t i = 0; i < array->size(); ++i) {
      if (valuesEqual(executor, command, (*array)[i], target)) {
         storeNumber(executor, command, i, false, "array-find");
         return;
      }
   }
   storeInRegister(executor, command, NULL_VALUE, "array-find");
}

void builtinArrayContains(const Command &command, Executor &executor) {
   std::vector<Value> *array;
   if (!arrayOrError(command, executor, "array-contains", array)) return;
   Value target = resolveVariable(executor, arg(executor, command, 1));
   for (size_t i = 0; i < array->size(); ++i) {
      if (valuesEqual(executor, command, (*array)[i], target)) {
         storeBoolean(executor, command, true, "array-contains");
         return;
      }
   }
   storeBoolean(executor, command, false, "array-contains");
}

void builtinArrayEraseAll(const Command &command, Executor &executor) {
   std::vector<Value> *array;
   if (!arrayOrError(command, executor, "array-erase-all", array)) return;
   Value target = resolveVariable(executor, arg(executor, command, 1));
   array->erase(std::remove_if(array->begin(), array->end(), [&](const Value &v){ return valuesEqual(executor, command, v, target); }), array->end());
}

void builtinArrayShallowCopy(const Command &command, Executor &executor) {
   std::vector<Value> *array;
   if (!arrayOrError(command, executor, "array-shallow-copy", array)) return;
   storeArray(executor, command, *array, back(executor, command), "array-shallow-copy");
}

void builtinArrayDeepCopy(const Command &command, Executor &executor) {
   std::vector<Value> *array;
   if (!arrayOrError(command, executor, "array-deep-copy", array)) return;
   storeArray(executor, command, deepCopy(command, executor, *array), back(executor, command), "array-deep-copy");
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
   jumpToLabel(executor, resolveVariable(executor, arg(executor, command, 0)), "goto", "1st", command.file, command.line, true);
}

void builtinJmp(const Command &command, Executor &executor) {
   jumpToLabel(executor, resolveVariable(executor, arg(executor, command, 1)), "jmp", "2nd", command.file, command.line, getBool(executor, command, 0));
}

void builtinJmpn(const Command &command, Executor &executor) {
   jumpToLabel(executor, resolveVariable(executor, arg(executor, command, 1)), "jmpn", "2nd", command.file, command.line, !getBool(executor, command, 0));
}

void builtinJmptable(const Command &command, Executor &executor) {
   Value value = resolveVariable(executor, arg(executor, command, 0));
   for (size_t i = 1; i < command.argCount; i += 2) {
      Value result = resolveVariable(executor, arg(executor, command, i));
      if (valuesEqual(executor, command, value, result)) {
         jumpToLabel(executor, resolveVariable(executor, arg(executor, command, i + 1)), "jmptable", "destination", command.file, command.line, true);
         return;
      }
   }
   if (command.argCount % 2 != 1) {
      jumpToLabel(executor, resolveVariable(executor, back(executor, command)), "jmptable", "destination", command.file, command.line, true);
   }
}

void builtinCall(const Command &command, Executor &executor) {
   Function &function = executor.functions[arg(executor, command, command.callee).function];
   call(executor, command, function, command.callee, command.callee, command.argCount - command.callee - 1);
}

void builtinFunccall(const Command &command, Executor &executor) {
   Value f = resolveVariable(executor, arg(executor, command, 0));
   if (f.type != VALUE_FUNCTION) {
      error(executor.diagnostics, command.file, command.line, "func-call: Expected function to call for the 1st argument, got %s instead", getValueName(f.type));
      return;
   }
   Function &function = executor.functions[f.function];
   size_t params = function.params.size();
   size_t args = command.argCount - 1;
   bool variadic = function.variadic;

   if ((!variadic && args != params) || (variadic && args < params)) {
      error(executor.diagnostics, command.file, command.line, "func-call: Called function expected %s%zu parameters, but received %zu arguments", (variadic ? ">" : ""), params, args);
      return;
   }
   Command copy = command;
   copy.argStart += 1;
   copy.argCount -= 1;
   call(executor, copy, function, -1, std::string::npos, args);
}

void builtinReturn(const Command &command, Executor &executor) {
   if (executor.stackTrace.size() <= 1) {
      executor.exitCalled = true;
      return;
   }
   Trace trace = executor.stackTrace.top();
   executor.pointer = trace.position;
   executor.returnCount = command.argCount;

   for (size_t i = 0; i < executor.returnCount; ++i) {
      Value value = resolveVariable(executor, arg(executor, command, i));
      executor.returnRegisters[i] = value;
   }
   executor.locals.resize(trace.localStart);
   executor.stackTrace.pop();

   // call shenanigans
   if (trace.callArgCount != std::string::npos) {
      if (executor.returnCount != trace.callArgCount) {
         warn(executor.diagnostics, command.file, command.line, "call: Expected %zu return values, but got %zu instead", trace.callArgCount, executor.returnCount);
      }

      size_t count = std::min(executor.returnCount, trace.callArgCount);
      for (size_t i = 0; i < count; ++i) {
         Value reg = executor.arguments[trace.callArgStart + i];
         storeInRegister(executor, command, reg, executor.returnRegisters[i], "call");
      }
   }
}

// error handling
void builtinAssert(const Command &command, Executor &executor) {
   if (!getBool(executor, command, 0)) {
      Value value = resolveVariable(executor, arg(executor, command, 1));
      if (value.type != VALUE_STRING && value.type != VALUE_CSTRING) {
         error(executor.diagnostics, command.file, command.line, "assert: Expected String as the 2nd argument, got %s instead", getValueName(value.type));
         return;
      }
      const char *msg = (value.type == VALUE_STRING ? getString(executor, value.string, command.file, command.line) : getLexeme(executor.cache, value.string)).c_str();
      error(executor.diagnostics, 0, 0, msg);
   }
}

void builtinWarn(const Command &command, Executor &executor) {
   Value value = resolveVariable(executor, arg(executor, command, 0));
   if (value.type != VALUE_STRING && value.type != VALUE_CSTRING) {
      error(executor.diagnostics, command.file, command.line, "warn: Expected String as the 1st argument, got %s instead", getValueName(value.type));
      return;
   }
   const char *msg = (value.type == VALUE_STRING ? getString(executor, value.string, command.file, command.line) : getLexeme(executor.cache, value.string)).c_str();
   warn(executor.diagnostics, 0, 0, msg);
}

void builtinError(const Command &command, Executor &executor) {
   Value value = resolveVariable(executor, arg(executor, command, 0));
   if (value.type != VALUE_STRING && value.type != VALUE_CSTRING) {
      error(executor.diagnostics, command.file, command.line, "error: Expected String as the 1st argument, got %s instead", getValueName(value.type));
      return;
   }
   const char *msg = (value.type == VALUE_STRING ? getString(executor, value.string, command.file, command.line) : getLexeme(executor.cache, value.string)).c_str();
   error(executor.diagnostics, 0, 0, msg);
}

void builtinExit(const Command &command, Executor &executor) {
   double code = getNum(executor, command, 0, "exit");
   exit(code);
}

void builtinStackdepth(const Command &command, Executor &executor) {
   storeNumber(executor, command, executor.stackTrace.size(), false, "stack-depth");
}

void builtinStackname(const Command &command, Executor &executor) {
   storeString(executor, command, getLexeme(executor.cache, executor.code[executor.stackTrace.top().position].lexeme), "stack-name");
}

void builtinStackline(const Command &command, Executor &executor) {
   storeNumber(executor, command, executor.code[executor.stackTrace.top().position].line, false, "stack-line");
}

void builtinStackfile(const Command &command, Executor &executor) {
   storeString(executor, command, getLexeme(executor.cache, executor.code[executor.stackTrace.top().position].file), "stack-line");
}

void builtinStacktrace(const Command &command, Executor &executor) {
   logStackTrace(executor, SEVERITY_NONE);
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
   case VALUE_ARRAY: string = "array"; break;
   case VALUE_FUNCTION: string = "function"; break;
   case VALUE_LABEL: string = "label"; break;
   case VALUE_COUNT: string = "null"; break;
   default:
      printf("PIL::builtinTypeof: Cannot get the type of value %s.\n", getValueName(type));
      exit(EXIT_FAILURE);
   }
   storeString(executor, command, string, "typeof");
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

void builtinIsarray(const Command &command, Executor &executor) {
   Value value = resolveVariable(executor, arg(executor, command, 0));
   storeBoolean(executor, command, value.type == VALUE_ARRAY, "is-array");
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
   case VALUE_STRING: case VALUE_CSTRING: {
      const std::string &str = (value.type == VALUE_STRING ? getString(executor, value.string, command.file, command.line) : getLexeme(executor.cache, value.string));
      try {
         size_t pos = 0;
         long result = std::stol(str, &pos);
         if (pos != str.size() || str.empty() || std::isspace(str.front())) integer.type = VALUE_COUNT;
         else integer.integer = result;
      }
      catch (...) { integer.type = VALUE_COUNT; }
      break;
   }
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
   case VALUE_STRING: case VALUE_CSTRING: {
      const std::string &str = (value.type == VALUE_STRING ? getString(executor, value.string, command.file, command.line) : getLexeme(executor.cache, value.string));
      try {
         size_t pos = 0;
         long result = std::stod(str, &pos);
         if (pos != str.size() || str.empty() || std::isspace(str.front())) floating.type = VALUE_COUNT;
         else floating.floating = result;
      }
      catch (...) { floating.type = VALUE_COUNT; }
      break;
   }
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
   Value string = resolveVariable(executor, arg(executor, command, 0));
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

void builtinValTable(const Command &command, Executor &executor) {
   Value value = resolveVariable(executor, arg(executor, command, 0));
   Value dest = arg(executor, command, 1);
   for (size_t i = 2; i < command.argCount; i += 2) {
      Value result = resolveVariable(executor, arg(executor, command, i));
      if (valuesEqual(executor, command, value, result)) {
         storeInRegister(executor, command, dest, resolveVariable(executor, arg(executor, command, i+1)), "valtable");
         return;
      }
   }
   Value defaultValue = (command.argCount % 2 != 0 ? resolveVariable(executor, back(executor, command)) : NULL_VALUE);
   storeInRegister(executor, command, dest, defaultValue, "valtable");
}

void builtinTableContains(const Command &command, Executor &executor) {
   Value value = resolveVariable(executor, arg(executor, command, 0));
   Value dest = arg(executor, command, 1);
   for (size_t i = 2; i < command.argCount; ++i) {
      Value result = resolveVariable(executor, arg(executor, command, i));
      if (valuesEqual(executor, command, value, result)) {
         Value value {VALUE_INTEGER};
         value.integer = 1;
         storeInRegister(executor, command, dest, value, "table-contains");
         return;
      }
   }
   Value returnValue {VALUE_INTEGER};
   returnValue.integer = 0;
   storeInRegister(executor, command, dest, returnValue, "table-contains");
}

void builtinVariadicSize(const Command &command, Executor &executor) {
   storeNumber(executor, command, executor.stackTrace.top().variadicCount, false, "variadic-size");
}

void builtinVariadicIdx(const Command &command, Executor &executor) {
   size_t id = getNum(executor, command, 0, "vararg-idx");
   Trace &trace = executor.stackTrace.top();
   if (id < 0 || id >= trace.variadicCount) {
      error(executor.diagnostics, command.file, command.line, "vararg-idx: Index %zu is out of bounds", id);
      return;
   }
   storeInRegister(executor, command, executor.locals[trace.localStart + trace.localCount + id], "vararg-idx");
}
