#include "builtin.hpp"
#include "lexemes.hpp"
#include "parser.hpp"
#include <algorithm>
#include <fstream>
#include <unordered_map>
#include <unordered_set>

// helpers
static const std::unordered_map<char, char> escapeCodeMap {
   {'a', '\a'}, {'b', '\b'}, {'t', '\t'}, {'n', '\n'}, {'v', '\v'}, {'f', '\f'},
   {'r', '\r'}, {'e', '\e'}, {'\\', '\\'}, {'\'', '\''}, {'"', '"'}
};

char handleEscapeCode(const std::string &code, size_t &i, size_t fileLexeme, size_t tokenLine) {
   if (code[i] != '\\') {
      return code[i];
   }
   i += 1;
   if (auto it = escapeCodeMap.find(code[i]); it != escapeCodeMap.end()) {
      return it->second;
   }
   printf("Unknown escape code '\\%c' at %s:%llu.\n", code[i], getLexeme(fileLexeme).c_str(), tokenLine);
   exit(EXIT_FAILURE);
}

// lexer

// translate code into tokens. we cache common lexemes that repeat often like identifiers and ops but don't cache numbers,
// characters and strings, which could change during execution and are usually longer and don't repeat as often. Registers
// are safe to cache since they're constants
void Parser::lex(const std::string &code, size_t fileLexeme) {
   lexerFileLexeme = fileLexeme;
   for (size_t i = 0; i < code.size(); ++i) {
      char ch = code[i];

      if (ch == '\n') {
         tokens.emplace_back(TOKEN_NEWLINE, cacheLexeme("NL"), fileLexeme, tokenLine);
         tokenLine += 1;
      }
      else if (ch == '(') {
         tokens.emplace_back(TOKEN_L_PAREN, cacheLexeme("("), fileLexeme, tokenLine);
      }
      else if (ch == ')') {
         tokens.emplace_back(TOKEN_R_PAREN, cacheLexeme(")"), fileLexeme, tokenLine);
      }
      else if (ch == ';') {
         while (i < code.size() && code[i] != '\n') i += 1;
         tokenLine += 1;
      }
      else if (ch == '$') {
         std::string reg;
         for (++i; i < code.size() && std::isdigit(code[i]); ++i) {
            reg.push_back(code[i]);
         }
         tokens.emplace_back(TOKEN_REGISTER, cacheLexeme(reg), fileLexeme, tokenLine);
         i -= 1;
      }
      else if (ch == '\'') {
         std::string ch = "_";
         i += 1;
         if (i >= code.size()) {
            printf("Unterminated character at %s:%llu.\n", getLexeme(fileLexeme).c_str(), tokenLine);
            exit(EXIT_FAILURE);
         }
         ch[0] = handleEscapeCode(code, i, fileLexeme, tokenLine);
         i += 1;

         if (i >= code.size() || code[i] != '\'') {
            printf("Unterminated character at %s:%llu.\n", getLexeme(fileLexeme).c_str(), tokenLine);
            exit(EXIT_FAILURE);
         }
         tokens.emplace_back(TOKEN_CHARACTER, pushLexeme(ch), fileLexeme, tokenLine);
      }
      else if (ch == '"') {
         std::string string;
         size_t end = code.find('"', i + 1);
         if (end == std::string::npos) {
            printf("Unterminated string at %s:%llu.\n", getLexeme(fileLexeme).c_str(), tokenLine);
            exit(EXIT_FAILURE);
         }

         string.reserve(end - i - 1);
         for (++i; i < code.size() && code[i] != '"'; ++i) {
            string.push_back(handleEscapeCode(code, i, fileLexeme, tokenLine));
            tokenLine += (code[i] == '\n');
         }
         tokens.emplace_back(TOKEN_STRING, pushLexeme(string), fileLexeme, tokenLine);
      }
      else if (ch == '-' || ch == '.' || std::isdigit(ch)) {
         std::string number;
         size_t end = code.find_first_not_of(".1234567890", i + 1);
         if (end == std::string::npos) {
            end = code.size();
         }
         number.reserve(end - i - 1);

         if (ch == '-') {
            number.push_back(ch);
            i += 1;
         }
         bool dot = false;

         for (; i < code.size() && (code[i] == '.' || std::isdigit(code[i])); ++i) {
            number.push_back(code[i]);
            if (code[i] == '.') {
               if (dot) {
                  printf("Number '%s' contains multiple decimal points at %s:%llu.\n", number.c_str(), getLexeme(fileLexeme).c_str(), tokenLine);
                  exit(EXIT_FAILURE);
               }
               dot = true;
            }
         }
         tokens.emplace_back(dot ? TOKEN_FLOATING : TOKEN_INTEGER, pushLexeme(number), fileLexeme, tokenLine);
         i -= 1;
      }
      else if (ch == '_' || std::isalpha(ch)) {
         std::string identifier;
         size_t end = i;

         for (++end; end < code.size() && (code[end] == '_' || code[end] == '-' || std::isalnum(code[end])); ++end);
         identifier = code.substr(i, end - i);
         std::transform(identifier.begin(), identifier.end(), identifier.begin(), tolower);
         tokens.emplace_back(TOKEN_IDENTIFIER, pushLexeme(identifier), fileLexeme, tokenLine);
         i = end - 1;
      }
      else if (!std::isspace(ch) && ch != ',') {
         printf("Unexpected character '%c' at %s:%llu.\n", ch, getLexeme(fileLexeme).c_str(), tokenLine);
         exit(EXIT_FAILURE);
      }
   }
}

