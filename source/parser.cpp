#include "builtin.hpp"
#include "pil.hpp"

// we only define built-in functions that actually get used. thanks, cache. return is a special built-in that is always pushed.
void pushConstant(Executor &executor, const std::string &lexeme, float value) {
   if (auto it = executor.cache.lexemeCache.find(lexeme); it != executor.cache.lexemeCache.end()) {
      Value v {VALUE_FLOATING};
      v.floating = value;

      ParseValue constant;
      constant.init = true;
      constant.type = GLOBAL;
      constant.global = v;
      executor.values[it->second] = constant;
   }
}

void pushBuiltin(Executor &executor, const std::string &lexeme, NativeFunction func, size_t paramCount, bool variadic) {
   if (auto it = executor.cache.lexemeCache.find(lexeme); it != executor.cache.lexemeCache.end()) {
      ParseValue function;
      function.init = true;
      function.type = NATIVE_FUNCTION;
      function.variadic = variadic;
      function.params.resize(paramCount);
      function.nativeFunction = func;
      executor.values[it->second] = function;
   }
}

void pushReservedBuiltin(Executor &executor, const std::string &lexeme, NativeFunction func, size_t paramCount, bool variadic) {
   ParseValue function;
   function.init = true;
   function.type = NATIVE_FUNCTION;
   function.reserved = true;
   function.variadic = variadic;
   function.params.resize(paramCount);
   function.nativeFunction = func;

   size_t index = cacheLexeme(executor.cache, lexeme);
   if (index >= executor.values.size()) {
      executor.values.push_back(function);
   }
   else {
      executor.values[index] = function;
   }
}

