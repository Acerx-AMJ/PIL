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
