#include "parser.hpp"
#include <unordered_map>

// constants
static const std::unordered_map<char, char> escapeCodeMap {
   {'a', '\a'}, {'b', '\b'}, {'t', '\t'}, {'n', '\n'}, {'v', '\v'}, {'f', '\f'},
   {'r', '\r'}, {'e', '\e'}, {'\\', '\\'}, {'\'', '\''}, {'"', '"'}
};

// lexer
void Parser::lex(const std::string &code) {
   for (size_t i = 0; i < code.size(); ++i) {
      char ch = code[i];

      if (std::isspace(ch)) {
         tokenLine += (ch == '\n');
      }
      else if (ch == '(') {
         insertToken(TOKEN_L_PAREN, "(");
      }
      else if (ch == ')') {
         insertToken(TOKEN_R_PAREN, ")");
      }
      else if (ch == '&') {
         insertToken(TOKEN_REFERENCE, "&");
      }
      else if (ch == '*') {
         insertToken(TOKEN_DEREFERENCE, "*");
      }
      else if (ch == ',') {
         insertToken(TOKEN_COMMA, ",");
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
         insertToken(TOKEN_REGISTER, reg);
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
         insertToken(TOKEN_CHARACTER, ch);
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
            string.push_back(code[i]);
            tokenLine += (code[i] == '\n');
         }
         insertToken(TOKEN_STRING, string);
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
         insertToken(dot ? TOKEN_FLOATING : TOKEN_INTEGER, number);
         i -= 1;
      }
      else if (ch == '_' || std::isalpha(ch)) {
         std::string identifier;
         size_t end = i;

         for (++end; end < code.size() && (code[end] == '_' || code[end] == '-' || std::isalnum(code[end])); ++end);
         identifier = code.substr(i, end - i);
         insertToken(TOKEN_IDENTIFIER, identifier);
         i = end - 1;
      }
      else {
         printf("Unexpected character '%c' at line %llu.\n", ch, tokenLine);
         exit(EXIT_FAILURE);
      }
   }
   insertToken(TOKEN_EOF, "EOF");
}

void Parser::insertToken(TokenType type, const std::string &lexeme) {
   tokens.emplace_back(type, lexeme, tokenLine);
}

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
