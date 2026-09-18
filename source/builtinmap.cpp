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

void builtinMapErase(const Command &command, Executor &executor) {
   InternalPILMap *map;
   if (!mapOrError(command, executor, "map-erase", map)) return;
   Value key = resolveVariable(executor, arg(executor, command, 1));
   if (key.type == VALUE_ARRAY || key.type == VALUE_MAP) {
      error(executor.diagnostics, command.file, command.line, "map-erase: %s cannot be used as a key in a Map", getValueName(key.type));
      return;
   }
   map->erase(key);
}

void builtinMapSet(const Command &command, Executor &executor) {
   InternalPILMap *map;
   if (!mapOrError(command, executor, "map-set", map)) return;
   Value key = resolveVariable(executor, arg(executor, command, 1));
   Value val = resolveVariable(executor, arg(executor, command, 2));
   if (key.type == VALUE_ARRAY || key.type == VALUE_MAP) {
      error(executor.diagnostics, command.file, command.line, "map-set: %s cannot be used as a key in a Map", getValueName(key.type));
      return;
   }
   (*map)[key] = val;
}

void builtinMapAt(const Command &command, Executor &executor) {
   InternalPILMap *map;
   if (!mapOrError(command, executor, "map-at", map)) return;
   Value key = resolveVariable(executor, arg(executor, command, 1));
   if (key.type == VALUE_ARRAY || key.type == VALUE_MAP) {
      error(executor.diagnostics, command.file, command.line, "map-at: %s cannot be used as a key in a Map", getValueName(key.type));
      return;
   }
   Value val = NULL_VALUE;
   if (auto it = map->find(key); it != map->end()) {
      val = it->second;
   }
   storeInRegister(executor, command, val, "map-at");
}

void builtinMapContains(const Command &command, Executor &executor) {
   InternalPILMap *map;
   if (!mapOrError(command, executor, "map-contains", map)) return;
   Value key = resolveVariable(executor, arg(executor, command, 1));
   if (key.type == VALUE_ARRAY || key.type == VALUE_MAP) {
      error(executor.diagnostics, command.file, command.line, "map-contains: %s cannot be used as a key in a Map", getValueName(key.type));
      return;
   }
   storeBoolean(executor, command, map->find(key) != map->end(), "map-contains");
}

void builtinMapSize(const Command &command, Executor &executor) {
   InternalPILMap *map;
   if (!mapOrError(command, executor, "map-size", map)) return;
   storeNumber(executor, command, map->size(), false, "map-size");
}

void builtinMapEmpty(const Command &command, Executor &executor) {
   InternalPILMap *map;
   if (!mapOrError(command, executor, "map-empty", map)) return;
   storeBoolean(executor, command, map->empty(), "map-empty");
}

void builtinMapClear(const Command &command, Executor &executor) {
   InternalPILMap *map;
   if (!mapOrError(command, executor, "map-clear", map)) return;
   map->clear();
}

void builtinMapKeys(const Command &command, Executor &executor) {
   InternalPILMap *map;
   if (!mapOrError(command, executor, "map-keys", map)) return;
   std::vector<Value> keys;
   keys.reserve(map->size());
   for (auto &[key, _]: *map) keys.push_back(key);
   storeArray(executor, command, keys, back(executor, command), "map-keys");
}

void builtinMapValues(const Command &command, Executor &executor) {
   InternalPILMap *map;
   if (!mapOrError(command, executor, "map-values", map)) return;
   std::vector<Value> values;
   values.reserve(map->size());
   for (auto &[_, value]: *map) values.push_back(value);
   storeArray(executor, command, values, back(executor, command), "map-values");
}

void builtinMapMerge(const Command &command, Executor &executor) {
   InternalPILMap *map1, *map2;
   if (!mapOrError(command, executor, "map-merge", map1) || !mapOrError(command, executor, "map-merge", map2, 1)) return;
   InternalPILMap result {std::max(map1->size(), map2->size()), ValueHash{&executor}, ValueEqual{&executor}};
   for (auto &[key, value]: *map1) result[key] = value;
   for (auto &[key, value]: *map2) result[key] = value;
   storeMap(executor, command, result, back(executor, command), "map-merge");
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

void builtinMapFreeMarked(const Command &command, Executor &executor) {
   int mark = getNum(executor, command, 0, "map-free-marked");
   for (auto it = executor.maps.begin(); it != executor.maps.end();) {
      it = (it->second.mark == mark ? executor.maps.erase(it) : std::next(it));
   }
}

void builtinMapGetMarkedCount(const Command &command, Executor &executor) {
   size_t count = 0;
   int mark = getNum(executor, command, 0, "map-get-marked-count");
   for (auto &[_, map]: executor.maps) count += (map.mark == mark);
   storeNumber(executor, command, count, false, "map-get-marked-count");
}

void builtinMapGetMarked(const Command &command, Executor &executor) {
   std::vector<Value> maps;
   int mark = getNum(executor, command, 0, "map-get-marked");
   for (auto &[id, map]: executor.maps) {
      if (mark == map.mark) {
         maps.push_back(Value{.type = VALUE_MAP, .map = id});
      }
   }
   storeArray(executor, command, maps, back(executor, command), "map-get-marked");
}

void builtinMapAnyMarked(const Command &command, Executor &executor) {
   int mark = getNum(executor, command, 0, "map-any-marked");
   for (auto &[_, map]: executor.maps) {
      if (mark == map.mark) {
         storeBoolean(executor, command, true, "map-any-marked");
         return;
      }
   }
   storeBoolean(executor, command, false, "map-any-marked");
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
