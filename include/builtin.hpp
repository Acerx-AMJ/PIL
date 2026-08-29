#pragma once

struct Command;
struct Diagnostics;
struct Executor;

void builtinAdd(const Command &command, Diagnostics &diagnostics, Executor &executor);
void builtinSub(const Command &command, Diagnostics &diagnostics, Executor &executor);
void builtinPrint(const Command &command, Diagnostics &diagnostics, Executor &executor);
void builtinReturn(const Command &command, Diagnostics &diagnostics, Executor &executor);
