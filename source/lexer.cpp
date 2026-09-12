#include "pil.hpp"
#include <algorithm>
#include <fstream>
#include <unordered_set>

// helpers

static const std::unordered_map<char, char> escapeCodeMap {
   {'a', '\a'}, {'b', '\b'}, {'t', '\t'}, {'n', '\n'}, {'v', '\v'}, {'f', '\f'},
   {'r', '\r'}, {'e', '\e'}, {'\\', '\\'}, {'\'', '\''}, {'"', '"'}
};

inline char handleEscapeCode(Diagnostics &diagnostics, LexemeCache &cache, PILFile &file, size_t &i, size_t tokenLine) {
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

constexpr bool isDigit(char c) { return c >= '0' && c <= '9'; }
constexpr bool isAlpha(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }
constexpr bool isSpace(char c) { return c == ' ' || c == '\t' || c == '\r' || c == '\f' || c == '\v'; }
constexpr bool isAlnum(char c) { return isDigit(c) || isAlpha(c); }
constexpr char toLower(char c) { return (c >= 'A' && c <= 'Z' ? c + ('a' - 'A') : c); }

// file reader
void readPIL(Diagnostics &diagnostics, LexemeCache &cache, const std::string &path, PILFile &pilFile, size_t parentFile, size_t line) {
   size_t fileLexeme = pushLexeme(cache, path);
   std::ifstream file (path, std::ios::binary | std::ios::ate);
   if (!file.is_open()) {
      error(diagnostics, parentFile, line, "Could not read file '%s'", path.c_str());
      return;
   }
   auto fileSize = file.tellg();
   file.seekg(std::ios::beg);
   pilFile.code.resize(fileSize);
   file.read(&pilFile.code[0], fileSize);
   pilFile.lexeme = fileLexeme;
}

// translate code into tokens. we cache common lexemes that repeat often like identifiers and ops but don't cache numbers,
// characters and strings, which could change during execution and are usually longer and don't repeat as often. Registers
// are safe to cache since they're constants
void lexPILFile(Diagnostics &diagnostics, LexemeCache &cache, PILFile &file, std::vector<Token> &tokens) {
   size_t size = file.code.size();
   size_t line = 1;
   size_t emptyLexeme = cacheLexeme(cache, "");
   tokens.reserve(size / 4);

   for (size_t i = 0; i < size; ++i) {
      char ch = file.code[i];

      switch (ch) {
      case '\n': tokens.emplace_back(TOKEN_NEWLINE, emptyLexeme, file.lexeme, line); line += 1; continue;
      case '(': tokens.emplace_back(TOKEN_L_PAREN, emptyLexeme, file.lexeme, line); continue;
      case ')': tokens.emplace_back(TOKEN_R_PAREN, emptyLexeme, file.lexeme, line); continue;
      case ':': tokens.emplace_back(TOKEN_LABEL, emptyLexeme, file.lexeme, line); continue;
      case '[': tokens.emplace_back(TOKEN_L_BRACKET, emptyLexeme, file.lexeme, line); continue;
      case ']': tokens.emplace_back(TOKEN_R_BRACKET, emptyLexeme, file.lexeme, line); continue;
      case '+': tokens.emplace_back(TOKEN_PLUS, emptyLexeme, file.lexeme, line); continue;
      case '-': tokens.emplace_back(TOKEN_MINUS, emptyLexeme, file.lexeme, line); continue;
      case '*': tokens.emplace_back(TOKEN_STAR, emptyLexeme, file.lexeme, line); continue;
      case '/': tokens.emplace_back(TOKEN_SLASH, emptyLexeme, file.lexeme, line); continue;
      case '%': tokens.emplace_back(TOKEN_PERCENT, emptyLexeme, file.lexeme, line); continue;
      case '^': tokens.emplace_back(TOKEN_CARET, emptyLexeme, file.lexeme, line); continue;
      }

      if (i + 2 < size && ch == '.' && file.code[i+1] == '.' && file.code[i+2] == '.') {
         tokens.emplace_back(TOKEN_VARIADIC, emptyLexeme, file.lexeme, line);
         i += 2;
      }
      else if (ch == ';') {
         while (i < size && file.code[i] != '\n') i += 1;
         tokens.emplace_back(TOKEN_NEWLINE, emptyLexeme, file.lexeme, line);
         line += 1;
      }
      else if ((ch == 'r' || ch == 'R') && i + 1 < size && file.code[i + 1] == '$') {
         std::string reg;
         for (i += 2; i < size && isDigit(file.code[i]); ++i) {
            reg.push_back(file.code[i]);
         }
         tokens.emplace_back(TOKEN_RETURN_REGISTER, cacheLexeme(cache, reg), file.lexeme, line);
         i -= 1;
      }
      else if (ch == '$') {
         std::string reg;
         for (++i; i < size && isDigit(file.code[i]); ++i) {
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
         tokens.emplace_back(TOKEN_CHARACTER, cacheLexeme(cache, ch), file.lexeme, line);
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
      else if (isDigit(ch)) {
         std::string number;
         bool dot = false;

         for (; i < size && (file.code[i] == '.' || isDigit(file.code[i])); ++i) {
            number.push_back(file.code[i]);
            if (file.code[i] == '.') {
               if (dot) {
                  error(diagnostics, file.lexeme, line, "Number '%s' contains multiple decimal points", number.c_str());
                  break;
               }
               dot = true;
            }
         }
         tokens.emplace_back(dot ? TOKEN_FLOATING : TOKEN_INTEGER, cacheLexeme(cache, number), file.lexeme, line);
         i -= 1;
      }
      else if (ch == '_' || isAlpha(ch)) {
         std::string identifier;
         size_t end = i;

         for (++end; end < size && (file.code[end] == '_' || file.code[end] == '-' || file.code[end] == '.' || isAlnum(file.code[end])); ++end);
         identifier = file.code.substr(i, end - i);
         std::transform(identifier.begin(), identifier.end(), identifier.begin(), toLower);
         tokens.emplace_back(TOKEN_IDENTIFIER, cacheLexeme(cache, identifier), file.lexeme, line);
         i = end - 1;
      }
      else if (!isSpace(ch) && ch != ',') {
         error(diagnostics, file.lexeme, line, "Unexpected character '%c'", ch);
      }
   }
   tokens.emplace_back(TOKEN_EOF, cacheLexeme(cache, "EOF"), file.lexeme, line);
}

// find all INCLUDE "FILE" statements and push their tokens if the files haven't been included yet. will erase all includes
// after and doesn't have more than a single file open at a time. also handles some other misc. directives.
void translatePIL(Executor &executor, PILFile &file, std::vector<Token> &tokens) {
   std::unordered_set<std::string> includedFiles;
   size_t size = tokens.size();

   size_t includeLexeme = cacheLexeme(executor.cache, "include");
   size_t registerLexeme = cacheLexeme(executor.cache, "register-size");
   size_t returnRegisterLexeme = cacheLexeme(executor.cache, "return-register-size");

   for (size_t i = 0; i < size; ++i) {
      if (tokens[i].type != TOKEN_IDENTIFIER) continue;
      
      // handle includes
      if (tokens[i].lexeme == includeLexeme && i + 1 < size && tokens[i + 1].type == TOKEN_STRING) {
         if (i + 2 >= size || tokens[i + 2].type != TOKEN_NEWLINE) {
            error(executor.diagnostics, tokens[i].file, tokens[i].line, "Excess tokens (or EOF) after include directive");
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
         PILFile newFile;
         std::vector<Token> newTokens;

         readPIL(executor.diagnostics, executor.cache, filename, newFile, tokens[i + 1].file, tokens[i + 1].line);
         lexPILFile(executor.diagnostics, executor.cache, newFile, newTokens);
         tokens.insert(tokens.begin() + i + 3, newTokens.begin(), newTokens.end());
         i += 2;
      }
      // handle register config
      else if ((tokens[i].lexeme == registerLexeme || tokens[i].lexeme == returnRegisterLexeme) && i + 1 < size && tokens[i + 1].type == TOKEN_INTEGER) {
         if (i + 2 >= size || tokens[i + 2].type != TOKEN_NEWLINE) {
            error(executor.diagnostics, tokens[i].file, tokens[i].line, "Excess tokens (or EOF) after register configuration directive");
            continue;
         }
         tokens[i].parsed = true;
         tokens[i + 1].parsed = true;
         tokens[i + 2].parsed = true;

         Value value = parseToken(executor, tokens[i + 1], {}, {});
         if (tokens[i].lexeme == registerLexeme) {
            executor.registers.resize(value.integer);
         }
         else {
            executor.returnRegisters.resize(value.integer);
         }
         i += 2;
      }
   }
   // erase all includes and EOFs
   tokens.erase(std::remove_if(tokens.begin(), tokens.end(), [](const Token &t) { return t.parsed || t.type == TOKEN_EOF; }), tokens.end());
   size_t EOFline = (tokens.empty() ? 1 : tokens.back().line);
   tokens.emplace_back(TOKEN_EOF, cacheLexeme(executor.cache, "EOF"), file.lexeme, EOFline);
}
