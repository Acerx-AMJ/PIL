#pragma once

struct Command;
struct Executor;

// misc, temp
void builtinMove(const Command &command, Executor &executor);
void builtinAdd(const Command &command, Executor &executor);
void builtinPrint(const Command &command, Executor &executor);

// comparison
void builtinLe(const Command &command, Executor &executor);
void builtinGr(const Command &command, Executor &executor);
void builtinLeeq(const Command &command, Executor &executor);
void builtinGreq(const Command &command, Executor &executor);
void builtinEq(const Command &command, Executor &executor);
void builtinNeq(const Command &command, Executor &executor);
void builtinNot(const Command &command, Executor &executor);

// control flow
void builtinGoto(const Command &command, Executor &executor);
void builtinJmp(const Command &command, Executor &executor);
void builtinJmpn(const Command &command, Executor &executor);
void builtinCall(const Command &command, Executor &executor);
void builtinReturn(const Command &command, Executor &executor);
