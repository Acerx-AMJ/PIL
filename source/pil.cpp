#include "builtin.hpp"
#include "pil.hpp"
#include <algorithm>
#include <fstream>
#include <unordered_map>
#include <unordered_set>

// lexeme cache
size_t pushLexeme(LexemeCache &cache, const std::string &lexeme) {
   size_t id = cache.lexemes.size();
   cache.lexemes.push_back(lexeme);
   return id;
}

size_t cacheLexeme(LexemeCache &cache, const std::string &lexeme) {
   if (auto it = cache.lexemeCache.find(lexeme); it != cache.lexemeCache.end()) {
      return it->second;
   }
   size_t id = cache.lexemes.size();
   cache.lexemes.push_back(lexeme);
   cache.lexemeCache[lexeme] = id;
   return id;
}

std::string &getLexeme(LexemeCache &cache, size_t id) {
   if (id < 0 || id >= cache.lexemes.size()) {
      printf("Invalid lexeme ID %zu.\n", id);
      exit(EXIT_FAILURE);
   }
   return cache.lexemes[id];
}

size_t getLexemeCount(LexemeCache &cache) {
   return cache.lexemes.size();
}

// file reader
PILFile readPIL(LexemeCache &cache, const std::string &path) {
   size_t fileLexeme = pushLexeme(cache, path);
   std::ifstream file (path);
   if (!file.is_open()) {
      printf("Could not read file '%s'.\n", path.c_str());
      exit(EXIT_FAILURE);
   }
   std::string code (std::istreambuf_iterator<char>(file), {});
   return PILFile{code, fileLexeme};
}

// lexer

// translate code into tokens. we cache common lexemes that repeat often like identifiers and ops but don't cache numbers,
// characters and strings, which could change during execution and are usually longer and don't repeat as often. Registers
// are safe to cache since they're constants
char handleEscapeCode(LexemeCache &cache, PILFile &file, size_t &i, size_t tokenLine) {
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
   printf("Unknown escape code '\\%c' at %s:%zu.\n", ch, getLexeme(cache, file.lexeme).c_str(), tokenLine);
   exit(EXIT_FAILURE);
}

std::vector<Token> lexPILFile(LexemeCache &cache, PILFile &file) {
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
         std::string ch = "_";
         i += 1;
         if (i >= size) {
            printf("Unterminated character at %s:%zu.\n", getLexeme(cache, file.lexeme).c_str(), line);
            exit(EXIT_FAILURE);
         }
         ch[0] = handleEscapeCode(cache, file, i, line);
         i += 1;

         if (i >= size || file.code[i] != '\'') {
            printf("Unterminated character at %s:%zu.\n", getLexeme(cache, file.lexeme).c_str(), line);
            exit(EXIT_FAILURE);
         }
         tokens.emplace_back(TOKEN_CHARACTER, pushLexeme(cache, ch), file.lexeme, line);
      }
      else if (ch == '"') {
         std::string string;
         size_t originalLine = line;
         string.reserve(16);

         for (++i; i < size && file.code[i] != '"'; ++i) {
            string.push_back(handleEscapeCode(cache, file, i, line));
            line += (file.code[i] == '\n');
         }

         if (i >= size || file.code[i] != '"') {
            printf("Unterminated string at %s:%zu.\n", getLexeme(cache, file.lexeme).c_str(), originalLine);
            exit(EXIT_FAILURE);
         }
         tokens.emplace_back(TOKEN_STRING, pushLexeme(cache, string), file.lexeme, originalLine);
      }
      else if (ch == '-' || ch == '.' || std::isdigit(ch)) {
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
                  printf("Number '%s' contains multiple decimal points at %s:%zu.\n", number.c_str(), getLexeme(cache, file.lexeme).c_str(), line);
                  exit(EXIT_FAILURE);
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
         printf("Unexpected character '%c' at %s:%zu.\n", ch, getLexeme(cache, file.lexeme).c_str(), line);
         exit(EXIT_FAILURE);
      }
   }
   tokens.emplace_back(TOKEN_EOF, cacheLexeme(cache, "EOF"), file.lexeme, line);
   return tokens;
}

// translator (handle includes)