// translator (handle includes)

// find all INCLUDE "FILE" statements and push their tokens if the files haven't been included yet. will erase all includes
// after and doesn't have more than a single file open at a time.
void Parser::handleIncludes() {
   std::unordered_set<std::string> includedFiles;
   for (size_t i = 0; i < tokens.size(); ++i) {
      if (tokens[i].type != TOKEN_IDENTIFIER || tokens[i + 1].type != TOKEN_STRING || getLexeme(tokens[i].lexeme) != "include") {
         continue;
      }

      std::string &filename = getLexeme(tokens[i + 1].lexeme);
      if (tokens[i + 2].type != TOKEN_NEWLINE) {
         printf("Expected a newline after include statement at %s:%llu.\n", filename.c_str(), tokens[i].line);
         exit(EXIT_FAILURE);
      }

      // destroy all include statements after the loop
      tokens[i].parsed = true;
      tokens[i + 1].parsed = true;
      tokens[i + 2].parsed = true;
      if (includedFiles.find(filename) != includedFiles.end()) {
         continue;
      }

      includedFiles.insert(filename);
      std::ifstream file (filename);
      if (!file.is_open()) {
         printf("Could not include file '%s' at %llu.\n", filename.c_str(), tokens[i].line);
         exit(EXIT_FAILURE);
      }
      std::string code (std::istreambuf_iterator<char>(file), {});
      file.close();

      Parser parser;
      parser.lex(code, tokens[i + 1].lexeme);
      tokens.insert(tokens.begin() + i + 3, parser.tokens.begin(), parser.tokens.end());
      i += 2; // incremented by an extra one in the loop
   }
   // erase all includes
   tokens.erase(std::remove_if(tokens.begin(), tokens.end(), [](const Token &t) { return t.parsed; }), tokens.end());
   tokens.emplace_back(TOKEN_EOF, pushLexeme("EOF"), lexerFileLexeme, tokenLine);
}

// built-in functions

// we only define built-in functions that actually get used. thanks, cache.
void Parser::pushBuiltin(const std::string &lexeme, NativeFunction func) {
   if (auto it = lexemeCache.find(lexeme); it != lexemeCache.end()) {
      Function function;
      function.native = true;
      function.nativeFunction = func;
      functionMap[it->second] = function;
   }
}

void Parser::defineBuiltins() {
   functionMap.resize(getLexemeCount());

   pushBuiltin("add", builtinAdd);
   pushBuiltin("sub", builtinSub);
   pushBuiltin("print", builtinPrint);
}

// parser

// take the tokens and turn them into executable function blocks and commands. we have 3 levels here: file -> functions ->
// commands. there can be no commands in the file level and no functions in the command level.
void Parser::parse() {
   // function prepass
   size_t functionCount = std::count_if(tokens.begin(), tokens.end(), [](const Token &t) { return t.type == TOKEN_L_PAREN; });
   blocks.reserve(functionCount);

   for (size_t i = 0; i < tokens.size(); ++i) {
      if (tokens[i].type == TOKEN_IDENTIFIER && tokens[i + 1].type == TOKEN_L_PAREN) {
         size_t position = tokens[i].lexeme;
         size_t size = functionMap.size();
         if (position >= size) {
            // try to do the normal vector allocation (2X size) or just use position if that's erroneous
            functionMap.resize(position >= size * 2 || size == 0 ? position : size);
         }
         Function function;
         function.native = false;
         function.function = position;
         functionMap[i] = function;

         blocks.emplace_back(tokens[i].lexeme, i);
      }
   }

   // real parsing
}
