#include "builtin.hpp"
#include "pil.hpp"

void builtinAdd(const Command &command, Executor &executor) {
   long result = 0;
   for (size_t i = 0; i < command.args.size() - 1; ++i) {
      const Value &arg = command.args[i];
      result += (arg.type == VALUE_REGISTER ? executor.registers[arg.reg].integer : arg.integer);
   }
   executor.registers[command.args.back().reg].integer = result;
}

void builtinSub(const Command &command, Executor &executor) {
   const Value &arg = command.args[0];
   long result = (arg.type == VALUE_REGISTER ? executor.registers[arg.reg].integer : arg.integer);

   for (size_t i = 1; i < command.args.size() - 1; ++i) {
      const Value &arg = command.args[i];
      result -= (arg.type == VALUE_REGISTER ? executor.registers[arg.reg].integer : arg.integer);
   }
   executor.registers[command.args.back().reg].integer = result;
}

void builtinPrint(const Command &command, Executor &executor) {
   for (size_t i = 0; i < command.args.size(); ++i) {
      const Value &arg = command.args[i];
      printf("%ld", (arg.type == VALUE_REGISTER ? executor.registers[arg.reg].integer : arg.integer));
   }
}

void builtinGoto(const Command &command, Executor &executor) {
   if (command.args[0].type != VALUE_IDENTIFIER || command.args[0].identifier >= executor.code.functions.size() || !executor.code.functions[command.args[0].identifier].init || executor.code.functions[command.args[0].identifier].type != LABEL) {
      error(executor.diagnostics, "goto: Expected label", command.file, command.line);
      return;
   }
   executor.pointer = executor.code.functions[command.args[0].identifier].label - 1;
}

void builtinReturn(const Command &command, Executor &executor) {
   if (executor.stack.empty()) {
      executor.exitCalled = true;
      return;
   }
   size_t back = executor.stack.top();
   executor.stack.pop();
   executor.pointer = back;
}
