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

// string ops
void builtinStringNew(const Command &command, Executor &executor);
void builtinStringFmt(const Command &command, Executor &executor);

// array ops
void builtinArrayNew(const Command &command, Executor &executor);
void builtinArrayFill(const Command &command, Executor &executor);
void builtinArrayIota(const Command &command, Executor &executor);
void builtinArrayClear(const Command &command, Executor &executor);
void builtinArrayMemFree(const Command &command, Executor &executor);
void builtinArrayEmpty(const Command &command, Executor &executor);
void builtinArraySize(const Command &command, Executor &executor);
void builtinArrayCapacity(const Command &command, Executor &executor);
void builtinArrayReserve(const Command &command, Executor &executor);
void builtinArrayResize(const Command &command, Executor &executor);
void builtinArraySet(const Command &command, Executor &executor);
void builtinArrayIdx(const Command &command, Executor &executor);
void builtinArrayBack(const Command &command, Executor &executor);
void builtinArrayFront(const Command &command, Executor &executor);
void builtinArrayPush(const Command &command, Executor &executor);
void builtinArrayInsert(const Command &command, Executor &executor);
void builtinArrayPop(const Command &command, Executor &executor);
void builtinArrayErase(const Command &command, Executor &executor);
void builtinArrayFree(const Command &command, Executor &executor);
void builtinArrayDeepFree(const Command &command, Executor &executor);
void builtinArrayMark(const Command &command, Executor &executor);
void builtinArrayFreeMarked(const Command &command, Executor &executor);
void builtinArrayJoin(const Command &command, Executor &executor);
void builtinArrayConcat(const Command &command, Executor &executor);
void builtinArraySlice(const Command &command, Executor &executor);
void builtinArrayShuffle(const Command &command, Executor &executor);
void builtinArraySort(const Command &command, Executor &executor);
void builtinArrayCount(const Command &command, Executor &executor);
void builtinArrayReverse(const Command &command, Executor &executor);
void builtinArrayFind(const Command &command, Executor &executor);
void builtinArrayContains(const Command &command, Executor &executor);
void builtinArrayEraseAll(const Command &command, Executor &executor);
void builtinArrayShallowCopy(const Command &command, Executor &executor);
void builtinArrayDeepCopy(const Command &command, Executor &executor);

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

// error handling
void builtinAssert(const Command &command, Executor &executor);
void builtinWarn(const Command &command, Executor &executor);
void builtinError(const Command &command, Executor &executor);
void builtinExit(const Command &command, Executor &executor);
void builtinStackdepth(const Command &command, Executor &executor);
void builtinStackname(const Command &command, Executor &executor);
void builtinStackline(const Command &command, Executor &executor);
void builtinStackfile(const Command &command, Executor &executor);
void builtinStacktrace(const Command &command, Executor &executor);

// types
void builtinTypeof(const Command &command, Executor &executor);
void builtinIsnum(const Command &command, Executor &executor);
void builtinIsfloat(const Command &command, Executor &executor);
void builtinIsint(const Command &command, Executor &executor);
void builtinIschar(const Command &command, Executor &executor);
void builtinIsstring(const Command &command, Executor &executor);
void builtinIsarray(const Command &command, Executor &executor);
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

// def table
struct BuiltinDef {
   const char *name;
   NativeFunction fn;
   size_t params;
   bool variadic = false;
   bool reserved = false;
};

