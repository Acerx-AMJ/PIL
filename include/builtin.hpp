#pragma once

struct Command;
struct Executor;

void builtinAdd(const Command &command, Executor &executor);
void builtinSub(const Command &command, Executor &executor);
void builtinPrint(const Command &command, Executor &executor);

void builtinGoto(const Command &command, Executor &executor);
void builtinReturn(const Command &command, Executor &executor);
