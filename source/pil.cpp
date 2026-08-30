#include "builtin.hpp"
#include "pil.hpp"
#include <algorithm>
#include <format>
#include <fstream>
#include <stack>
#include <unordered_map>
#include <unordered_set>

// file reader
PILFile readPILInternal(Diagnostics &diagnostics, LexemeCache &cache, const std::string &path, size_t parentFile, size_t line) {
   size_t fileLexeme = pushLexeme(cache, path);
   std::ifstream file (path);
   if (!file.is_open()) {
      error(diagnostics, std::format("Could not read file '{}'", path), parentFile, line);
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
   warn(diagnostics, std::format("Unknown escape code '\\{}'", ch), file.lexeme, tokenLine);
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
            error(diagnostics, "Unterminated character", file.lexeme, line);
            continue;
         }

         i += 1;
         std::string ch (1, handleEscapeCode(diagnostics, cache, file, i, line));

         if (i + 1 >= size || file.code[i + 1] != '\'') {
            error(diagnostics, "Unterminated character", file.lexeme, line);
            continue;
         }
         i += 1;
         tokens.emplace_back(TOKEN_CHARACTER, pushLexeme(cache, ch), file.lexeme, line);
      }
      else if (ch == '"') {
         std::string string;
         size_t originalLine = line;
         string.reserve(16);

         for (++i; i < size && file.code[i] != '"'; ++i) {
            string.push_back(handleEscapeCode(diagnostics, cache, file, i, line));
            line += (file.code[i] == '\n');
         }

         if (i >= size || file.code[i] != '"') {
            error(diagnostics, "Unterminated string", file.lexeme, originalLine);
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
                  error(diagnostics, std::format("Number '{}' contains multiple decimal points", number), file.lexeme, line);
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
         error(diagnostics, std::format("Unexpected character '{}'", ch), file.lexeme, line);
      }
   }
   tokens.emplace_back(TOKEN_EOF, cacheLexeme(cache, "EOF"), file.lexeme, line);
   return tokens;
}

// translator (handle includes)

// find all INCLUDE "FILE" statements and push their tokens if the files haven't been included yet. will erase all includes
// after and doesn't have more than a single file open at a time.
void handlePILFileIncludes(Diagnostics &diagnostics, LexemeCache &cache, PILFile &file, std::vector<Token> &tokens) {
   std::unordered_set<std::string> includedFiles;
   size_t size = tokens.size();

   for (size_t i = 0; i < size; i += 3) {
      if (tokens[i].type != TOKEN_IDENTIFIER || i + 1 >= size || tokens[i + 1].type != TOKEN_STRING || getLexeme(cache, tokens[i].lexeme) != "include") {
         i -= 2;
         continue;
      }

      if (i + 2 >= size || tokens[i + 2].type != TOKEN_NEWLINE) {
         error(diagnostics, "Excess tokens (or EOF) after include statement", tokens[i].fileLexeme, tokens[i + 1].line);
         i -= 2;
         continue;
      }

      // destroy all include statements after the loop
      tokens[i].parsed = true;
      tokens[i + 1].parsed = true;
      tokens[i + 2].parsed = true;

      std::string &filename = getLexeme(cache, tokens[i + 1].lexeme);
      if (includedFiles.find(filename) != includedFiles.end()) {
         continue;
      }

      includedFiles.insert(filename);
      PILFile newFile = readPILInternal(diagnostics, cache, filename, tokens[i + 1].fileLexeme, tokens[i + 1].line);
      std::vector<Token> newTokens = lexPILFile(diagnostics, cache, newFile);
      tokens.insert(tokens.begin() + i + 3, newTokens.begin(), newTokens.end());
   }
   // erase all includes and EOFs
   tokens.erase(std::remove_if(tokens.begin(), tokens.end(), [](const Token &t) { return t.parsed || t.type == TOKEN_EOF; }), tokens.end());

   size_t EOFline = (tokens.empty() ? 1 : tokens.back().line);
   tokens.emplace_back(TOKEN_EOF, cacheLexeme(cache, "EOF"), file.lexeme, EOFline);
}

// built-in functions

// we only define built-in functions that actually get used. thanks, cache. return is a special built-in that is 
void pushBuiltin(LexemeCache &cache, ByteCode &data, const std::string &lexeme, NativeFunction func, size_t paramCount, bool variadic) {
   if (auto it = cache.lexemeCache.find(lexeme); it != cache.lexemeCache.end()) {
      Function function;
      function.init = true;
      function.type = NATIVE_FUNCTION;
      function.variadic = variadic;
      function.params.resize(paramCount);
      function.nativeFunction = func;
      data.functions[it->second] = function;
   }
}

