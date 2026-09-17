#include "pil.hpp"

size_t ValueHash::operator () (Value v) const {
   switch (v.type) {
   case VALUE_INTEGER: return std::hash<double>{}((double)v.integer);
   case VALUE_FLOATING: return std::hash<double>{}(v.floating);
   case VALUE_CHARACTER: return std::hash<char>{}(v.character);
   case VALUE_STRING: return std::hash<std::string>{}(getString(*executor, v.string, 0, 0));
   case VALUE_CSTRING: return std::hash<std::string>{}(getLexeme(executor->cache, v.string));
   case VALUE_FUNCTION: return std::hash<size_t>{}(v.function) ^ 0x9E3779B9;
   case VALUE_LABEL: return std::hash<size_t>{}(v.label) ^ 0x517CC1B7;
   default: return 0;
   }
}

bool ValueEqual::operator () (Value a, Value b) const {
   if ((a.type == VALUE_INTEGER || a.type == VALUE_FLOATING) && (b.type == VALUE_INTEGER || b.type == VALUE_INTEGER)) {
      double x = (a.type == VALUE_INTEGER) ? (double)a.integer : a.floating;
      double y = (b.type == VALUE_INTEGER) ? (double)b.integer : b.floating;
      return x == y;
   }
   else if (a.type == VALUE_CHARACTER && b.type == VALUE_CHARACTER) {
      return a.character == b.character;
   }
   else if ((a.type == VALUE_STRING || a.type == VALUE_CSTRING) && (b.type == VALUE_STRING || b.type == VALUE_CSTRING)) {
      const std::string &as = (a.type == VALUE_STRING ? getString(*executor, a.string, 0, 0) : getLexeme(executor->cache, a.string));
      const std::string &bs = (b.type == VALUE_STRING ? getString(*executor, b.string, 0, 0) : getLexeme(executor->cache, b.string));
      return as == bs;
   }
   else if (a.type == VALUE_COUNT && b.type == VALUE_COUNT) {
      return true;
   }
   else if (a.type == VALUE_FUNCTION && b.type == VALUE_FUNCTION) {
      return a.function == b.function;
   }
   else if (a.type == VALUE_LABEL && b.type == VALUE_LABEL) {
      return a.label == b.label;
   }
   return false;
}

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
   return arrayID;
}

std::unordered_map<Value, Value, ValueHash, ValueEqual> &getMap(Executor &executor, size_t ID, size_t file, size_t line) {
   if (auto it = executor.maps.find(ID); it != executor.maps.end()) {
      return it->second.map;
   }
   error(executor.diagnostics, file, line, "Invalid map ID %zu. Use after free", ID);
   static std::unordered_map<Value, Value, ValueHash, ValueEqual> map;
   return map;
}

size_t allocateMap(Executor &executor, const std::unordered_map<Value, Value, ValueHash, ValueEqual> &map) {
   static size_t mapID = 0;
   mapID += 1;
   executor.maps[mapID].map = map;
   return mapID;
}
