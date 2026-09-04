#include "builtin.hpp"
#include "pil.hpp"
#include <algorithm>
#include <fstream>
#include <stack>
#include <unordered_map>
#include <unordered_set>

constexpr size_t DEFAULT_REGISTER_COUNT = 16;
constexpr size_t DEFAULT_RETURN_REGISTER_COUNT = 4;

// file reader
PILFile readPILInternal(Diagnostics &diagnostics, LexemeCache &cache, const std::string &path, size_t parentFile, size_t line) {
   size_t fileLexeme = pushLexeme(cache, path);
   std::ifstream file (path);
   if (!file.is_open()) {
      error(diagnostics, parentFile, line, "Could not read file '%s'", path.c_str());
      return PILFile{};
   }
   std::string code (std::istreambuf_iterator<char>(file), {});
   return PILFile{code, fileLexeme};
}

PILFile readPIL(Diagnostics &diagnostics, LexemeCache &cache, const std::string &path) {
   return readPILInternal(diagnostics, cache, path, 0, 0);
}

// lexer

// translate code into tokens. we cache common lexemes that repeat often like identifiers and ops but don't cache numbers,
// characters and strings, which could change during execution and are usually longer and don't repeat as often. Registers
// are safe to cache since they're constants
char handleEscapeCode(Diagnostics &diagnostics, LexemeCache &cache, PILFile &file, size_t &i, size_t tokenLine) {
   static const std::unordered_map<char, char> escapeCodeMap {
      {'a', '\a'}, {'b', '\b'}, {'t', '\t'}, {'n', '\n'}, {'v', '\v'}, {'f', '\f'},
      {'r', '\r'}, {'e', '\e'}, {'\\', '\\'}, {'\'', '\''}, {'"', '"'}
   };

   char ch = file.code[i];
   if (ch != '\\') {
      return ch;
   }

   i += 1;
   ch = file.code[i];
   if (auto it = escapeCodeMap.find(ch); it != escapeCodeMap.end()) {
      return it->second;
   }
   warn(diagnostics, file.lexeme, tokenLine, "Unknown escape code '\\%c'", ch);
   return ch;
}

