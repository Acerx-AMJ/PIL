#include "builtin.hpp"
#include "builtinhelpers.hpp"

// helpers
inline void unaryBuiltin(Executor &executor, const Command &command, double(*fn)(double), const char *function) {
   storeNumber(executor, command, fn(getNum(executor, command, 0, function)), true, function);
}

inline void binaryBuiltin(Executor &executor, const Command &command, double(*fn)(double, double), const char *function) {
   storeNumber(executor, command, fn(getNum(executor, command, 0, function), getNum(executor, command, 1, function)), true, function);
}

// math
void builtinIncr(const Command &command, Executor &executor) {
   bool floating = false;
   double number = getNum(executor, command, 0, "incr");
   storeNumber(executor, command, number + 1, floating, "incr");
}

void builtinDecr(const Command &command, Executor &executor) {
   bool floating = false;
   double number = getNum(executor, command, 0, "decr");
   storeNumber(executor, command, number - 1, floating, "decr");
}

void builtinAdd(const Command &command, Executor &executor) {
   bool floating = false;
   double number = 0.0;
   for (size_t i = 0; i < command.argCount - 1; ++i) {
      number += getNum(executor, command, i, "add", &floating);
   }
   storeNumber(executor, command, number, floating, "add");
}

void builtinSub(const Command &command, Executor &executor) {
   bool floating = false;
   double number = getNum(executor, command, 0, "sub", &floating);
   for (size_t i = 1; i < command.argCount - 1; ++i) {
      number -= getNum(executor, command, i, "sub", &floating);
   }
   storeNumber(executor, command, number, floating, "sub");
}

void builtinMul(const Command &command, Executor &executor) {
   bool floating = false;
   double number = 1.0;
   for (size_t i = 0; i < command.argCount - 1; ++i) {
      number *= getNum(executor, command, i, "mul", &floating);
   }
   storeNumber(executor, command, number, floating, "mul");
}

void builtinDiv(const Command &command, Executor &executor) {
   bool floating = false;
   double number = getNum(executor, command, 0, "div", &floating);
   for (size_t i = 1; i < command.argCount - 1; ++i) {
      double num = getNum(executor, command, i, "div", &floating);
      number = (num == 0.0 ? 0.0 : number / num); // defined behavior
   }
   storeNumber(executor, command, number, floating, "div");
}

void builtinMod(const Command &command, Executor &executor) {
   bool floating = false;
   double a = getNum(executor, command, 0, "mod", &floating);
   double b = getNum(executor, command, 1, "mod", &floating);
   storeNumber(executor, command, (b == 0.0 ? 0.0 : fmod(a, b)), floating, "mod");
}

void builtinFloorMod(const Command &command, Executor &executor) {
   bool floating = false;
   double a = getNum(executor, command, 0, "floor-mod", &floating);
   double b = getNum(executor, command, 1, "floor-mod", &floating);
   storeNumber(executor, command, (b == 0.0 ? 0.0 : fmod(fmod(a, b) + b, b)), floating, "floor-mod");
}

void builtinPow(const Command &command, Executor &executor) {
   bool floating = false;
   double a = getNum(executor, command, 0, "pow", &floating);
   double b = getNum(executor, command, 1, "pow", &floating);
   storeNumber(executor, command, pow(a, b), floating, "pow");
}

void builtinNeg(const Command &command, Executor &executor) {
   bool floating = false;
   double n = getNum(executor,command, 0, "neg", &floating);
   storeNumber(executor, command, -n, floating, "neg");
}

void builtinSqrt(const Command &command, Executor &executor) {
   unaryBuiltin(executor, command, sqrt, "sqrt");
}

void builtinCbrt(const Command &command, Executor &executor) {
   unaryBuiltin(executor, command, cbrt, "cbrt");
}

void builtinSin(const Command &command, Executor &executor) {
   unaryBuiltin(executor, command, sin, "sin");
}

void builtinCos(const Command &command, Executor &executor) {
   unaryBuiltin(executor, command, cos, "cos");
}

void builtinTan(const Command &command, Executor &executor) {
   unaryBuiltin(executor, command, tan, "tan");
}

void builtinAsin(const Command &command, Executor &executor) {
   unaryBuiltin(executor, command, asin, "asin");
}

void builtinAcos(const Command &command, Executor &executor) {
   unaryBuiltin(executor, command, acos, "acos");
}

void builtinAtan(const Command &command, Executor &executor) {
   unaryBuiltin(executor, command, atan, "atan");
}

void builtinAtan2(const Command &command, Executor &executor) {
   binaryBuiltin(executor, command, atan2, "atan2");
}

void builtinAsinh(const Command &command, Executor &executor) {
   unaryBuiltin(executor, command, asinh, "asinh");
}

void builtinAcosh(const Command &command, Executor &executor) {
   unaryBuiltin(executor, command, acosh, "acosh");
}

void builtinAtanh(const Command &command, Executor &executor) {
   unaryBuiltin(executor, command, atanh, "atanh");
}

