#pragma once
#include "values.hpp"

// i/o
void builtinPrint(const Command &command, Executor &executor);
void builtinPrintn(const Command &command, Executor &executor);
void builtinPrintf(const Command &command, Executor &executor);
void builtinPrintfn(const Command &command, Executor &executor);
void builtinRead(const Command &command, Executor &executor);
void builtinReadline(const Command &command, Executor &executor);
void builtinReadchar(const Command &command, Executor &executor);
void builtinSetecho(const Command &command, Executor &executor);

// string
void builtinStringNew(const Command &command, Executor &executor);
void builtinStringFmt(const Command &command, Executor &executor);

// math
void builtinIncr(const Command &command, Executor &executor);
void builtinDecr(const Command &command, Executor &executor);
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
void builtinSign(const Command &command, Executor &executor);
void builtinTrunc(const Command &command, Executor &executor);
void builtinCeil(const Command &command, Executor &executor);
void builtinFloor(const Command &command, Executor &executor);
void builtinRound(const Command &command, Executor &executor);
void builtinExp(const Command &command, Executor &executor);
void builtinLn(const Command &command, Executor &executor);
void builtinLog(const Command &command, Executor &executor);
void builtinLog2(const Command &command, Executor &executor);
void builtinLog10(const Command &command, Executor &executor);
void builtinLerp(const Command &command, Executor &executor);
void builtinStepTowards(const Command &command, Executor &executor);

// comparison
void builtinLe(const Command &command, Executor &executor);
void builtinGr(const Command &command, Executor &executor);
void builtinLeeq(const Command &command, Executor &executor);
void builtinGreq(const Command &command, Executor &executor);
void builtinEq(const Command &command, Executor &executor);
void builtinNeq(const Command &command, Executor &executor);
void builtinAnd(const Command &command, Executor &executor);
void builtinOr(const Command &command, Executor &executor);
void builtinNot(const Command &command, Executor &executor);

// control flow
void builtinGoto(const Command &command, Executor &executor);
void builtinJmp(const Command &command, Executor &executor);
void builtinJmpn(const Command &command, Executor &executor);
void builtinJmptable(const Command &command, Executor &executor);
void builtinCall(const Command &command, Executor &executor);
void builtinFunccall(const Command &command, Executor &executor);
void builtinReturn(const Command &command, Executor &executor);

// types
void builtinTypeof(const Command &command, Executor &executor);
void builtinSizeof(const Command &command, Executor &executor);
void builtinIsnum(const Command &command, Executor &executor);
void builtinIsfloat(const Command &command, Executor &executor);
void builtinIsint(const Command &command, Executor &executor);
void builtinIschar(const Command &command, Executor &executor);
void builtinIsstring(const Command &command, Executor &executor);
void builtinIsreg(const Command &command, Executor &executor);
void builtinIsfunction(const Command &command, Executor &executor);
void builtinIslabel(const Command &command, Executor &executor);
void builtinIsnull(const Command &command, Executor &executor);
void builtinIsinf(const Command &command, Executor &executor);
void builtinIsnan(const Command &command, Executor &executor);
void builtinToint(const Command &command, Executor &executor);
void builtinTofloat(const Command &command, Executor &executor);
void builtinTochar(const Command &command, Executor &executor);

// misc. (time, random)
void builtinTime(const Command &command, Executor &executor);
void builtinUnixTime(const Command &command, Executor &executor);
void builtinDate(const Command &command, Executor &executor);
void builtinSleep(const Command &command, Executor &executor);
void builtinSeedRandom(const Command &command, Executor &executor);
void builtinRandom(const Command &command, Executor &executor);
void builtinRandfRange(const Command &command, Executor &executor);
void builtinRandiRange(const Command &command, Executor &executor);

// variables/registers/values
void builtinSwap(const Command &command, Executor &executor);
void builtinSet(const Command &command, Executor &executor);
void builtinValTable(const Command &command, Executor &executor);
void builtinTableContains(const Command &command, Executor &executor);
void builtinVariadicSize(const Command &command, Executor &executor);
void builtinVariadicIdx(const Command &command, Executor &executor);