void defineStandardBuiltins(Executor &executor) {
   executor.values.resize(getLexemeCount(executor.cache) + 2); // plus reserved built-ins. avoid extra allocation

   // output
   pushBuiltin(executor, "print", builtinPrint, 1, true);
   pushBuiltin(executor, "printn", builtinPrintn, 1, true);
   pushBuiltin(executor, "printf", builtinPrintf, 1, true);
   pushBuiltin(executor, "printfn", builtinPrintfn, 1, true);
   pushBuiltin(executor, "string-new", builtinStringNew, 1, true);
   pushBuiltin(executor, "format", builtinFormat, 2, true);

   // math
   pushBuiltin(executor, "incr", builtinIncr, 1, false);
   pushBuiltin(executor, "decr", builtinDecr, 1, false);
   pushBuiltin(executor, "add", builtinAdd, 3, true);
   pushBuiltin(executor, "sub", builtinSub, 3, true);
   pushBuiltin(executor, "mul", builtinMul, 3, true);
   pushBuiltin(executor, "div", builtinDiv, 3, true);
   pushBuiltin(executor, "mod", builtinMod, 3, false);
   pushBuiltin(executor, "pow", builtinPow, 3, false);
   pushBuiltin(executor, "neg", builtinNeg, 2, false);
   pushBuiltin(executor, "sqrt", builtinSqrt, 2, false);
   pushBuiltin(executor, "cbrt", builtinCbrt, 2, false);
   pushBuiltin(executor, "sin", builtinSin, 2, false);
   pushBuiltin(executor, "cos", builtinCos, 2, false);
   pushBuiltin(executor, "tan", builtinTan, 2, false);
   pushBuiltin(executor, "asin", builtinAsin, 2, false);
   pushBuiltin(executor, "acos", builtinAcos, 2, false);
   pushBuiltin(executor, "atan", builtinAtan, 2, false);
   pushBuiltin(executor, "atan2", builtinAtan2, 2, false);
   pushBuiltin(executor, "asinh", builtinAsinh, 2, false);
   pushBuiltin(executor, "acosh", builtinAcosh, 2, false);
   pushBuiltin(executor, "atanh", builtinAtanh, 2, false);
   pushBuiltin(executor, "sinh", builtinSinh, 2, false);
   pushBuiltin(executor, "cosh", builtinCosh, 2, false);
   pushBuiltin(executor, "tanh", builtinTanh, 2, false);
   pushBuiltin(executor, "abs", builtinAbs, 2, false);
   pushBuiltin(executor, "min", builtinMin, 3, true);
   pushBuiltin(executor, "max", builtinMax, 3, true);
   pushBuiltin(executor, "clamp", builtinClamp, 4, false);
   pushBuiltin(executor, "sign", builtinSign, 2, false);
   pushBuiltin(executor, "trunc", builtinTrunc, 2, false);
   pushBuiltin(executor, "ceil", builtinCeil, 2, false);
   pushBuiltin(executor, "floor", builtinFloor, 2, false);
   pushBuiltin(executor, "round", builtinRound, 2, false);
   pushBuiltin(executor, "exp", builtinExp, 2, false);
   pushBuiltin(executor, "ln", builtinLn, 2, false);
   pushBuiltin(executor, "log", builtinLog, 3, false);
   pushBuiltin(executor, "log2", builtinLog2, 2, false);
   pushBuiltin(executor, "log10", builtinLog10, 2, false);
   pushBuiltin(executor, "lerp", builtinLerp, 4, false);
   pushBuiltin(executor, "step-towards", builtinStepTowards, 3, false);

   // comparison
   pushBuiltin(executor, "le", builtinLe, 3, false);
   pushBuiltin(executor, "gr", builtinGr, 3, false);
   pushBuiltin(executor, "leeq", builtinLeeq, 3, false);
   pushBuiltin(executor, "greq", builtinGreq, 3, false);
   pushBuiltin(executor, "eq", builtinEq, 3, false);
   pushBuiltin(executor, "neq", builtinNeq, 3, false);
   pushBuiltin(executor, "not", builtinNot, 2, false);

   // control flow
   pushBuiltin(executor, "goto", builtinGoto, 1, false);
   pushBuiltin(executor, "jmp", builtinJmp, 2, false);
   pushBuiltin(executor, "jmpn", builtinJmpn, 2, false);

   // types
   pushBuiltin(executor, "typeof", builtinTypeof, 2, false);
   pushBuiltin(executor, "sizeof", builtinSizeof, 2, false);
   pushBuiltin(executor, "isnum", builtinIsnum, 2, false);
   pushBuiltin(executor, "isfloat", builtinIsfloat, 2, false);
   pushBuiltin(executor, "isint", builtinIsint, 2, false);
   pushBuiltin(executor, "ischar", builtinIschar, 2, false);
   pushBuiltin(executor, "isstring", builtinIsstring, 2, false);
   pushBuiltin(executor, "isreg", builtinIsreg, 2, false);
   pushBuiltin(executor, "isfunction", builtinIsfunction, 2, false);
   pushBuiltin(executor, "islabel", builtinIslabel, 2, false);
   pushBuiltin(executor, "isnull", builtinIsnull, 2, false);
   pushBuiltin(executor, "isinf", builtinIsinf, 2, false);
   pushBuiltin(executor, "isnan", builtinIsnan, 2, false);
   pushBuiltin(executor, "toint", builtinToint, 2, false);
   pushBuiltin(executor, "tofloat", builtinTofloat, 2, false);
   pushBuiltin(executor, "tochar", builtinTochar, 2, false);
   pushBuiltin(executor, "exists", builtinExists, 2, false);

   // misc. (time, random)
   pushBuiltin(executor, "time", builtinTime, 1, false);
   pushBuiltin(executor, "unix-time", builtinUnixTime, 1, false);
   pushBuiltin(executor, "date", builtinDate, 2, false);
   pushBuiltin(executor, "sleep", builtinSleep, 1, false);
   pushBuiltin(executor, "seed-random", builtinSeedRandom, 1, false);
   pushBuiltin(executor, "random", builtinRandom, 1, false);
   pushBuiltin(executor, "randf-range", builtinRandfRange, 3, false);
   pushBuiltin(executor, "randi-range", builtinRandiRange, 3, false);

   // variables
   pushBuiltin(executor, "swap", builtinSwap, 2, false);
   pushBuiltin(executor, "set", builtinSet, 2, false);
   pushBuiltin(executor, "global", builtinGlobal, 1, true);

   // built-in constants
   pushConstant(executor, "pi", 3.1415926535897932384626);
   pushConstant(executor, "tau", 2.0 * 3.1415926535897932384626);
   pushConstant(executor, "e", 2.7182818284590452353602);
}