std::vector<Token> lexPILFile(Diagnostics &diagnostics, LexemeCache &cache, PILFile &file) {
   std::vector<Token> tokens;
   size_t size = file.code.size();
   size_t line = 1;

   for (size_t i = 0; i < size; ++i) {
      char ch = file.code[i];

      if (ch == '\n') {
         tokens.emplace_back(TOKEN_NEWLINE, cacheLexeme(cache, "\n"), file.lexeme, line);
         line += 1;
      }
      else if (ch == '(') {
         tokens.emplace_back(TOKEN_L_PAREN, cacheLexeme(cache, "("), file.lexeme, line);
      }
      else if (ch == ')') {
         tokens.emplace_back(TOKEN_R_PAREN, cacheLexeme(cache, ")"), file.lexeme, line);
      }
      else if (ch == ':') {
         tokens.emplace_back(TOKEN_LABEL, cacheLexeme(cache, ":"), file.lexeme, line);
      }
      else if (i + 2 < size && ch == '.' && file.code[i+1] == '.' && file.code[i+2] == '.') {
         tokens.emplace_back(TOKEN_VARIADIC, cacheLexeme(cache, "..."), file.lexeme, line);
         i += 2;
      }
      else if (ch == ';') {
         while (i < size && file.code[i] != '\n') i += 1;
         tokens.emplace_back(TOKEN_NEWLINE, cacheLexeme(cache, "\n"), file.lexeme, line);
         line += 1;
      }
      else if ((ch == 'r' || ch == 'R') && i + 1 < size && file.code[i + 1] == '$') {
         std::string reg;
         for (i += 2; i < size && std::isdigit(file.code[i]); ++i) {
            reg.push_back(file.code[i]);
         }
         tokens.emplace_back(TOKEN_RETURN_REGISTER, cacheLexeme(cache, reg), file.lexeme, line);
         i -= 1;
      }
      else if (ch == '$') {
         std::string reg;
         for (++i; i < size && std::isdigit(file.code[i]); ++i) {
            reg.push_back(file.code[i]);
         }
         tokens.emplace_back(TOKEN_REGISTER, cacheLexeme(cache, reg), file.lexeme, line);
         i -= 1;
      }
      else if (ch == '\'') {
         if (i + 1 >= size || file.code[i + 1] == '\n') {
            error(diagnostics, file.lexeme, line, "Unterminated character");
            continue;
         }

         i += 1;
         std::string ch (1, handleEscapeCode(diagnostics, cache, file, i, line));

         if (i + 1 >= size || file.code[i + 1] != '\'') {
            error(diagnostics, file.lexeme, line, "Unterminated character");
            continue;
         }
         i += 1;
         tokens.emplace_back(TOKEN_CHARACTER, pushLexeme(cache, ch), file.lexeme, line);
      }
      else if (ch == '"') {
         std::string string;
         size_t originalLine = line;
         string.reserve(16);

         for (++i; i < size && file.code[i] != '"' && file.code[i] != '\n'; ++i) {
            string.push_back(handleEscapeCode(diagnostics, cache, file, i, line));
         }

         if (i >= size || file.code[i] != '"') {
            i -= 1;
            error(diagnostics, file.lexeme, originalLine, "Unterminated string");
            continue;
         }
         tokens.emplace_back(TOKEN_STRING, pushLexeme(cache, string), file.lexeme, originalLine);
      }
      else if ((ch == '-' && i + 1 < size && std::isdigit(file.code[i + 1])) || std::isdigit(ch)) {
         std::string number;
         size_t end = file.code.find_first_not_of(".1234567890", i + 1);
         if (end == std::string::npos) {
            end = size;
         }
         number.reserve(end - i - 1);

         if (ch == '-') {
            number.push_back(ch);
            i += 1;
         }
         bool dot = false;

         for (; i < size && (file.code[i] == '.' || std::isdigit(file.code[i])); ++i) {
            number.push_back(file.code[i]);
            if (file.code[i] == '.') {
               if (dot) {
                  error(diagnostics, file.lexeme, line, "Number '%s' contains multiple decimal points", number.c_str());
                  break;
               }
               dot = true;
            }
         }
         tokens.emplace_back(dot ? TOKEN_FLOATING : TOKEN_INTEGER, pushLexeme(cache, number), file.lexeme, line);
         i -= 1;
      }
      else if (ch == '_' || std::isalpha(ch)) {
         std::string identifier;
         size_t end = i;

         for (++end; end < size && (file.code[end] == '_' || file.code[end] == '-' || std::isalnum(file.code[end])); ++end);
         identifier = file.code.substr(i, end - i);
         std::transform(identifier.begin(), identifier.end(), identifier.begin(), tolower);
         tokens.emplace_back(TOKEN_IDENTIFIER, cacheLexeme(cache, identifier), file.lexeme, line);
         i = end - 1;
      }
      else if (!std::isspace(ch) && ch != ',') {
         error(diagnostics, file.lexeme, line, "Unexpected character '%c'", ch);
      }
   }
   tokens.emplace_back(TOKEN_EOF, cacheLexeme(cache, "EOF"), file.lexeme, line);
   return tokens;
}

// translator (handle includes and misc. directives)

