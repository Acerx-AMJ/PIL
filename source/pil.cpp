#include "builtin.hpp"
#include "pil.hpp"
#include <algorithm>
#include <format>
#include <fstream>
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
         line += 1;
      }
      else if (ch == '(') {
         tokens.emplace_back(TOKEN_L_PAREN, cacheLexeme(cache, "("), file.lexeme, line);
      }
      else if (ch == ')') {
         tokens.emplace_back(TOKEN_R_PAREN, cacheLexeme(cache, ")"), file.lexeme, line);
      }
      else if (ch == ';') {
         while (i < size && file.code[i] != '\n') i += 1;
         line += 1;
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
   for (size_t i = 0; i < tokens.size(); i += 2) {
      if (tokens[i].type != TOKEN_IDENTIFIER || i + 1 >= tokens.size() || tokens[i + 1].type != TOKEN_STRING || getLexeme(cache, tokens[i].lexeme) != "include") {
         i -= 1;
         continue;
      }

      // destroy all include statements after the loop
      tokens[i].parsed = true;
      tokens[i + 1].parsed = true;

      std::string &filename = getLexeme(cache, tokens[i + 1].lexeme);
      if (includedFiles.find(filename) != includedFiles.end()) {
         continue;
      }

      includedFiles.insert(filename);
      PILFile newFile = readPILInternal(diagnostics, cache, filename, tokens[i + 1].fileLexeme, tokens[i + 1].line);
      std::vector<Token> newTokens = lexPILFile(diagnostics, cache, newFile);
      tokens.insert(tokens.begin() + i + 2, newTokens.begin(), newTokens.end());
   }
   // erase all includes and EOFs
   tokens.erase(std::remove_if(tokens.begin(), tokens.end(), [](const Token &t) { return t.parsed || t.type == TOKEN_EOF; }), tokens.end());

   size_t EOFline = (tokens.empty() ? 1 : tokens.back().line);
   tokens.emplace_back(TOKEN_EOF, cacheLexeme(cache, "EOF"), file.lexeme, EOFline);
}

// built-in functions

// we only define built-in functions that actually get used. thanks, cache.
void pushBuiltin(LexemeCache &cache, FunctionData &data, const std::string &lexeme, NativeFunction func) {
   if (auto it = cache.lexemeCache.find(lexeme); it != cache.lexemeCache.end()) {
      Function function;
      function.init = true;
      function.native = true;
      function.nativeFunction = func;
      data.functionMap[it->second] = function;
   }
}

void defineStandardBuiltins(LexemeCache &cache, FunctionData &data) {
   data.functionMap.resize(getLexemeCount(cache));

   pushBuiltin(cache, data, "add", builtinAdd);
   pushBuiltin(cache, data, "sub", builtinSub);
   pushBuiltin(cache, data, "print", builtinPrint);
}

// parser

