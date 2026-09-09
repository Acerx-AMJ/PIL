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
