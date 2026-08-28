#include "builtin.hpp"
#include <cstdio>

void builtinAdd(const std::vector<Value> &args) {
   printf("Called builtinAdd.\n");
}

void builtinSub(const std::vector<Value> &args) {
   printf("Called builtinSub.\n");
}

void builtinPrint(const std::vector<Value> &args) {
   printf("Called builtinPrint.\n");
}