// take the tokens and turn them into executable function blocks and commands. we have 3 levels here: file -> functions ->
// commands. there can be no commands in the file level and no functions in the command level.
void parsePIL(Diagnostics &diagnostics, LexemeCache &cache, FunctionData &data, std::vector<Token> &tokens) {
   // function prepass
   size_t functionCount = std::count_if(tokens.begin(), tokens.end(), [](const Token &t) { return t.type == TOKEN_L_PAREN; });
   data.blocks.reserve(functionCount);

   for (size_t i = 0; i < tokens.size(); ++i) {
      if (tokens[i].type == TOKEN_IDENTIFIER && tokens[i + 1].type == TOKEN_L_PAREN) {
         size_t position = tokens[i].lexeme;

         if (data.functionMap[position].init) {
            Function &definition = data.functionMap[position];
            if (definition.native) {
               warn(diagnostics, std::format("Native function '{}' redefined", getLexeme(cache, position)), tokens[i].fileLexeme, tokens[i].line);
            }
            else {
               size_t tokenPos = data.blocks[definition.function].tokenPosition;
               Token &token = tokens[tokenPos];
               warn(diagnostics, std::format("Function '{}' at {}:{} redefined", getLexeme(cache, token.lexeme), getLexeme(cache, token.fileLexeme), token.line), tokens[i].fileLexeme, tokens[i].line);
            }
         }
         Function function;
         function.init = true;
         function.native = false;
         function.function = data.blocks.size();
         data.functionMap[position] = function;

         data.blocks.emplace_back(tokens[i].lexeme, i);
      }
   }

   // real parsing
   for (Block &block: data.blocks) {
      size_t start = block.tokenPosition;
      tokens[start].parsed = true;
      tokens[start + 1].parsed = true;

      for (start += 2; start < tokens.size() && tokens[start].type != TOKEN_EOF && tokens[start].type != TOKEN_R_PAREN; ++start) {
         if (tokens[start].type != TOKEN_IDENTIFIER) {
            error(diagnostics, std::format("Function parameters: expected Identifier, got {} instead", getTokenName(tokens[start].type)), tokens[start].fileLexeme, tokens[start].line);
         }
         tokens[start].parsed = true;
         block.params.push_back(tokens[start].lexeme);
      }

      if (tokens[start].type != TOKEN_R_PAREN) {
         error(diagnostics, "Unterminated function parameters", tokens[block.tokenPosition].fileLexeme, tokens[block.tokenPosition].line);
         continue;
      }

      for (++start; start < tokens.size() && tokens[start].type != TOKEN_EOF;) {
         size_t startLexeme = tokens[start].lexeme;
         if (tokens[start].type == TOKEN_IDENTIFIER && tokens[start + 1].type == TOKEN_L_PAREN && startLexeme < data.functionMap.size() && data.functionMap[startLexeme].init) {
            break; // got to the next function declaration, end of body
         }

         if (tokens[start].type != TOKEN_IDENTIFIER || startLexeme >= data.functionMap.size() || !data.functionMap[startLexeme].init) {
            error(diagnostics, std::format("Expected a function call, got {} instead", getTokenName(tokens[start].type)), tokens[start].fileLexeme, tokens[start].line);
            start += 1;
            continue;
         }

         Command command;
         command.lexeme = tokens[start].lexeme;
         for (++start; start < tokens.size() && tokens[start].type != TOKEN_EOF && (tokens[start].type != TOKEN_IDENTIFIER || tokens[start].lexeme >= data.functionMap.size() || !data.functionMap[tokens[start].lexeme].init); ++start) {
            Value value;
            value.line = tokens[start].line;
            value.fileLexeme = tokens[start].fileLexeme;
            switch (tokens[start].type) {
            case TOKEN_IDENTIFIER:
               value.type = VALUE_IDENTIFIER;
               value.identifier = tokens[start].lexeme;
               break;
            case TOKEN_INTEGER:
               value.type = VALUE_INTEGER;
               try {
                  value.integer = std::stol(getLexeme(cache, tokens[start].lexeme));
               }
               catch (...) {
                  value.integer = 0;
                  error(diagnostics, std::format("Invalid integer: {}", getLexeme(cache, tokens[start].lexeme)), value.fileLexeme, value.line);
               }
               break;
            case TOKEN_FLOATING:
               value.type = VALUE_FLOATING;
               try {
                  value.floating = std::stod(getLexeme(cache, tokens[start].lexeme));
               }
               catch (...) {
                  value.floating = 0;
                  error(diagnostics, std::format("Invalid floating point number: {}", getLexeme(cache, tokens[start].lexeme)), value.fileLexeme, value.line);
               }
               break;
            case TOKEN_STRING:
               value.type = VALUE_STRING;
               value.string = tokens[start].lexeme;
               break;
            case TOKEN_CHARACTER:
               value.type = VALUE_CHARACTER;
               value.character = getLexeme(cache, tokens[start].lexeme).front();
               break;
            case TOKEN_REGISTER:
               value.type = VALUE_REGISTER;
               try {
                  value.reg = std::stoull(getLexeme(cache, tokens[start].lexeme));
               }
               catch (...) {
                  value.reg = 0;
                  error(diagnostics, std::format("Invalid register: ${}", getLexeme(cache, tokens[start].lexeme)), value.fileLexeme, value.line);
               }
               break;
            default:
               error(diagnostics, std::format("Unexpected token {} in function call", getLexeme(cache, tokens[start].lexeme)), value.fileLexeme, value.line);
            }
            tokens[start].parsed = true;
            command.values.push_back(value);
         }
         block.commands.push_back(command);
      }
   }
}