void builtinSinh(const Command &command, Executor &executor) {
   unaryBuiltin(executor, command, sinh, "sinh");
}

void builtinCosh(const Command &command, Executor &executor) {
   unaryBuiltin(executor, command, cosh, "cosh");
}

void builtinTanh(const Command &command, Executor &executor) {
   unaryBuiltin(executor, command, tanh, "tanh");
}

void builtinAbs(const Command &command, Executor &executor) {
   bool floating = false;
   double n = getNum(executor,command, 0, "abs", &floating);
   storeNumber(executor, command, fabs(n), floating, "abs");
}

void builtinMin(const Command &command, Executor &executor) {
   bool floating = false;
   double number = std::numeric_limits<double>::max();
   for (size_t i = 0; i < command.argCount - 1; ++i) {
      number = std::min(number, getNum(executor, command, i, "min", &floating));
   }
   storeNumber(executor, command, number, floating, "min");
}

void builtinMax(const Command &command, Executor &executor) {
   bool floating = false;
   double number = std::numeric_limits<double>::min();
   for (size_t i = 0; i < command.argCount - 1; ++i) {
      number = std::max(number, getNum(executor, command, i, "max", &floating));
   }
   storeNumber(executor, command, number, floating, "max");
}

void builtinClamp(const Command &command, Executor &executor) {
   bool floating = false;
   double x = getNum(executor, command, 0, "clamp", &floating);
   double lo = getNum(executor, command, 1, "clamp", &floating);
   double hi = getNum(executor, command, 2, "clamp", &floating);
   storeNumber(executor, command, std::clamp(x, lo, hi), floating, "clamp");
}

void builtinSign(const Command &command, Executor &executor) {
   double a = getNum(executor, command, 0, "sign");
   storeNumber(executor, command, (a < 0 ? -1 : a > 0 ? 1 : 0), false, "sign");
}

void builtinTrunc(const Command &command, Executor &executor) {
   unaryBuiltin(executor, command, trunc, "trunc");
}

void builtinCeil(const Command &command, Executor &executor) {
   unaryBuiltin(executor, command, ceil, "ceil");
}

void builtinFloor(const Command &command, Executor &executor) {
   unaryBuiltin(executor, command, floor, "floor");
}

void builtinRound(const Command &command, Executor &executor) {
   unaryBuiltin(executor, command, round, "round");
}

void builtinExp(const Command &command, Executor &executor) {
   unaryBuiltin(executor, command, exp, "exp");
}

void builtinLn(const Command &command, Executor &executor) {
   unaryBuiltin(executor, command, log, "ln");
}

void builtinLog(const Command &command, Executor &executor) {
   binaryBuiltin(executor, command, [](double a, double b){ return log(a) / log(b); }, "log");
}

void builtinLog2(const Command &command, Executor &executor) {
   unaryBuiltin(executor, command, log2, "log2");
}

void builtinLog10(const Command &command, Executor &executor) {
   unaryBuiltin(executor, command, log10, "log10");
}

void builtinLerp(const Command &command, Executor &executor) {
   double a = getNum(executor, command, 0, "lerp");
   double b = getNum(executor, command, 1, "lerp");
   double t = getNum(executor, command, 2, "lerp");
   storeNumber(executor, command, a + (b - a) * t, true, "lerp");
}

void builtinStepTowards(const Command &command, Executor &executor) {
   bool floating = false;
   double a = getNum(executor, command, 0, "step-towards", &floating);
   double b = getNum(executor, command, 1, "step-towards", &floating);
   storeNumber(executor, command, (a < b ? a + 1 : a > b ? a - 1 : a), floating, "step-towards");
}

void builtinSeedRandom(const Command &command, Executor &executor) {
   double seed = getNum(executor, command, 0, "seed-random");
   RNG().seed(seed);
}

void builtinRandom(const Command &command, Executor &executor) {
   double r = std::uniform_real_distribution<double>{}(RNG());
   storeNumber(executor, command, r, true, "random");
}

void builtinRandfRange(const Command &command, Executor &executor) {
   double min = getNum(executor, command, 0, "randf-range");
   double max = getNum(executor, command, 1, "randf-range");
   if (min > max) {
      error(executor.diagnostics, command.file, command.line, "randf-range: Min %F is bigger than Max %F. Flip the arguments", min, max);
      return;
   }
   double r = std::uniform_real_distribution<double>{min, max}(RNG());
   storeNumber(executor, command, r, true, "randf-range");
}

void builtinRandiRange(const Command &command, Executor &executor) {
   long min = getNum(executor, command, 0, "randi-range");
   long max = getNum(executor, command, 1, "randi-range");
   if (min > max) {
      error(executor.diagnostics, command.file, command.line, "randi-range: Min %ld is bigger than Max %ld. Flip the arguments", min, max);
      return;
   }
   double r = std::uniform_int_distribution<long>{min, max}(RNG());
   storeNumber(executor, command, r, false, "randi-range");
}

