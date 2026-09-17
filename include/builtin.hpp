#pragma once
#include "values.hpp"

// string ops
void builtinStringNew(const Command &command, Executor &executor);
void builtinStringFmt(const Command &command, Executor &executor);
void builtinStringRepeat(const Command &command, Executor &executor);
void builtinStringClear(const Command &command, Executor &executor);
void builtinStringMemFree(const Command &command, Executor &executor);
void builtinStringEmpty(const Command &command, Executor &executor);
void builtinStringSize(const Command &command, Executor &executor);
void builtinStringCapacity(const Command &command, Executor &executor);
void builtinStringReserve(const Command &command, Executor &executor);
void builtinStringResize(const Command &command, Executor &executor);
void builtinStringSet(const Command &command, Executor &executor);
void builtinStringIdx(const Command &command, Executor &executor);
void builtinStringBack(const Command &command, Executor &executor);
void builtinStringFront(const Command &command, Executor &executor);
void builtinStringPush(const Command &command, Executor &executor);
void builtinStringInsert(const Command &command, Executor &executor);
void builtinStringPop(const Command &command, Executor &executor);
void builtinStringErase(const Command &command, Executor &executor);
void builtinStringFree(const Command &command, Executor &executor);
void builtinStringMark(const Command &command, Executor &executor);
void builtinStringGetMark(const Command &command, Executor &executor);
void builtinStringFreeMarked(const Command &command, Executor &executor);
void builtinStringSplit(const Command &command, Executor &executor);
void builtinStringConcat(const Command &command, Executor &executor);
void builtinStringSubstr(const Command &command, Executor &executor);
void builtinStringCount(const Command &command, Executor &executor);
void builtinStringReverse(const Command &command, Executor &executor);
void builtinStringFind(const Command &command, Executor &executor);
void builtinStringRfind(const Command &command, Executor &executor);
void builtinStringFindFirstOf(const Command &command, Executor &executor);
void builtinStringFindFirstNotOf(const Command &command, Executor &executor);
void builtinStringFindLastOf(const Command &command, Executor &executor);
void builtinStringFindLastNotOf(const Command &command, Executor &executor);
void builtinStringReplace(const Command &command, Executor &executor);
void builtinStringReplaceAll(const Command &command, Executor &executor);
void builtinStringContains(const Command &command, Executor &executor);
void builtinStringEraseAll(const Command &command, Executor &executor);
void builtinStringStartsWith(const Command &command, Executor &executor);
void builtinStringEndsWith(const Command &command, Executor &executor);
void builtinStringTrim(const Command &command, Executor &executor);
void builtinStringTolower(const Command &command, Executor &executor);
void builtinStringToupper(const Command &command, Executor &executor);
void builtinStringCopy(const Command &command, Executor &executor);

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
void builtinArrayGetMark(const Command &command, Executor &executor);
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

// map ops
void builtinMapNew(const Command &command, Executor &executor);
void builtinMapFree(const Command &command, Executor &executor);
void builtinMapDeepFree(const Command &command, Executor &executor);
void builtinMapMark(const Command &command, Executor &executor);
void builtinMapGetMark(const Command &command, Executor &executor);
void builtinMapFreeMark(const Command &command, Executor &executor);
void builtinMapShallowCopy(const Command &command, Executor &executor);
void builtinMapDeepCopy(const Command &command, Executor &executor);

// math
void builtinIncr(const Command &command, Executor &executor);
void builtinDecr(const Command &command, Executor &executor);
void builtinAdd(const Command &command, Executor &executor);
void builtinSub(const Command &command, Executor &executor);
void builtinMul(const Command &command, Executor &executor);
void builtinDiv(const Command &command, Executor &executor);
void builtinMod(const Command &command, Executor &executor);
void builtinFloorMod(const Command &command, Executor &executor);
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
void builtinSeedRandom(const Command &command, Executor &executor);
void builtinRandom(const Command &command, Executor &executor);
void builtinRandfRange(const Command &command, Executor &executor);
void builtinRandiRange(const Command &command, Executor &executor);
void builtinGcd(const Command &command, Executor &executor);
void builtinLcm(const Command &command, Executor &executor);
void builtinHypot(const Command &command, Executor &executor);
void builtinHypot3(const Command &command, Executor &executor);
void builtinBitand(const Command &command, Executor &executor);
void builtinBitor(const Command &command, Executor &executor);
void builtinBitxor(const Command &command, Executor &executor);
void builtinBitnot(const Command &command, Executor &executor);
void builtinBitshl(const Command &command, Executor &executor);
void builtinBitshr(const Command &command, Executor &executor);
void builtinBitcount(const Command &command, Executor &executor);
void builtinBittest(const Command &command, Executor &executor);
void builtinBitset(const Command &command, Executor &executor);
void builtinBitclear(const Command &command, Executor &executor);
void builtinBittoggle(const Command &command, Executor &executor);

