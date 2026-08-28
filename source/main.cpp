#include "lexemes.hpp"
#include "parser.hpp"
#include <fstream>

int main(int argc, char *argv[]) {
   if (argc != 2) {
      printf("Expected single argument - input file.\n");
      exit(EXIT_FAILURE);
   }

   size_t fileLexeme = pushLexeme(argv[1]);
   std::ifstream file (getLexeme(fileLexeme));
   if (!file.is_open()) {
      printf("Could not read file '%s'.\n", argv[1]);
      exit(EXIT_FAILURE);
   }
   std::string code (std::istreambuf_iterator<char>(file), {});
   file.close();

   Parser parser;
   parser.lex(code, fileLexeme);
   code.clear(); // free up memory for the includes, which will read more files.
   code.shrink_to_fit();
   parser.handleIncludes();

   for (Token &token: parser.tokens) {
      printf("%s:%-5llu %s: '%s'.\n", getLexeme(token.fileLexeme).c_str(), token.line, getTokenName(token.type), getLexeme(token.lexeme).c_str());
   }

   parser.parse();
}