// find all INCLUDE "FILE" statements and push their tokens if the files haven't been included yet. will erase all includes
// after and doesn't have more than a single file open at a time. also handles some other misc. directives.
void translatePIL(Executor &executor, PILFile &file, std::vector<Token> &tokens) {
   std::unordered_set<std::string> includedFiles;
   size_t size = tokens.size();

   size_t includeLexeme = cacheLexeme(executor.cache, "include");
   size_t registerLexeme = cacheLexeme(executor.cache, "register-count");
   size_t returnRegisterLexeme = cacheLexeme(executor.cache, "return-register-count");

   for (size_t i = 0; i < size; ++i) {
      // handle includes
      if (tokens[i].type == TOKEN_IDENTIFIER && tokens[i].lexeme == includeLexeme && i + 1 < size && tokens[i + 1].type == TOKEN_STRING) {
         if (i + 2 >= size || tokens[i + 2].type != TOKEN_NEWLINE) {
            error(executor.diagnostics, tokens[i].file, tokens[i].line, "Excess tokens (or EOF) after include statement");
            continue;
         }

         // destroy all after the loop
         tokens[i].parsed = true;
         tokens[i + 1].parsed = true;
         tokens[i + 2].parsed = true;

         std::string &filename = getLexeme(executor.cache, tokens[i + 1].lexeme);
         if (includedFiles.find(filename) != includedFiles.end()) {
            continue;
         }

         includedFiles.insert(filename);
         PILFile newFile = readPILInternal(executor.diagnostics, executor.cache, filename, tokens[i + 1].file, tokens[i + 1].line);
         std::vector<Token> newTokens = lexPILFile(executor.diagnostics, executor.cache, newFile);
         tokens.insert(tokens.begin() + i + 3, newTokens.begin(), newTokens.end());
         i += 2;
      }
      // handle register config
      else if (tokens[i].type == TOKEN_IDENTIFIER && (tokens[i].lexeme == registerLexeme || tokens[i].lexeme == returnRegisterLexeme) && i + 1 < size && tokens[i + 1].type == TOKEN_INTEGER) {
         if (i + 2 >= size || tokens[i + 2].type != TOKEN_NEWLINE) {
            error(executor.diagnostics, tokens[i].file, tokens[i].line, "Excess tokens (or EOF) after register configuration statement");
            continue;
         }
         tokens[i].parsed = true;
         tokens[i + 1].parsed = true;
         tokens[i + 2].parsed = true;

         size_t count = 0;
         try {
            count = std::stol(getLexeme(executor.cache, tokens[i + 1].lexeme));
         }
         catch (...) {
            error(executor.diagnostics, tokens[i].file, tokens[i].line, "Invalid integer: %s", getLexeme(executor.cache, tokens[i + 1].lexeme).c_str());
            continue;
         }

         if (tokens[i].lexeme == registerLexeme) {
            executor.registers.resize(count);
         }
         else {
            executor.returnRegisters.resize(count);
         }
         i += 2;
      }
   }
   // erase all includes and EOFs
   tokens.erase(std::remove_if(tokens.begin(), tokens.end(), [](const Token &t) { return t.parsed || t.type == TOKEN_EOF; }), tokens.end());
   size_t EOFline = (tokens.empty() ? 1 : tokens.back().line);
   tokens.emplace_back(TOKEN_EOF, cacheLexeme(executor.cache, "EOF"), file.lexeme, EOFline);
}

// built-in functions

// we only define built-in functions that actually get used. thanks, cache. return is a special built-in that is 
void pushConstant(LexemeCache &cache, ByteCode &data, const std::string &lexeme, float value) {
   if (auto it = cache.lexemeCache.find(lexeme); it != cache.lexemeCache.end()) {
      Value v {VALUE_FLOATING};
      v.floating = value;
      v.line = 0;
      v.file = 0;

      ParseValue constant;
      constant.init = true;
      constant.type = GLOBAL;
      constant.global = v;
      data.values[it->second] = constant;
   }
}

void pushBuiltin(LexemeCache &cache, ByteCode &data, const std::string &lexeme, NativeFunction func, size_t paramCount, bool variadic) {
   if (auto it = cache.lexemeCache.find(lexeme); it != cache.lexemeCache.end()) {
      ParseValue function;
      function.init = true;
      function.type = NATIVE_FUNCTION;
      function.variadic = variadic;
      function.params.resize(paramCount);
      function.nativeFunction = func;
      data.values[it->second] = function;
   }
}

void pushReservedBuiltin(LexemeCache &cache, ByteCode &data, const std::string &lexeme, NativeFunction func, size_t paramCount, bool variadic) {
   ParseValue function;
   function.init = true;
   function.type = NATIVE_FUNCTION;
   function.reserved = true;
   function.variadic = variadic;
   function.params.resize(paramCount);
   function.nativeFunction = func;

   size_t index = cacheLexeme(cache, lexeme);
   if (index >= data.values.size()) {
      data.values.push_back(function);
   }
   else {
      data.values[index] = function;
   }
}