void pushReservedBuiltin(LexemeCache &cache, ByteCode &data, const std::string &lexeme, NativeFunction func, size_t paramCount, bool variadic) {
   Function function;
   function.init = true;
   function.type = NATIVE_FUNCTION;
   function.reserved = true;
   function.variadic = variadic;
   function.params.resize(paramCount);
   function.nativeFunction = func;
   data.functions[cacheLexeme(cache, lexeme)] = function;
}

void defineStandardBuiltins(LexemeCache &cache, ByteCode &data) {
   data.functions.resize(getLexemeCount(cache) + 2); // reserved built-ins might overflow, so adjust for that

   // misc, temp
   pushBuiltin(cache, data, "move", builtinMove, 2, false);
   pushBuiltin(cache, data, "add", builtinAdd, 3, true);
   pushBuiltin(cache, data, "print", builtinPrint, 1, true);

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

         if (data.functions[position].init) {
            Function &definition = data.functions[position];
            const char *type = (definition.type == NATIVE_FUNCTION ? "Native function" : (definition.type == LABEL ? "Label" : "Function"));
            if (data.functions[position].reserved) {
               error(diagnostics, std::format("'{}' is a reserved built-in. You cannot redefine it", getLexeme(cache, position)), tokens[i].fileLexeme, tokens[i].line);
               return; // you fucked up hard here
            }
            else {
               warn(diagnostics, std::format("{} '{}' redefined", type, getLexeme(cache, position)), tokens[i].fileLexeme, tokens[i].line);
            }
         }
         Function function;
         function.init = true;
         function.type = (tokens[i + 1].type == TOKEN_LABEL ? LABEL : FUNCTION);
         data.functions[position] = function;
      }
   }

   // real parsing
   bool firstFunction = true;
   size_t returnLexeme = cacheLexeme(cache, "return");

   for (size_t i = 0; i < size && tokens[i].type != TOKEN_EOF; ++i) {
      // skip extraneous newlines
      while (i < tokens.size() && tokens[i].type == TOKEN_NEWLINE) ++i;
      if (tokens[i].type == TOKEN_EOF) break;

      // labels
      if (tokens[i].type == TOKEN_IDENTIFIER && tokens[i + 1].type == TOKEN_LABEL) {
         size_t start = i;
         Function &label = data.functions[tokens[i].lexeme];
         label.label = data.code.size();

         i += 2;
         if (i >= size || tokens[i].type != TOKEN_NEWLINE) {
            error(diagnostics, "Excess tokens (or EOF) after label", tokens[start].fileLexeme, tokens[start].line);
         }
      }
      // function declarations
      else if (tokens[i].type == TOKEN_IDENTIFIER && tokens[i + 1].type == TOKEN_L_PAREN) {
         if (!firstFunction && (data.code.empty() || data.code.back().lexeme != returnLexeme)) {
            data.code.emplace_back(returnLexeme, tokens[i-1].fileLexeme, tokens[i-1].line, std::vector<Value>{});
         }
         firstFunction = false;

         size_t start = i;
         Function &function = data.functions[tokens[i].lexeme];
         bool variadic = false;

         for (i += 2; i < size && tokens[i].type != TOKEN_EOF && tokens[i].type != TOKEN_R_PAREN; ++i) {
            if (tokens[i + 1].type == TOKEN_VARIADIC) {
               variadic = true;
               i += 2;
               break;
            }

            if (tokens[i].type != TOKEN_IDENTIFIER) {
               error(diagnostics, std::format("Function parameters: expected Identifier, got {} instead", getTokenName(tokens[i].type)), tokens[i].fileLexeme, tokens[i].line);
            }
            function.params.push_back(tokens[i].lexeme);
         }

         if (variadic && tokens[i].type != TOKEN_R_PAREN) {
            error(diagnostics, std::format("Function parameters: variadic parameter should be at the end of the parameter list"), tokens[i].fileLexeme, tokens[i].line);
         }
         else if (!variadic && tokens[i].type != TOKEN_R_PAREN) {
            error(diagnostics, "Unterminated function parameters", tokens[start].fileLexeme, tokens[start].line);
         }
         function.variadic = variadic;
         function.function = data.code.size();

         i += 1;
         if (i >= size || tokens[i].type != TOKEN_NEWLINE) {
            error(diagnostics, "Excess tokens (or EOF) after function definition", tokens[start].fileLexeme, tokens[start].line);
         }
      }
      // function calls
      else {
         if (tokens[i].type != TOKEN_IDENTIFIER || tokens[i].lexeme >= data.functions.size() || !data.functions[tokens[i].lexeme].init || data.functions[tokens[i].lexeme].type == LABEL) {
            if (tokens[i].type == TOKEN_IDENTIFIER) {
               error(diagnostics, std::format("No such function '{}'", getLexeme(cache, tokens[i].lexeme)), tokens[i].fileLexeme, tokens[i].line);
            }
            else {
               error(diagnostics, std::format("Expected a function call, got {} instead", getTokenName(tokens[i].type)), tokens[i].fileLexeme, tokens[i].line);
            }
            // to not spiral errors out of control
            while (i < size && tokens[i].type != TOKEN_EOF && tokens[i].type != TOKEN_NEWLINE) i += 1;
            i -= 1;
            continue;
         }

         data.code.emplace_back(tokens[i].lexeme, tokens[i].fileLexeme, tokens[i].line, std::vector<Value>{});
         Command &command = data.code.back();

         for (++i; i < size && tokens[i].type != TOKEN_EOF && tokens[i].type != TOKEN_NEWLINE; ++i) {
            Value value;
            value.line = tokens[i].line;
            value.fileLexeme = tokens[i].fileLexeme;
            switch (tokens[i].type) {
            case TOKEN_IDENTIFIER:
               value.type = VALUE_IDENTIFIER;
               value.identifier = tokens[i].lexeme;
               break;
            case TOKEN_INTEGER:
               value.type = VALUE_INTEGER;
               try {
                  value.integer = std::stol(getLexeme(cache, tokens[i].lexeme));
               }
               catch (...) {
                  value.integer = 0;
                  error(diagnostics, std::format("Invalid integer: {}", getLexeme(cache, tokens[i].lexeme)), value.fileLexeme, value.line);
               }
               break;
            case TOKEN_FLOATING:
               value.type = VALUE_FLOATING;
               try {
                  value.floating = std::stod(getLexeme(cache, tokens[i].lexeme));
               }
               catch (...) {
                  value.floating = 0;
                  error(diagnostics, std::format("Invalid floating point number: {}", getLexeme(cache, tokens[i].lexeme)), value.fileLexeme, value.line);
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
                  error(diagnostics, std::format("Invalid register: {}${}", tokens[i].type == TOKEN_RETURN_REGISTER ? "R" : "", getLexeme(cache, tokens[i].lexeme)), value.fileLexeme, value.line);
               }
               break;
            default:
               error(diagnostics, std::format("Unexpected token {} in function call", getLexeme(cache, tokens[i].lexeme)), value.fileLexeme, value.line);
            }
            command.args.push_back(value);
         }
      }
   }
   if (!tokens.empty()) {
      data.code.emplace_back(returnLexeme, tokens.back().fileLexeme, tokens.back().line, std::vector<Value>{});
   }
}