// input/output
void builtinPrintch(const Command &command, Executor &executor);
void builtinPrint(const Command &command, Executor &executor);
void builtinPrintln(const Command &command, Executor &executor);
void builtinPrintf(const Command &command, Executor &executor);
void builtinPrintfln(const Command &command, Executor &executor);
void builtinRead(const Command &command, Executor &executor);
void builtinReadln(const Command &command, Executor &executor);
void builtinReadch(const Command &command, Executor &executor);
void builtinSetecho(const Command &command, Executor &executor);

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
void builtinCatch(const Command &command, Executor &executor);
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

// misc.
void builtinTime(const Command &command, Executor &executor);
void builtinUnixTime(const Command &command, Executor &executor);
void builtinDate(const Command &command, Executor &executor);
void builtinSleep(const Command &command, Executor &executor);
void builtinSwap(const Command &command, Executor &executor);
void builtinSet(const Command &command, Executor &executor);
void builtinValTable(const Command &command, Executor &executor);
void builtinTableContains(const Command &command, Executor &executor);
void builtinVariadicSize(const Command &command, Executor &executor);
void builtinVariadicIdx(const Command &command, Executor &executor);
void builtinRegSize(const Command &command, Executor &executor);
void builtinRegIdx(const Command &command, Executor &executor);
void builtinRegSet(const Command &command, Executor &executor);
void builtinReturnRegSize(const Command &command, Executor &executor);
void builtinReturnRegIdx(const Command &command, Executor &executor);
void builtinReturnRegSet(const Command &command, Executor &executor);
void builtinReturnCount(const Command &command, Executor &executor);
void builtinFuncArity(const Command &command, Executor &executor);
void builtinFuncVariadic(const Command &command, Executor &executor);
void builtinFuncArgMatch(const Command &command, Executor &executor);

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
   // string ops
   {"string-new", builtinStringNew, 1, VARIADIC},
   {"string-fmt", builtinStringFmt, 2, VARIADIC},
   {"string-repeat", builtinStringRepeat, 3},
   {"string-clear", builtinStringClear, 1},
   {"string-memfree", builtinStringMemFree, 1},
   {"string-empty", builtinStringEmpty, 2},
   {"string-size", builtinStringSize, 2},
   {"string-capacity", builtinStringCapacity, 2},
   {"string-reserve", builtinStringReserve, 2},
   {"string-resize", builtinStringResize, 3},
   {"string-set", builtinStringSet, 3},
   {"string-idx", builtinStringIdx, 3},
   {"string-back", builtinStringBack, 2},
   {"string-front", builtinStringFront, 2},
   {"string-push", builtinStringPush, 2},
   {"string-insert", builtinStringInsert, 3},
   {"string-pop", builtinStringPop, 1},
   {"string-erase", builtinStringErase, 2},
   {"string-free", builtinStringFree, 1, VARIADIC},
   {"string-mark", builtinStringMark, 2},
   {"string-get-mark", builtinStringGetMark, 2},
   {"string-free-marked", builtinStringFreeMarked, 1},
   {"string-split", builtinStringSplit, 3},
   {"string-concat", builtinStringConcat, 2, VARIADIC},
   {"string-substr", builtinStringSubstr, 4},
   {"string-count", builtinStringCount, 3},
   {"string-reverse", builtinStringReverse, 1},
   {"string-find", builtinStringFind, 4},
   {"string-rfind", builtinStringRfind, 4},
   {"string-find-first-of", builtinStringFindFirstOf, 3},
   {"string-find-first-not-of", builtinStringFindFirstNotOf, 3},
   {"string-find-last-of", builtinStringFindLastOf, 3},
   {"string-find-last-not-of", builtinStringFindLastNotOf, 3},
   {"string-replace", builtinStringReplace, 4},
   {"string-replace-all", builtinStringReplaceAll, 3},
   {"string-contains", builtinStringContains, 3},
   {"string-erase-all", builtinStringEraseAll, 2},
   {"string-starts-with", builtinStringStartsWith, 3},
   {"string-ends-with", builtinStringEndsWith, 3},
   {"string-trim", builtinStringTrim, 1},
   {"string-to-lower", builtinStringTolower, 1},
   {"string-to-upper", builtinStringToupper, 1},
   {"string-copy", builtinStringCopy, 2},

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
   {"array-get-mark", builtinArrayGetMark, 2},
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

   // map ops
   {"map-new", builtinMapNew, 1, VARIADIC},
   {"map-free", builtinMapFree, 1, VARIADIC},
   {"map-deep-free", builtinMapDeepFree, 1, VARIADIC},
   {"map-mark", builtinMapMark, 2},
   {"map-get-mark", builtinMapGetMark, 2},
   {"map-free-marked", builtinMapFreeMark, 1},
   {"map-shallow-copy", builtinMapShallowCopy, 2},
   {"map-deep-copy", builtinMapDeepCopy, 2},

   // math
   {"incr", builtinIncr, 1},
   {"decr", builtinDecr, 1},
   {"add", builtinAdd, 3, VARIADIC},
   {"sub", builtinSub, 3, VARIADIC},
   {"mul", builtinMul, 3, VARIADIC},
   {"div", builtinDiv, 3, VARIADIC},
   {"mod", builtinMod, 3},
   {"floor-mod", builtinFloorMod, 3},
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
   {"seed-random", builtinSeedRandom, 1},
   {"random", builtinRandom, 1},
   {"randf-range", builtinRandfRange, 3},
   {"randi-range", builtinRandiRange, 3},
   {"gcd", builtinGcd, 3},
   {"lcm", builtinLcm, 3},
   {"hypot", builtinHypot, 3},
   {"hypot3", builtinHypot3, 4},
   {"bit-and", builtinBitand, 3},
   {"bit-or", builtinBitor, 3},
   {"bit-xor", builtinBitxor, 3},
   {"bit-not", builtinBitnot, 2},
   {"bit-shl", builtinBitshl, 3},
   {"bit-shr", builtinBitshr, 3},
   {"bit-count", builtinBitcount, 2},
   {"bit-test", builtinBittest, 3},
   {"bit-set", builtinBitset, 3},
   {"bit-clear", builtinBitclear, 3},
   {"bit-toggle", builtinBittoggle, 3},

   // input/output
   {"printch", builtinPrintch, 1},
   {"print", builtinPrint, 1, VARIADIC},
   {"println", builtinPrintln, 1, VARIADIC},
   {"printf", builtinPrintf, 1, VARIADIC},
   {"printfln", builtinPrintfln, 1, VARIADIC},
   {"read", builtinRead, 1},
   {"readln", builtinReadln, 1},
   {"readch", builtinReadch, 1},
   {"setecho", builtinSetecho, 1},

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
   {"catch", builtinCatch, 2, VARIADIC},
   {"assert", builtinAssert, 2, VARIADIC},
   {"warn", builtinWarn, 1, VARIADIC},
   {"error", builtinError, 1, VARIADIC},
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

   // misc.
   {"time", builtinTime, 1},
   {"unix-time", builtinUnixTime, 1},
   {"date", builtinDate, 2},
   {"sleep", builtinSleep, 1},
   {"swap", builtinSwap, 2},
   {"set", builtinSet, 2},
   {"valtable", builtinValTable, 4, VARIADIC},
   {"table-contains", builtinTableContains, 3, VARIADIC},
   {"variadic-size", builtinVariadicSize, 1},
   {"variadic-idx", builtinVariadicIdx, 2},
   {"reg-size", builtinRegSize, 1},
   {"reg-idx", builtinRegIdx, 2},
   {"reg-set", builtinRegSet, 2},
   {"return-reg-size", builtinReturnRegSize, 1},
   {"return-reg-idx", builtinReturnRegIdx, 2},
   {"return-reg-set", builtinReturnRegSet, 2},
   {"return-count", builtinReturnCount, 1},
   {"func-arity", builtinFuncArity, 2},
   {"func-variadic", builtinFuncVariadic, 2},
   {"func-arg-match", builtinFuncArgMatch, 3},
};