void defineStandardBuiltins(LexemeCache &cache, ByteCode &data) {
   data.values.resize(getLexemeCount(cache) + 2); // reserved built-ins might overflow, so adjust for that

   // output
   pushBuiltin(cache, data, "print", builtinPrint, 1, true);
   pushBuiltin(cache, data, "printn", builtinPrintn, 1, true);
   pushBuiltin(cache, data, "printf", builtinPrintf, 1, true);
   pushBuiltin(cache, data, "printfn", builtinPrintfn, 1, true);
   pushBuiltin(cache, data, "str", builtinStr, 1, true);
   pushBuiltin(cache, data, "format", builtinFormat, 2, true);

   // math
   pushBuiltin(cache, data, "add", builtinAdd, 3, true);
   pushBuiltin(cache, data, "sub", builtinSub, 3, true);
   pushBuiltin(cache, data, "mul", builtinMul, 3, true);
   pushBuiltin(cache, data, "div", builtinDiv, 3, true);
   pushBuiltin(cache, data, "mod", builtinMod, 3, false);
   pushBuiltin(cache, data, "pow", builtinPow, 3, false);
   pushBuiltin(cache, data, "neg", builtinNeg, 2, false);
   pushBuiltin(cache, data, "sqrt", builtinSqrt, 2, false);
   pushBuiltin(cache, data, "cbrt", builtinCbrt, 2, false);
   pushBuiltin(cache, data, "sin", builtinSin, 2, false);
   pushBuiltin(cache, data, "cos", builtinCos, 2, false);
   pushBuiltin(cache, data, "tan", builtinTan, 2, false);
   pushBuiltin(cache, data, "asin", builtinAsin, 2, false);
   pushBuiltin(cache, data, "acos", builtinAcos, 2, false);
   pushBuiltin(cache, data, "atan", builtinAtan, 2, false);
   pushBuiltin(cache, data, "atan2", builtinAtan2, 2, false);
   pushBuiltin(cache, data, "asinh", builtinAsinh, 2, false);
   pushBuiltin(cache, data, "acosh", builtinAcosh, 2, false);
   pushBuiltin(cache, data, "atanh", builtinAtanh, 2, false);
   pushBuiltin(cache, data, "sinh", builtinSinh, 2, false);
   pushBuiltin(cache, data, "cosh", builtinCosh, 2, false);
   pushBuiltin(cache, data, "tanh", builtinTanh, 2, false);
   pushBuiltin(cache, data, "abs", builtinAbs, 2, false);
   pushBuiltin(cache, data, "min", builtinMin, 3, true);
   pushBuiltin(cache, data, "max", builtinMax, 3, true);
   pushBuiltin(cache, data, "clamp", builtinClamp, 4, false);
   pushBuiltin(cache, data, "ceil", builtinCeil, 2, false);
   pushBuiltin(cache, data, "floor", builtinFloor, 2, false);
   pushBuiltin(cache, data, "round", builtinRound, 2, false);
   pushBuiltin(cache, data, "exp", builtinExp, 2, false);
   pushBuiltin(cache, data, "ln", builtinLn, 2, false);
   pushBuiltin(cache, data, "log", builtinLog, 3, false);
   pushBuiltin(cache, data, "log2", builtinLog2, 2, false);
   pushBuiltin(cache, data, "log10", builtinLog10, 2, false);

   // comparison
   pushBuiltin(cache, data, "le", builtinLe, 3, false);
   pushBuiltin(cache, data, "gr", builtinGr, 3, false);
   pushBuiltin(cache, data, "leeq", builtinLeeq, 3, false);
   pushBuiltin(cache, data, "greq", builtinGreq, 3, false);
   pushBuiltin(cache, data, "eq", builtinEq, 3, false);
   pushBuiltin(cache, data, "neq", builtinNeq, 3, false);
   pushBuiltin(cache, data, "not", builtinNot, 2, false);

   // control flow
   pushBuiltin(cache, data, "goto", builtinGoto, 1, false);
   pushBuiltin(cache, data, "jmp", builtinJmp, 2, false);
   pushBuiltin(cache, data, "jmpn", builtinJmpn, 2, false);

   // variables
   pushBuiltin(cache, data, "move", builtinMove, 2, false);
   pushBuiltin(cache, data, "set", builtinSet, 2, false);
   pushBuiltin(cache, data, "global", builtinGlobal, 1, true);

   // built-in constants
   pushConstant(cache, data, "pi", 3.1415926535897932384626);
   pushConstant(cache, data, "tau", 2.0 * 3.1415926535897932384626);
   pushConstant(cache, data, "e", 2.7182818284590452353602);
}