Value parseToken(Executor &executor, Token token, const std::unordered_map<size_t, size_t> &functionParamMap) {
   Value value {VALUE_COUNT};
   switch (token.type) {
   case TOKEN_IDENTIFIER:
      if (auto it = functionParamMap.find(token.lexeme); it != functionParamMap.end()) {
         value.type = VALUE_LOCAL;
         value.local = it->second;
      }
      else {
         value.type = VALUE_IDENTIFIER;
         value.identifier = token.lexeme;
      }
      break;
   case TOKEN_INTEGER:
      value.type = VALUE_INTEGER;
      try {
         value.integer = std::stol(getLexeme(executor.cache, token.lexeme));
      }
      catch (...) {
         value.integer = 0;
         error(executor.diagnostics, token.file, token.line, "Invalid integer: %s", getLexeme(executor.cache, token.lexeme).c_str());
      }
      break;
   case TOKEN_FLOATING:
      value.type = VALUE_FLOATING;
      try {
         value.floating = std::stod(getLexeme(executor.cache, token.lexeme));
      }
      catch (...) {
         value.floating = 0;
         error(executor.diagnostics, token.file, token.line, "Invalid floating point number: %s", getLexeme(executor.cache, token.lexeme).c_str());
      }
      break;
   case TOKEN_STRING:
      value.type = VALUE_CSTRING;
      value.string = token.lexeme;
      break;
   case TOKEN_CHARACTER:
      value.type = VALUE_CHARACTER;
      value.character = getLexeme(executor.cache, token.lexeme).front();
      break;
   case TOKEN_RETURN_REGISTER:
   case TOKEN_REGISTER: {
      std::vector<Value> &container = (token.type == TOKEN_RETURN_REGISTER ? executor.returnRegisters : executor.registers);
      size_t maxDefaultValue = (token.type == TOKEN_RETURN_REGISTER ? DEFAULT_RETURN_REGISTER_COUNT : DEFAULT_REGISTER_COUNT);
      size_t maxValue = (container.empty() ? maxDefaultValue : container.size());

      value.type = (token.type == TOKEN_RETURN_REGISTER ? VALUE_RETURN_REGISTER : VALUE_REGISTER);
      try {
         value.reg = std::stoull(getLexeme(executor.cache, token.lexeme));
      }
      catch (...) {
         value.reg = 0;
         error(executor.diagnostics, token.file, token.line, "Invalid register: %s$%s", token.type == TOKEN_RETURN_REGISTER ? "R" : "", getLexeme(executor.cache, token.lexeme).c_str());
      }

      if (value.reg >= maxValue) {
         error(executor.diagnostics, token.file, token.line, "Register %s$%zu is out of bounds", token.type == TOKEN_RETURN_REGISTER ? "R" : "", value.reg);
      }
      break;
   }
   default:
      error(executor.diagnostics, token.file, token.line, "Unexpected token %s in function call", getLexeme(executor.cache, token.lexeme).c_str());
   }
   return value;
}

