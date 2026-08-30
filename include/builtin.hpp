#pragma once
#include "values.hpp"

Value resolveVariable(Executor &executor, Value value, const char *function);

// output
void builtinPrint(const Command &command, Executor &executor);
void builtinPrintn(const Command &command, Executor &executor);
void builtinPrintf(const Command &command, Executor &executor);
void builtinPrintfn(const Command &command, Executor &executor);
void builtinStr(const Command &command, Executor &executor);
void builtinFormat(const Command &command, Executor &executor);

// math
void builtinAdd(const Command &command, Executor &executor);
void builtinSub(const Command &command, Executor &executor);
void builtinMul(const Command &command, Executor &executor);
void builtinDiv(const Command &command, Executor &executor);
void builtinMod(const Command &command, Executor &executor);
void builtinPow(const Command &command, Executor &executor);
void builtinNeg(const Command &command, Executor &executor);
void builtinSqrt(const Command &command, Executor &executor);
void builtinCbrt(const Command &command, Executor &executor);
void builtinSin(const Command &command, Executor &executor);
void builtinCos(const Command &command, Executor &executor);
void builtinTan(const Command &command, Executor &executor);
void builtinAsin(const Command &command, Executor &executor);
void builtinAcos(const Command &command, Executor &executor);
void builtinAtan(const Command &command, Executor &executor);
void builtinAtan2(const Command &command, Executor &executor);
void builtinAsinh(const Command &command, Executor &executor);
void builtinAcosh(const Command &command, Executor &executor);
void builtinAtanh(const Command &command, Executor &executor);
void builtinSinh(const Command &command, Executor &executor);
void builtinCosh(const Command &command, Executor &executor);
void builtinTanh(const Command &command, Executor &executor);
void builtinAbs(const Command &command, Executor &executor);
void builtinMin(const Command &command, Executor &executor);
void builtinMax(const Command &command, Executor &executor);
void builtinClamp(const Command &command, Executor &executor);
void builtinCeil(const Command &command, Executor &executor);
void builtinFloor(const Command &command, Executor &executor);
void builtinRound(const Command &command, Executor &executor);
void builtinExp(const Command &command, Executor &executor);
void builtinLn(const Command &command, Executor &executor);
void builtinLog(const Command &command, Executor &executor);
void builtinLog2(const Command &command, Executor &executor);
void builtinLog10(const Command &command, Executor &executor);

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

// variables
void builtinSet(const Command &command, Executor &executor);
void builtinMove(const Command &command, Executor &executor);
void builtinGlobal(const Command &command, Executor &executor);