void builtinGcd(const Command &command, Executor &executor) {
   long a = getNum(executor, command, 0, "gcd");
   long b = getNum(executor, command, 1, "gcd");
   storeNumber(executor, command, std::gcd(a, b), false, "gcd");
}

void builtinLcm(const Command &command, Executor &executor) {
   long a = getNum(executor, command, 0, "lcm");
   long b = getNum(executor, command, 1, "lcm");
   storeNumber(executor, command, std::lcm(a, b), false, "lcm");
}

void builtinHypot(const Command &command, Executor &executor) {
   binaryBuiltin(executor, command, std::hypot, "hypot");
}

void builtinHypot3(const Command &command, Executor &executor) {
   double x = getNum(executor, command, 0, "hypot3");
   double y = getNum(executor, command, 1, "hypot3");
   double z = getNum(executor, command, 2, "hypot3");
   storeNumber(executor, command, std::hypot(x, y, z), true, "hypot3");
}

void builtinBitand(const Command &command, Executor &executor) {
   unsigned long a = getNum(executor, command, 0, "bit-and");
   unsigned long b = getNum(executor, command, 1, "bit-and");
   storeNumber(executor, command, a & b, false, "bit-and");
}

void builtinBitor(const Command &command, Executor &executor) {
   unsigned long a = getNum(executor, command, 0, "bit-or");
   unsigned long b = getNum(executor, command, 1, "bit-or");
   storeNumber(executor, command, a | b, false, "bit-or");
}

void builtinBitxor(const Command &command, Executor &executor) {
   unsigned long a = getNum(executor, command, 0, "bit-xor");
   unsigned long b = getNum(executor, command, 1, "bit-xor");
   storeNumber(executor, command, a ^ b, false, "bit-xor");
}

void builtinBitnot(const Command &command, Executor &executor) {
   unsigned long a = getNum(executor, command, 0, "bit-not");
   storeNumber(executor, command, ~a, false, "bit-not");
}

void builtinBitshl(const Command &command, Executor &executor) {
   unsigned long a = getNum(executor, command, 0, "bit-shl");
   unsigned long b = getNum(executor, command, 1, "bit-shl");
   if (b >= sizeof(unsigned long) * 8) {
      error(executor.diagnostics, command.file, command.line, "bit-shl: Shift position %lu is out of range", b);
      return;
   }
   storeNumber(executor, command, a << b, false, "bit-shl");
}

void builtinBitshr(const Command &command, Executor &executor) {
   unsigned long a = getNum(executor, command, 0, "bit-shr");
   unsigned long b = getNum(executor, command, 1, "bit-shr");
   if (b >= sizeof(unsigned long) * 8) {
      error(executor.diagnostics, command.file, command.line, "bit-shr: Shift position %lu is out of range", b);
      return;
   }
   storeNumber(executor, command, a >> b, false, "bit-shr");
}

void builtinBitcount(const Command &command, Executor &executor) {
   unsigned long a = getNum(executor, command, 0, "bit-count");
   storeNumber(executor, command, std::popcount(a), false, "bit-count");
}

void builtinBittest(const Command &command, Executor &executor) {
   unsigned long val = getNum(executor, command, 0, "bit-test");
   unsigned long pos = getNum(executor, command, 1, "bit-test");
   if (pos >= sizeof(unsigned long) * 8) {
      error(executor.diagnostics, command.file, command.line, "bit-test: Bit position %lu is out of range", pos);
      return;
   }
   storeBoolean(executor, command, val & (1ul << pos), "bit-test");
}

void builtinBitset(const Command &command, Executor &executor) {
   unsigned long val = getNum(executor, command, 0, "bit-set");
   unsigned long pos = getNum(executor, command, 1, "bit-set");
   if (pos >= sizeof(unsigned long) * 8) {
      error(executor.diagnostics, command.file, command.line, "bit-set: Bit position %lu is out of range", pos);
      return;
   }
   storeNumber(executor, command, val | (1ul << pos), false, "bit-set");
}

void builtinBitclear(const Command &command, Executor &executor) {
   unsigned long val = getNum(executor, command, 0, "bit-clear");
   unsigned long pos = getNum(executor, command, 1, "bit-clear");
   if (pos >= sizeof(unsigned long) * 8) {
      error(executor.diagnostics, command.file, command.line, "bit-clear: Bit position %lu is out of range", pos);
      return;
   }
   storeNumber(executor, command, val & ~(1ul << pos), false, "bit-clear");
}

void builtinBittoggle(const Command &command, Executor &executor) {
   unsigned long val = getNum(executor, command, 0, "bit-toggle");
   unsigned long pos = getNum(executor, command, 1, "bit-toggle");
   if (pos >= sizeof(unsigned long) * 8) {
      error(executor.diagnostics, command.file, command.line, "bit-toggle: Bit position %lu is out of range", pos);
      return;
   }
   storeNumber(executor, command, val ^ (1ul << pos), false, "bit-toggle");
}
