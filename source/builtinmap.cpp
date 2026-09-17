#include "builtin.hpp"
#include "builtinhelpers.hpp"

void builtinMapNew(const Command &command, Executor &executor) {
   if (command.argCount % 2 != 1) {
      error(executor.diagnostics, command.file, command.line, "map-new: Expected odd number of arguments");
      return;
   }
   InternalPILMap map ((command.argCount-1) / 2, ValueHash{&executor}, ValueEqual{&executor});
   for (size_t i = 1; i < command.argCount; i += 2) {
      Value key = resolveVariable(executor, arg(executor, command, i));
      Value value = resolveVariable(executor, arg(executor, command, i + 1));
      if (key.type == VALUE_ARRAY || key.type == VALUE_MAP) {
         error(executor.diagnostics, command.file, command.line, "map-new: %s cannot be used as a key in a Map", getValueName(key.type));
         return;
      }
      map[key] = value;
   }
   storeMap(executor, command, map, arg(executor, command, 0), "map-new");
}

void builtinMapFree(const Command &command, Executor &executor) {
   for (size_t i = 0; i < command.argCount; ++i) {
      Value a = arg(executor, command, i);
      Value &map = resolveVariableByRef(executor, a);
      if (map.type != VALUE_MAP) {
         error(executor.diagnostics, command.file, command.line, "map-free: Expected Map, got %s instead", getValueName(map.type));
         return;
      }
      executor.maps.erase(map.map);
      map = NULL_VALUE;
   }
}

void builtinMapDeepFree(const Command &command, Executor &executor) {
   std::set<std::pair<size_t, ValueType>> visited;
   for (size_t i = 0; i < command.argCount; ++i) {
      Value a = arg(executor, command, i);
      Value &map = resolveVariableByRef(executor, a);
      if (map.type != VALUE_MAP) {
         error(executor.diagnostics, command.file, command.line, "map-deep-free: Expected Map, got %s instead", getValueName(map.type));
         return;
      }
      visited.clear();
      deepFree(command, executor, map, visited);
   }
}

void builtinMapMark(const Command &command, Executor &executor) {
   Value map = resolveVariable(executor, arg(executor, command, 0));
   if (map.type != VALUE_MAP) {
      error(executor.diagnostics, command.file, command.line, "map-mark: Expected Map, got %s instead", getValueName(map.type));
      return;
   }
   auto it = executor.maps.find(map.map);
   if (it == executor.maps.end()) {
      error(executor.diagnostics, command.file, command.line, "Invalid Map ID %zu. Use after free", map.map);
      return;
   }
   it->second.mark = getNum(executor, command, 1, "map-mark");
}

void builtinMapGetMark(const Command &command, Executor &executor) {
   Value map = resolveVariable(executor, arg(executor, command, 0));
   if (map.type != VALUE_MAP) {
      error(executor.diagnostics, command.file, command.line, "map-get-mark: Expected Map, got %s instead", getValueName(map.type));
      return;
   }
   auto it = executor.maps.find(map.map);
   if (it == executor.maps.end()) {
      error(executor.diagnostics, command.file, command.line, "Invalid Map ID %zu. Use after free", map.map);
      return;
   }
   storeNumber(executor, command, it->second.mark, false, "map-get-mark");
}

void builtinMapFreeMark(const Command &command, Executor &executor) {
   int mark = getNum(executor, command, 0, "map-free-marked");
   for (auto it = executor.maps.begin(); it != executor.maps.end();) {
      it = (it->second.mark == mark ? executor.maps.erase(it) : std::next(it));
   }
}

void builtinMapShallowCopy(const Command &command, Executor &executor) {
   InternalPILMap *map;
   if (!mapOrError(command, executor, "map-shallow-copy", map)) return;
   storeMap(executor, command, *map, back(executor, command), "map-shallow-copy");
}

void builtinMapDeepCopy(const Command &command, Executor &executor) {
   Value map = resolveVariable(executor, arg(executor, command, 0));
   if (map.type != VALUE_MAP) {
      error(executor.diagnostics, command.file, command.line, "map-deep-copy: Expected Map, got %s instead", getValueName(map.type));
      return;
   }
   if (auto it = executor.maps.find(map.map); it == executor.maps.end()) {
      error(executor.diagnostics, command.file, command.line, "Invalid Map ID %zu. Use after free", map.map);
      return;
   }
   std::map<std::pair<size_t, ValueType>, size_t> copied;
   Value result {VALUE_MAP};
   result.map = deepCopy(command, executor, map.map, map.type, copied);
   storeInRegister(executor, command, back(executor, command), result, "map-deep-copy");
}