// parser

// take the tokens and turn them into executable function blocks and commands. we have 3 levels here: file -> functions ->
// commands. there can be no commands in the file level and no functions in the command level.
void parsePIL(Diagnostics &diagnostics, LexemeCache &cache, ByteCode &data, std::vector<Token> &tokens) {
   // reserved built-ins. must always be there.
   pushReservedBuiltin(cache, data, "return", builtinReturn, 0, true);
   pushReservedBuiltin(cache, data, "call", builtinCall, 1, true);

   // estimate code size
   size_t size = tokens.size();
   data.code.reserve(size / 3);

   // function name and label prepass
   for (size_t i = 0; i < size; ++i) {
      if (tokens[i].type == TOKEN_IDENTIFIER && (tokens[i + 1].type == TOKEN_L_PAREN || tokens[i + 1].type == TOKEN_LABEL)) {
         size_t position = tokens[i].lexeme;

         if (data.values[position].init) {
            ParseValue &definition = data.values[position];
            const char *type = (definition.type == NATIVE_FUNCTION ? "Native function" : (definition.type == LABEL ? "Label" : "Function"));
            if (data.values[position].reserved) {
               error(diagnostics, tokens[i].file, tokens[i].line, "'%s' is a reserved built-in. You cannot redefine it", getLexeme(cache, position).c_str());
               return; // you fucked up hard here
            }
            else {
               warn(diagnostics, tokens[i].file, tokens[i].line, "%s '%s' redefined", type, getLexeme(cache, position).c_str());
            }
         }
         ParseValue function;
         function.init = true;
         function.type = (tokens[i + 1].type == TOKEN_LABEL ? LABEL : FUNCTION);
         function.label = 0;
         data.values[position] = function;
      }
   }

   // real parsing
   std::unordered_map<size_t, size_t> functionParamMap;
   size_t returnLexeme = cacheLexeme(cache, "return");
   size_t defineLexeme = cacheLexeme(cache, "define");
   bool firstFunction = true;

   for (size_t i = 0; i < size && tokens[i].type != TOKEN_EOF; ++i) {
      // skip extraneous newlines
      while (i < tokens.size() && tokens[i].type == TOKEN_NEWLINE) ++i;
      if (tokens[i].type == TOKEN_EOF) break;

      // labels
      if (tokens[i].type == TOKEN_IDENTIFIER && tokens[i + 1].type == TOKEN_LABEL) {
         size_t start = i;
         ParseValue &label = data.values[tokens[i].lexeme];
         label.label = data.code.size();

         i += 2;
         if (i >= size || tokens[i].type != TOKEN_NEWLINE) {
            error(diagnostics, tokens[start].file, tokens[start].line, "Excess tokens (or EOF) after label");
         }
      }
      // function declarations
      else if (tokens[i].type == TOKEN_IDENTIFIER && tokens[i + 1].type == TOKEN_L_PAREN) {
         if (!firstFunction && (data.code.empty() || data.code.back().lexeme != returnLexeme)) {
            data.code.emplace_back(returnLexeme, tokens[i-1].file, tokens[i-1].line, std::vector<Value>{});
         }
         firstFunction = false;

         size_t start = i;
         ParseValue &function = data.values[tokens[i].lexeme];
         bool variadic = false;
         functionParamMap.clear();

         for (i += 2; i < size && tokens[i].type != TOKEN_EOF && tokens[i].type != TOKEN_R_PAREN; ++i) {
            if (tokens[i + 1].type == TOKEN_VARIADIC) {
               variadic = true;
               i += 2;
               break;
            }

            if (tokens[i].type != TOKEN_IDENTIFIER) {
               error(diagnostics, tokens[i].file, tokens[i].line, "Function parameters: expected Identifier, got %s instead", getTokenName(tokens[i].type));
            }

            if (functionParamMap.find(tokens[i].lexeme) != functionParamMap.end() || data.values[tokens[i].lexeme].init) {
               error(diagnostics, tokens[i].file, tokens[i].line, "Function parameters: redefined parameter '%s'", getLexeme(cache, tokens[i].lexeme).c_str());
            }
            function.params.push_back(tokens[i].lexeme);
            functionParamMap[tokens[i].lexeme] = functionParamMap.size();
         }

         if (variadic && tokens[i].type != TOKEN_R_PAREN) {
            error(diagnostics, tokens[i].file, tokens[i].line, "Function parameters: variadic parameter should be at the end of the parameter list");
         }
         else if (!variadic && tokens[i].type != TOKEN_R_PAREN) {
            error(diagnostics, tokens[start].file, tokens[start].line, "Unterminated function parameters");
         }

         // variable declarations
         i += 1;
         if (tokens[i].type == TOKEN_IDENTIFIER && tokens[i].lexeme == defineLexeme) {
            for (++i; i < size && tokens[i].type != TOKEN_EOF && tokens[i].type != TOKEN_NEWLINE; ++i) {
               if (tokens[i].type != TOKEN_IDENTIFIER) {
                  error(diagnostics, tokens[i].file, tokens[i].line, "Expected unique Identifier, got %s instead", getTokenName(tokens[i].type));
                  continue;
               }

               if (functionParamMap.find(tokens[i].lexeme) != functionParamMap.end() || data.values[tokens[i].lexeme].init) {
                  error(diagnostics, tokens[i].file, tokens[i].line, "Redefined define '%s'", getLexeme(cache, tokens[i].lexeme).c_str());
                  continue;
               }
               functionParamMap[tokens[i].lexeme] = functionParamMap.size();
            }
         }
         function.variadic = variadic;
         function.function = data.code.size();
         function.localCount = functionParamMap.size();

         if (i >= size || tokens[i].type != TOKEN_NEWLINE) {
            error(diagnostics, tokens[start].file, tokens[start].line, "Excess tokens (or EOF) after function definition");
         }
      }
      // function calls
      else {
         if (tokens[i].type != TOKEN_IDENTIFIER || tokens[i].lexeme >= data.values.size() || !data.values[tokens[i].lexeme].init || (data.values[tokens[i].lexeme].type != FUNCTION && data.values[tokens[i].lexeme].type != NATIVE_FUNCTION)) {
            if (tokens[i].type == TOKEN_IDENTIFIER) {
               error(diagnostics, tokens[i].file, tokens[i].line, "No such function '%s'", getLexeme(cache, tokens[i].lexeme).c_str());
            }
            else {
               error(diagnostics, tokens[i].file, tokens[i].line, "Expected a function call, got %s instead", getTokenName(tokens[i].type));
            }
            // to not spiral errors out of control
            while (i < size && tokens[i].type != TOKEN_EOF && tokens[i].type != TOKEN_NEWLINE) i += 1;
            i -= 1;
            continue;
         }

         data.code.emplace_back(tokens[i].lexeme, tokens[i].file, tokens[i].line, std::vector<Value>{});
         Command &command = data.code.back();

         for (++i; i < size && tokens[i].type != TOKEN_EOF && tokens[i].type != TOKEN_NEWLINE; ++i) {
            Value value {VALUE_COUNT};
            value.line = tokens[i].line;
            value.file = tokens[i].file;

            switch (tokens[i].type) {
            case TOKEN_IDENTIFIER:
               if (auto it = functionParamMap.find(tokens[i].lexeme); it != functionParamMap.end()) {
                  value.type = VALUE_LOCAL;
                  value.local = it->second;
               }
               else {
                  value.type = VALUE_IDENTIFIER;
                  value.identifier = tokens[i].lexeme;
               }
               break;
            case TOKEN_INTEGER:
               value.type = VALUE_INTEGER;
               try {
                  value.integer = std::stol(getLexeme(cache, tokens[i].lexeme));
               }
               catch (...) {
                  value.integer = 0;
                  error(diagnostics, value.file, value.line, "Invalid integer: %s", getLexeme(cache, tokens[i].lexeme).c_str());
               }
               break;
            case TOKEN_FLOATING:
               value.type = VALUE_FLOATING;
               try {
                  value.floating = std::stod(getLexeme(cache, tokens[i].lexeme));
               }
               catch (...) {
                  value.floating = 0;
                  error(diagnostics, value.file, value.line, "Invalid floating point number: %s", getLexeme(cache, tokens[i].lexeme).c_str());
               }
               break;
            case TOKEN_STRING:
               value.type = VALUE_STRING;
               value.string = tokens[i].lexeme;
               break;
            case TOKEN_CHARACTER:
               value.type = VALUE_CHARACTER;
               value.character = getLexeme(cache, tokens[i].lexeme).front();
               break;
            case TOKEN_RETURN_REGISTER:
            case TOKEN_REGISTER:
               value.type = (tokens[i].type == TOKEN_RETURN_REGISTER ? VALUE_RETURN_REGISTER : VALUE_REGISTER);
               try {
                  value.reg = std::stoull(getLexeme(cache, tokens[i].lexeme));
               }
               catch (...) {
                  value.reg = 0;
                  error(diagnostics, value.file, value.line, "Invalid register: %s$%s", tokens[i].type == TOKEN_RETURN_REGISTER ? "R" : "", getLexeme(cache, tokens[i].lexeme).c_str());
               }
               break;
            default:
               error(diagnostics, value.file, value.line, "Unexpected token %s in function call", getLexeme(cache, tokens[i].lexeme).c_str());
            }
            command.args.push_back(value);
         }
      }
   }
   if (!tokens.empty()) {
      data.code.emplace_back(returnLexeme, tokens.back().file, tokens.back().line, std::vector<Value>{});
   }
}

