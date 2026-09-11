#include "pil.hpp"

std::string &getString(Executor &executor, size_t ID, size_t file, size_t line) {
   if (auto it = executor.strings.find(ID); it != executor.strings.end()) {
      return it->second.string;
   }
   error(executor.diagnostics, file, line, "Invalid string ID %zu. Use after free", ID);
   static std::string temp;
   return temp;
}

size_t allocateString(Executor &executor, const std::string &string) {
   static size_t stringID = 0;
   stringID += 1;
   executor.strings[stringID].string = string;
   executor.strings[stringID].allocations = 0;
   return stringID;
}

std::vector<Value> &getArray(Executor &executor, size_t ID, size_t file, size_t line) {
   if (auto it = executor.arrays.find(ID); it != executor.arrays.end()) {
      return it->second.array;
   }
   error(executor.diagnostics, file, line, "Invalid array ID %zu. Use after free", ID);
   static std::vector<Value> temp;
   return temp;
}

size_t allocateArray(Executor &executor, const std::vector<Value> &array) {
   static size_t arrayID = 0;
   arrayID += 1;
   executor.arrays[arrayID].array = array;
   executor.arrays[arrayID].allocations = 0;
   return arrayID;
}