constexpr bool VARIADIC = true;
constexpr bool RESERVED = true;
constexpr BuiltinDef BUILTIN_DEFINITIONS[] = {
   // output
   {"print", builtinPrint, 1, VARIADIC},
   {"printn", builtinPrintn, 1, VARIADIC},
   {"printf", builtinPrintf, 1, VARIADIC},
   {"printfn", builtinPrintfn, 1, VARIADIC},
   {"read", builtinRead, 1},
   {"readline", builtinReadline, 1},
   {"readchar", builtinReadchar, 1},
   {"setecho", builtinSetecho, 1},

   // string ops
   {"string-new", builtinStringNew, 1, VARIADIC},
   {"string-fmt", builtinStringFmt, 2, VARIADIC},

   // array ops
   {"array-new", builtinArrayNew, 1, VARIADIC},
   {"array-fill", builtinArrayFill, 3},
   {"array-iota", builtinArrayIota, 3},
   {"array-clear", builtinArrayClear, 1},
   {"array-memfree", builtinArrayMemFree, 1},
   {"array-empty", builtinArrayEmpty, 2},
   {"array-size", builtinArraySize, 2},
   {"array-capacity", builtinArrayCapacity, 2},
   {"array-reserve", builtinArrayReserve, 2},
   {"array-resize", builtinArrayResize, 3},
   {"array-set", builtinArraySet, 3},
   {"array-idx", builtinArrayIdx, 3},
   {"array-back", builtinArrayBack, 2},
   {"array-front", builtinArrayFront, 2},
   {"array-push", builtinArrayPush, 2},
   {"array-insert", builtinArrayInsert, 3},
   {"array-pop", builtinArrayPop, 1},
   {"array-erase", builtinArrayErase, 2},
   {"array-free", builtinArrayFree, 1, VARIADIC},
   {"array-deep-free", builtinArrayDeepFree, 1, VARIADIC},
   {"array-mark", builtinArrayMark, 2},
   {"array-free-marked", builtinArrayFreeMarked, 1},
   {"array-join", builtinArrayJoin, 3},
   {"array-concat", builtinArrayConcat, 3},
   {"array-slice", builtinArraySlice, 4},
   {"array-shuffle", builtinArrayShuffle, 1},
   {"array-sort", builtinArraySort, 2},
   {"array-count", builtinArrayCount, 3},
   {"array-reverse", builtinArrayReverse, 1},
   {"array-find", builtinArrayFind, 3},
   {"array-contains", builtinArrayContains, 3},
   {"array-erase-all", builtinArrayEraseAll, 2},
   {"array-shallow-copy", builtinArrayShallowCopy, 2},
   {"array-deep-copy", builtinArrayDeepCopy, 2},

   // math
   {"incr", builtinIncr, 1},
   {"decr", builtinDecr, 1},
   {"add", builtinAdd, 3, VARIADIC},
   {"sub", builtinSub, 3, VARIADIC},
   {"mul", builtinMul, 3, VARIADIC},
   {"div", builtinDiv, 3, VARIADIC},
   {"mod", builtinMod, 3},
   {"pow", builtinPow, 3},
   {"neg", builtinNeg, 2},
   {"sqrt", builtinSqrt, 2},
   {"cbrt", builtinCbrt, 2},
   {"sin", builtinSin, 2},
   {"cos", builtinCos, 2},
   {"tan", builtinTan, 2},
   {"asin", builtinAsin, 2},
   {"acos", builtinAcos, 2},
   {"atan", builtinAtan, 2},
   {"atan2", builtinAtan2, 2},
   {"asinh", builtinAsinh, 2},
   {"acosh", builtinAcosh, 2},
   {"atanh", builtinAtanh, 2},
   {"sinh", builtinSinh, 2},
   {"cosh", builtinCosh, 2},
   {"tanh", builtinTanh, 2},
   {"abs", builtinAbs, 2},
   {"min", builtinMin, 3, VARIADIC},
   {"max", builtinMax, 3, VARIADIC},
   {"clamp", builtinClamp, 4},
   {"sign", builtinSign, 2},
   {"trunc", builtinTrunc, 2},
   {"ceil", builtinCeil, 2},
   {"floor", builtinFloor, 2},
   {"round", builtinRound, 2},
   {"exp", builtinExp, 2},
   {"ln", builtinLn, 2},
   {"log", builtinLog, 3},
   {"log2", builtinLog2, 2},
   {"log10", builtinLog10, 2},
   {"lerp", builtinLerp, 4},
   {"step-towards", builtinStepTowards, 3},

   // comparison
   {"le", builtinLe, 3},
   {"gr", builtinGr, 3},
   {"leeq", builtinLeeq, 3},
   {"greq", builtinGreq, 3},
   {"eq", builtinEq, 3},
   {"neq", builtinNeq, 3},
   {"and", builtinAnd, 3, VARIADIC},
   {"or", builtinOr, 3, VARIADIC},
   {"not", builtinNot, 2},

   // control flow
   {"goto", builtinGoto, 1},
   {"jmp", builtinJmp, 2},
   {"jmpn", builtinJmpn, 2},
   {"jmptable", builtinJmptable, 3, VARIADIC},
   {"call", builtinCall, 1, VARIADIC},
   {"func-call", builtinFunccall, 1, VARIADIC},
   {"return", builtinReturn, 0, VARIADIC, RESERVED},

   // error handling
   {"error", builtinError, 1},
   {"warn", builtinWarn, 1},
   {"assert", builtinAssert, 2},
   {"exit", builtinExit, 1},
   {"stack-depth", builtinStackdepth, 1},
   {"stack-name", builtinStackname, 1},
   {"stack-line", builtinStackline, 1},
   {"stack-file", builtinStackfile, 1},
   {"stack-trace", builtinStacktrace, 0},

   // types
   {"typeof", builtinTypeof, 2},
   {"is-num", builtinIsnum, 2},
   {"is-float", builtinIsfloat, 2},
   {"is-int", builtinIsint, 2},
   {"is-char", builtinIschar, 2},
   {"is-string", builtinIsstring, 2},
   {"is-array", builtinIsarray, 2},
   {"is-reg", builtinIsreg, 2},
   {"is-function", builtinIsfunction, 2},
   {"is-label", builtinIslabel, 2},
   {"is-null", builtinIsnull, 2},
   {"is-inf", builtinIsinf, 2},
   {"is-nan", builtinIsnan, 2},
   {"to-int", builtinToint, 2},
   {"to-float", builtinTofloat, 2},
   {"to-char", builtinTochar, 2},

   // misc. (time, random)
   {"time", builtinTime, 1},
   {"unix-time", builtinUnixTime, 1},
   {"date", builtinDate, 2},
   {"sleep", builtinSleep, 1},
   {"seed-random", builtinSeedRandom, 1},
   {"random", builtinRandom, 1},
   {"randf-range", builtinRandfRange, 3},
   {"randi-range", builtinRandiRange, 3},

   // variables
   {"swap", builtinSwap, 2},
   {"set", builtinSet, 2},
   {"valtable", builtinValTable, 4, VARIADIC},
   {"table-contains", builtinTableContains, 3, VARIADIC},
   {"variadic-size", builtinVariadicSize, 1},
   {"variadic-idx", builtinVariadicIdx, 2},
};