// take the tokens and turn them into executable function blocks and commands. we have 3 levels here: file -> functions ->
// commands. there can be no commands in the file level and no functions in the command level.
void parsePIL(Executor &executor, std::vector<Token> &tokens) {
   // reserved built-ins. must always be there.
   pushReservedBuiltin(executor, "return", builtinReturn, 0, true);
   pushReservedBuiltin(executor, "call", builtinCall, 1, true);

   // estimate code size
   size_t size = tokens.size();
   executor.code.reserve(size / 3);
   executor.arguments.reserve(size / 4);

   // function name and label prepass
   for (size_t i = 0; i < size; ++i) {
      if (tokens[i].type == TOKEN_IDENTIFIER && (tokens[i + 1].type == TOKEN_L_PAREN || tokens[i + 1].type == TOKEN_LABEL)) {
         size_t position = tokens[i].lexeme;

         if (executor.values[position].init) {
            ParseValue &definition = executor.values[position];
            const char *type = getParseValueName(definition.type);
            const char *lexeme = getLexeme(executor.cache, position).c_str();
            error(executor.diagnostics, tokens[i].file, tokens[i].line, "%s '%s' redefined", type, lexeme);
         }
         ParseValue function;
         function.init = true;
         function.type = (tokens[i + 1].type == TOKEN_LABEL ? LABEL : FUNCTION);
         function.label = 0;
         executor.values[position] = function;
      }
   }

   // real parsing
   std::unordered_map<size_t, size_t> functionParamMap;
   size_t returnLexeme = cacheLexeme(executor.cache, "return");
   size_t defineLexeme = cacheLexeme(executor.cache, "let");
   size_t callLexeme = cacheLexeme(executor.cache, "call");
   bool firstFunction = true;

   for (size_t i = 0; i < size && tokens[i].type != TOKEN_EOF; ++i) {
      // skip extraneous newlines
      while (i < tokens.size() && tokens[i].type == TOKEN_NEWLINE) ++i;
      if (tokens[i].type == TOKEN_EOF) break;

      // labels
      if (tokens[i].type == TOKEN_IDENTIFIER && tokens[i + 1].type == TOKEN_LABEL) {
         size_t start = i;
         ParseValue &label = executor.values[tokens[i].lexeme];
         label.label = executor.code.size();

         i += 2;
         if (i >= size || tokens[i].type != TOKEN_NEWLINE) {
            error(executor.diagnostics, tokens[start].file, tokens[start].line, "Excess tokens (or EOF) after label");
         }
      }
      // function declarations
      else if (tokens[i].type == TOKEN_IDENTIFIER && tokens[i + 1].type == TOKEN_L_PAREN) {
         if (!firstFunction && (executor.code.empty() || executor.code.back().lexeme != returnLexeme)) {
            executor.code.emplace_back(returnLexeme, tokens[i-1].file, tokens[i-1].line, 0, 0);
         }
         size_t start = i;
         ParseValue &function = executor.values[tokens[i].lexeme];
         bool variadic = false;
         firstFunction = false;
         functionParamMap.clear();

         for (i += 2; i < size && tokens[i].type != TOKEN_EOF && tokens[i].type != TOKEN_R_PAREN; ++i) {
            if (tokens[i + 1].type == TOKEN_VARIADIC) {
               variadic = true;
               i += 2;
               break;
            }

            if (tokens[i].type != TOKEN_IDENTIFIER) {
               error(executor.diagnostics, tokens[i].file, tokens[i].line, "Function parameters: expected Identifier, got %s instead", getTokenName(tokens[i].type));
            }

            if (functionParamMap.find(tokens[i].lexeme) != functionParamMap.end() || executor.values[tokens[i].lexeme].init) {
               error(executor.diagnostics, tokens[i].file, tokens[i].line, "Function parameters: redefined parameter '%s'", getLexeme(executor.cache, tokens[i].lexeme).c_str());
            }
            function.params.push_back(tokens[i].lexeme);
            functionParamMap[tokens[i].lexeme] = functionParamMap.size();
         }

         if (variadic && tokens[i].type != TOKEN_R_PAREN) {
            error(executor.diagnostics, tokens[i].file, tokens[i].line, "Function parameters: variadic parameter should be at the end of the parameter list");
         }
         else if (!variadic && tokens[i].type != TOKEN_R_PAREN) {
            error(executor.diagnostics, tokens[start].file, tokens[start].line, "Unterminated function parameters");
         }

         // variable declarations
         i += 1;
         if (tokens[i].type == TOKEN_IDENTIFIER && tokens[i].lexeme == defineLexeme) {
            for (++i; i < size && tokens[i].type != TOKEN_EOF && tokens[i].type != TOKEN_NEWLINE; ++i) {
               if (tokens[i].type != TOKEN_IDENTIFIER) {
                  error(executor.diagnostics, tokens[i].file, tokens[i].line, "Expected unique Identifier, got %s instead", getTokenName(tokens[i].type));
                  continue;
               }

               if (functionParamMap.find(tokens[i].lexeme) != functionParamMap.end() || executor.values[tokens[i].lexeme].init) {
                  error(executor.diagnostics, tokens[i].file, tokens[i].line, "Redefined define '%s'", getLexeme(executor.cache, tokens[i].lexeme).c_str());
                  continue;
               }
               functionParamMap[tokens[i].lexeme] = functionParamMap.size();
            }
         }
         function.variadic = variadic;
         function.function = executor.code.size();
         function.localCount = functionParamMap.size();

         if (i >= size || tokens[i].type != TOKEN_NEWLINE) {
            error(executor.diagnostics, tokens[start].file, tokens[start].line, "Excess tokens (or EOF) after function definition");
         }
      }
      // function calls
      else {
         if (tokens[i].type != TOKEN_IDENTIFIER || !executor.values[tokens[i].lexeme].init || (executor.values[tokens[i].lexeme].type != FUNCTION && executor.values[tokens[i].lexeme].type != NATIVE_FUNCTION)) {
            if (tokens[i].type == TOKEN_IDENTIFIER) {
               error(executor.diagnostics, tokens[i].file, tokens[i].line, "No such function '%s'", getLexeme(executor.cache, tokens[i].lexeme).c_str());
            }
            else {
               error(executor.diagnostics, tokens[i].file, tokens[i].line, "Expected a function call, got %s instead", getTokenName(tokens[i].type));
            }
            // to not spiral errors out of control
            while (i < size && tokens[i].type != TOKEN_EOF && tokens[i].type != TOKEN_NEWLINE) i += 1;
            i -= 1;
            continue;
         }

         executor.code.emplace_back(tokens[i].lexeme, tokens[i].file, tokens[i].line, executor.arguments.size(), 0);
         Command &command = executor.code.back();
         bool isCall = (tokens[i].lexeme == callLexeme);
         size_t start = i + 1;

         for (++i; i < size && tokens[i].type != TOKEN_EOF && tokens[i].type != TOKEN_NEWLINE; ++i) {
            Value value = parseToken(executor, tokens[i], functionParamMap);
            if (isCall && value.type == VALUE_IDENTIFIER && executor.values[value.identifier].init && executor.values[value.identifier].type == FUNCTION) {
               if (command.callee != std::string::npos) {
                  error(executor.diagnostics, command.file, command.line, "call: Cannot call multiple functions in a single call");
               }
               command.callee = i - start;
            }

            executor.arguments.push_back(value);
            command.argCount += 1;
         }

         if (isCall && command.callee == std::string::npos) {
            error(executor.diagnostics, command.file, command.line, "call: Expected function name to call");
         }

         size_t args = command.argCount;
         size_t params = executor.values[command.lexeme].params.size();
         bool variadic = executor.values[command.lexeme].variadic;

         if ((!variadic && args != params) || (variadic && args < params)) {
            error(executor.diagnostics, command.file, command.line, "Function '%s' expected %s%zu parameters, but received %zu arguments", getLexeme(executor.cache, command.lexeme).c_str(), (variadic ? ">" : ""), params, args);
         }

         if (isCall) {
            size_t lexeme = executor.arguments[command.argStart + command.callee].identifier;
            ParseValue &function = executor.values[lexeme];
            args = command.argCount - command.callee - 1;
            params = function.params.size();
            variadic = function.variadic;
            if ((!variadic && args != params) || (variadic && args < params)) {
               error(executor.diagnostics, command.file, command.line, "call: Function '%s' expected %s%zu parameters, but received %zu arguments", getLexeme(executor.cache, lexeme).c_str(), (variadic ? ">" : ""), params, args);
            }
         }
      }
   }
   if (!tokens.empty()) {
      executor.code.emplace_back(returnLexeme, tokens.back().file, tokens.back().line, 0, 0);
   }
}