// executor

// execute bytecode. it's really simple - execute the function that the pointer is on. return and call logic can be found
// in the builtin header since they're also callable functions.
void callPILFunction(Executor &executor, const std::string &name, ErrorSeverity stopSeverity) {
   size_t lexeme = cacheLexeme(executor.cache, name);
   if (lexeme >= executor.code.functions.size() || !executor.code.functions[lexeme].init || executor.code.functions[lexeme].type == NATIVE_FUNCTION) {
      error(executor.diagnostics, std::format("Function '{}' cannot be called as it is not defined", name), 0, 0);
      return;
   }

   if (!executor.code.functions[lexeme].params.empty() || executor.code.functions[lexeme].variadic) {
      error(executor.diagnostics, std::format("Attempted to call function '{}' with 0 arguments", name), 0, 0);
      return;
   }

   executor.stackTrace = std::stack<Trace>();
   executor.pointer = executor.code.functions[lexeme].function;
   executor.returnCount = 0;
   executor.exitCalled = false;

   while (true) {
      Command &command = executor.code.code[executor.pointer];
      Function &function = executor.code.functions[command.lexeme];
      size_t args = command.args.size();
      size_t params = function.params.size();
      bool variadic = function.variadic;

      if ((!variadic && args != params) || (variadic && args < params)) {
         error(executor.diagnostics, std::format("Function expected {}{} parameters, but received {} arguments", (variadic ? ">" : ""), params, args), command.file, command.line);
      }
      else {
         if (function.type == NATIVE_FUNCTION) {
            function.nativeFunction(command, executor);
         }
         else if (function.type == FUNCTION) {
            executor.stackTrace.emplace(executor.pointer, command.lexeme, 0);
            executor.pointer = function.function;
            continue;
         }
         else {
            error(executor.diagnostics, std::format("Stray label '{}'", getLexeme(executor.cache, command.lexeme)), command.file, command.line);
         }
      }

      if (executor.exitCalled || shouldError(executor.diagnostics, stopSeverity)) {
         break;
      }
      executor.pointer += 1;
   }
}