// executor

// execute bytecode. it's really simple - execute the function that the pointer is on. return and call logic can be found
// in the builtin header since they're also callable functions.
void callPILFunction(Executor &executor, const std::string &name, ErrorSeverity stopSeverity) {
   size_t lexeme = cacheLexeme(executor.cache, name);
   if (lexeme >= executor.code.values.size() || !executor.code.values[lexeme].init || executor.code.values[lexeme].type != FUNCTION) {
      error(executor.diagnostics, 0, 0, "Function '%s' cannot be called as it is not defined", name.c_str());
      return;
   }

   if (!executor.code.values[lexeme].params.empty() || executor.code.values[lexeme].variadic) {
      error(executor.diagnostics, 0, 0, "Attempted to call function '%s' with 0 arguments", name.c_str());
      return;
   }

   if (executor.registers.empty()) executor.registers.resize(DEFAULT_REGISTER_COUNT);
   if (executor.returnRegisters.empty()) executor.returnRegisters.resize(DEFAULT_RETURN_REGISTER_COUNT);
   executor.stackTrace = {};
   executor.pointer = executor.code.values[lexeme].function;
   executor.returnCount = 0;
   executor.exitCalled = false;

   while (true) {
      Command &command = executor.code.code[executor.pointer];
      ParseValue &function = executor.code.values[command.lexeme];
      size_t args = command.args.size();
      size_t params = function.params.size();
      bool variadic = function.variadic;

      if ((!variadic && args != params) || (variadic && args < params)) {
         error(executor.diagnostics, command.file, command.line, "Function expected %s%zu parameters, but received %zu arguments", (variadic ? ">" : ""), params, args);
      }
      else {
         if (function.type == NATIVE_FUNCTION) {
            function.nativeFunction(command, executor);
         }
         else if (function.type == FUNCTION) {
            Trace trace (executor.pointer, command.lexeme, 0);
            trace.locals = std::vector<Value>(function.localCount, Value{VALUE_COUNT});

            for (size_t i = 0; i < params; ++i) {
               trace.locals[i] = resolveVariable(executor, command.args[i], "PIL::callPILFunction");
            }
            executor.stackTrace.push(trace);
            executor.pointer = function.function;
            continue;
         }
         else {
            error(executor.diagnostics, command.file, command.line, "Stray %s '%s'", getParseValueName(function.type), getLexeme(executor.cache, command.lexeme).c_str());
         }
      }

      if (executor.exitCalled || shouldError(executor.diagnostics, stopSeverity)) {
         break;
      }
      executor.pointer += 1;
   }
}
