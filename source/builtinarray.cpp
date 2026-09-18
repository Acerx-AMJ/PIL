#include "builtin.hpp"
#include "builtinhelpers.hpp"

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

void builtinArrayAt(const Command &command, Executor &executor) {
   std::vector<Value> *array;
   if (!arrayOrError(command, executor, "array-at", array)) return;
   size_t id = getNum(executor, command, 1, "array-at");
   if (id < 0 || id >= array->size()) {
      error(executor.diagnostics, command.file, command.line, "array-at: Index %zu is out of bounds", id);
      return;
   }
   storeInRegister(executor, command, (*array)[id], "array-at");
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
         error(executor.diagnostics, command.file, command.line, "array-free: Expected Array, got %s instead", getValueName(array.type));
         return;
      }
      executor.arrays.erase(array.array);
      array = NULL_VALUE;
   }
}

void builtinArrayDeepFree(const Command &command, Executor &executor) {
   std::set<std::pair<size_t, ValueType>> visited;
   for (size_t i = 0; i < command.argCount; ++i) {
      Value a = arg(executor, command, i);
      Value &array = resolveVariableByRef(executor, a);
      if (array.type != VALUE_ARRAY) {
         error(executor.diagnostics, command.file, command.line, "array-deep-free: Expected Array, got %s instead", getValueName(array.type));
         return;
      }
      visited.clear();
      deepFree(command, executor, array, visited);
   }
}

void builtinArrayMark(const Command &command, Executor &executor) {
   Value array = resolveVariable(executor, arg(executor, command, 0));
   if (array.type != VALUE_ARRAY) {
      error(executor.diagnostics, command.file, command.line, "array-mark: Expected Array, got %s instead", getValueName(array.type));
      return;
   }
   auto it = executor.arrays.find(array.array);
   if (it == executor.arrays.end()) {
      error(executor.diagnostics, command.file, command.line, "Invalid array ID %zu. Use after free", array.array);
      return;
   }
   it->second.mark = getNum(executor, command, 1, "array-mark");
}

void builtinArrayGetMark(const Command &command, Executor &executor) {
   Value array = resolveVariable(executor, arg(executor, command, 0));
   if (array.type != VALUE_ARRAY) {
      error(executor.diagnostics, command.file, command.line, "array-get-mark: Expected Array, got %s instead", getValueName(array.type));
      return;
   }
   auto it = executor.arrays.find(array.array);
   if (it == executor.arrays.end()) {
      error(executor.diagnostics, command.file, command.line, "Invalid array ID %zu. Use after free", array.array);
      return;
   }
   storeNumber(executor, command, it->second.mark, false, "array-get-mark");
}

void builtinArrayFreeMarked(const Command &command, Executor &executor) {
   int mark = getNum(executor, command, 0, "array-free-marked");
   for (auto it = executor.arrays.begin(); it != executor.arrays.end();) {
      it = (it->second.mark == mark ? executor.arrays.erase(it) : std::next(it));
   }
}

void builtinArrayGetMarkedCount(const Command &command, Executor &executor) {
   size_t count = 0;
   int mark = getNum(executor, command, 0, "array-get-marked-count");
   for (auto &[_, array]: executor.arrays) count += (array.mark == mark);
   storeNumber(executor, command, count, false, "array-get-marked-count");
}

void builtinArrayGetMarked(const Command &command, Executor &executor) {
   std::vector<Value> arrays;
   int mark = getNum(executor, command, 0, "array-get-marked");
   for (auto &[id, array]: executor.arrays) {
      if (mark == array.mark) {
         arrays.push_back(Value{.type = VALUE_ARRAY, .array = id});
      }
   }
   storeArray(executor, command, arrays, back(executor, command), "array-get-marked");
}

void builtinArrayAnyMarked(const Command &command, Executor &executor) {
   int mark = getNum(executor, command, 0, "array-any-marked");
   for (auto &[_, array]: executor.arrays) {
      if (mark == array.mark) {
         storeBoolean(executor, command, true, "array-any-marked");
         return;
      }
   }
   storeBoolean(executor, command, false, "array-any-marked");
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
   storeString(executor, command, result, back(executor, command), "array-join");
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
   for (Value &v : *array) count += valuesEqual(executor, command, v, target, "array-count");
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
      if (valuesEqual(executor, command, (*array)[i], target, "array-find")) {
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
      if (valuesEqual(executor, command, (*array)[i], target, "array-contains")) {
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
   array->erase(std::remove_if(array->begin(), array->end(), [&](const Value &v){ return valuesEqual(executor, command, v, target, "array-erase-all"); }), array->end());
}

void builtinArrayShallowCopy(const Command &command, Executor &executor) {
   std::vector<Value> *array;
   if (!arrayOrError(command, executor, "array-shallow-copy", array)) return;
   storeArray(executor, command, *array, back(executor, command), "array-shallow-copy");
}

void builtinArrayDeepCopy(const Command &command, Executor &executor) {
   Value array = resolveVariable(executor, arg(executor, command, 0));
   if (array.type != VALUE_ARRAY) {
      error(executor.diagnostics, command.file, command.line, "array-deep-copy: Expected Array, got %s instead", getValueName(array.type));
      return;
   }
   if (auto it = executor.arrays.find(array.array); it == executor.arrays.end()) {
      error(executor.diagnostics, command.file, command.line, "Invalid Array ID %zu. Use after free", array.array);
      return;
   }
   std::map<std::pair<size_t, ValueType>, size_t> copied;
   Value result {VALUE_ARRAY};
   result.array = deepCopy(command, executor, array.array, array.type, copied);
   storeInRegister(executor, command, back(executor, command), result, "array-deep-copy");
}
