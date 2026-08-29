#include "builtin.hpp"
#include "pil.hpp"

void builtinAdd(const Command &command, Diagnostics &diagnostics, Executor &executor) {
   long result = 0;
   for (size_t i = 0; i < command.args.size() - 1; ++i) {
      const Value &arg = command.args[i];
      result += (arg.type == VALUE_REGISTER ? executor.registers[arg.reg].integer : arg.integer);
   }
   executor.registers[command.args.back().reg].integer = result;
}

void builtinSub(const Command &command, Diagnostics &diagnostics, Executor &executor) {
   const Value &arg = command.args[0];
   long result = (arg.type == VALUE_REGISTER ? executor.registers[arg.reg].integer : arg.integer);

   for (size_t i = 1; i < command.args.size() - 1; ++i) {
      const Value &arg = command.args[i];
      result -= (arg.type == VALUE_REGISTER ? executor.registers[arg.reg].integer : arg.integer);
   }
   executor.registers[command.args.back().reg].integer = result;
}

void builtinPrint(const Command &command, Diagnostics &diagnostics, Executor &executor) {
   for (size_t i = 0; i < command.args.size(); ++i) {
      const Value &arg = command.args[i];
      printf("%ld", (arg.type == VALUE_REGISTER ? executor.registers[arg.reg].integer : arg.integer));
   }
}

void builtinReturn(const Command &command, Diagnostics &diagnostics, Executor &executor) {
   if (executor.stack.empty()) {
      executor.exitCalled = true;
      return;
   }
   size_t back = executor.stack.top();
   executor.stack.pop();
   executor.pointer = back;
}