// find all INCLUDE "FILE" statements and push their tokens if the files haven't been included yet. will erase all includes
// after and doesn't have more than a single file open at a time.
void handlePILFileIncludes(LexemeCache &cache, PILFile &file, std::vector<Token> &tokens) {
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
      PILFile newFile = readPIL(cache, filename);
      std::vector<Token> newTokens = lexPILFile(cache, newFile);
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
void parsePIL(LexemeCache &cache, FunctionData &data, std::vector<Token> &tokens) {
   // function prepass
   size_t functionCount = std::count_if(tokens.begin(), tokens.end(), [](const Token &t) { return t.type == TOKEN_L_PAREN; });
   data.blocks.reserve(functionCount);

   for (size_t i = 0; i < tokens.size(); ++i) {
      if (tokens[i].type == TOKEN_IDENTIFIER && tokens[i + 1].type == TOKEN_L_PAREN) {
         size_t position = tokens[i].lexeme;

         if (data.functionMap[position].init) {
            Function &definition = data.functionMap[position];
            if (definition.native) {
               printf("Native function '%s' redefined at %s:%zu.\n", getLexeme(cache, position).c_str(), getLexeme(cache, tokens[i].fileLexeme).c_str(), tokens[i].line);
            }
            else {
               size_t tokenPos = data.blocks[definition.function].tokenPosition;
               Token &token = tokens[tokenPos];
               printf("Function '%s' at %s:%zu redefined again with the same name at %s:%zu.\n", getLexeme(cache, token.lexeme).c_str(), getLexeme(cache, token.fileLexeme).c_str(), token.line, getLexeme(cache, tokens[i].fileLexeme).c_str(), tokens[i].line);
            }
            exit(EXIT_FAILURE);
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
            printf("Function parameters: expected Identifier, got %s instead at %s:%zu.\n", getTokenName(tokens[start].type), getLexeme(cache, tokens[start].fileLexeme).c_str(), tokens[start].line);
            exit(EXIT_FAILURE);
         }
         tokens[start].parsed = true;
         block.params.push_back(tokens[start].lexeme);
      }

      if (tokens[start].type != TOKEN_R_PAREN) {
         printf("Unterminated function parameters at %s:%zu.\n", getLexeme(cache, tokens[block.tokenPosition].fileLexeme).c_str(), tokens[block.tokenPosition].line);
         exit(EXIT_FAILURE);
      }

      for (++start; start < tokens.size() && tokens[start].type != TOKEN_EOF;) {
         size_t startLexeme = tokens[start].lexeme;
         if (tokens[start].type == TOKEN_IDENTIFIER && tokens[start + 1].type == TOKEN_L_PAREN && startLexeme < data.functionMap.size() && data.functionMap[startLexeme].init) {
            break; // got to the next function declaration, end of body
         }

         if (tokens[start].type != TOKEN_IDENTIFIER || startLexeme >= data.functionMap.size() || !data.functionMap[startLexeme].init) {
            printf("Expected a function call, got %s instead at %s:%zu.\n", getTokenName(tokens[start].type), getLexeme(cache, tokens[start].fileLexeme).c_str(), tokens[start].line);
            exit(EXIT_FAILURE);
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
                  printf("Invalid integer: %s.\n", getLexeme(cache, tokens[start].lexeme).c_str());
                  exit(EXIT_FAILURE);
               }
               break;
            case TOKEN_FLOATING:
               value.type = VALUE_FLOATING;
               try {
                  value.floating = std::stod(getLexeme(cache, tokens[start].lexeme));
               }
               catch (...) {
                  printf("Invalid floating point number: %s.\n", getLexeme(cache, tokens[start].lexeme).c_str());
                  exit(EXIT_FAILURE);
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
                  printf("Invalid register: $%s.\n", getLexeme(cache, tokens[start].lexeme).c_str());
                  exit(EXIT_FAILURE);
               }
               break;
            default:
               printf("Unexpected token %s in function call at %s:%zu.\n", getTokenName(tokens[start].type), getLexeme(cache, tokens[start].fileLexeme).c_str(), tokens[start].line);
               exit(EXIT_FAILURE);
            }
            tokens[start].parsed = true;
            command.values.push_back(value);
         }
         block.commands.push_back(command);
      }
   }
}
