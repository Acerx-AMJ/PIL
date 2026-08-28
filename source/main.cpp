#include "parser.hpp"
#include <filesystem>
#include <fstream>

int main(int argc, char *argv[]) {
   if (argc != 2) {
      printf("Expected single argument.\n");
      exit(EXIT_FAILURE);
   }

   std::string code = argv[1];
   if (std::filesystem::exists(code) && std::filesystem::is_regular_file(code)) {
      std::ifstream file (code);
      if (!file.is_open()) {
         printf("You don't have permission to run this file.\n");
         exit(EXIT_FAILURE);
      }
      code = std::string(std::istreambuf_iterator<char>(file), {});
      file.close();
   }

   Parser parser;
   parser.lex(code);

   for (Token &token: parser.tokens) {
      printf("%5llu %s: '%s'.\n", token.line, getTokenName(token.type), token.lexeme.c_str());
   }
}
