#include "lexemes.hpp"
#include "parser.hpp"
#include <algorithm>
#include <unordered_map>

// lexer

// translate code into tokens. we cache common lexemes that repeat often like identifiers and ops but don't cache numbers,
// characters and strings, which could change during execution and are usually longer and don't repeat as often. Registers
// are safe to cache since they're constants
void Parser::lex(const std::string &code) {
   for (size_t i = 0; i < code.size(); ++i) {
      char ch = code[i];

      if (std::isspace(ch)) {
         tokenLine += (ch == '\n');
      }
      else if (ch == '(') {
         tokens.emplace_back(TOKEN_L_PAREN, cacheLexeme("("), tokenLine);
      }
      else if (ch == ')') {
         tokens.emplace_back(TOKEN_R_PAREN, cacheLexeme(")"), tokenLine);
      }
      else if (ch == '&') {
         tokens.emplace_back(TOKEN_REFERENCE, cacheLexeme("&"), tokenLine);
      }
      else if (ch == '*') {
         tokens.emplace_back(TOKEN_DEREFERENCE, cacheLexeme("*"), tokenLine);
      }
      else if (ch == ',') {
         tokens.emplace_back(TOKEN_COMMA, cacheLexeme(","), tokenLine);
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
         tokens.emplace_back(TOKEN_REGISTER, cacheLexeme(reg), tokenLine);
         i -= 1;
      }
      else if (ch == '\'') {
         std::string ch = "_";
         i += 1;
         if (i >= code.size()) {
            printf("Unterminated character at line %llu.\n", tokenLine);
            exit(EXIT_FAILURE);
         }
         ch[0] = handleEscapeCode(code, i);
         i += 1;

         if (i >= code.size() || code[i] != '\'') {
            printf("Unterminated character at line %llu.\n", tokenLine);
            exit(EXIT_FAILURE);
         }
         tokens.emplace_back(TOKEN_CHARACTER, pushLexeme(ch), tokenLine);
      }
      else if (ch == '"') {
         std::string string;
         size_t end = code.find('"', i + 1);
         if (end == std::string::npos) {
            printf("Unterminated string at line %llu.\n", tokenLine);
            exit(EXIT_FAILURE);
         }

         string.reserve(end - i - 1);
         for (++i; i < code.size() && code[i] != '"'; ++i) {
            string.push_back(handleEscapeCode(code, i));
            tokenLine += (code[i] == '\n');
         }
         tokens.emplace_back(TOKEN_STRING, pushLexeme(string), tokenLine);
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
                  printf("Number '%s' contains multiple decimal points at line %llu.\n", number.c_str(), tokenLine);
                  exit(EXIT_FAILURE);
               }
               dot = true;
            }
         }
         tokens.emplace_back(dot ? TOKEN_FLOATING : TOKEN_INTEGER, pushLexeme(number), tokenLine);
         i -= 1;
      }
      else if (ch == '_' || std::isalpha(ch)) {
         std::string identifier;
         size_t end = i;

         for (++end; end < code.size() && (code[end] == '_' || code[end] == '-' || std::isalnum(code[end])); ++end);
         identifier = code.substr(i, end - i);
         std::transform(identifier.begin(), identifier.end(), identifier.begin(), tolower);
         tokens.emplace_back(TOKEN_IDENTIFIER, pushLexeme(identifier), tokenLine);
         i = end - 1;
      }
      else {
         printf("Unexpected character '%c' at line %llu.\n", ch, tokenLine);
         exit(EXIT_FAILURE);
      }
   }
   // no need to cache a one-time token
   tokens.emplace_back(TOKEN_EOF, pushLexeme("EOF"), tokenLine);
}

static const std::unordered_map<char, char> escapeCodeMap {
   {'a', '\a'}, {'b', '\b'}, {'t', '\t'}, {'n', '\n'}, {'v', '\v'}, {'f', '\f'},
   {'r', '\r'}, {'e', '\e'}, {'\\', '\\'}, {'\'', '\''}, {'"', '"'}
};

char Parser::handleEscapeCode(const std::string &code, size_t &i) {
   if (code[i] != '\\') {
      return code[i];
   }
   i += 1;
   if (auto it = escapeCodeMap.find(code[i]); it != escapeCodeMap.end()) {
      return it->second;
   }
   printf("Unknown escape code '\\%c' at line %llu.\n", code[i], tokenLine);
   exit(EXIT_FAILURE);
}

// parser
void Parser::parse() {

}
