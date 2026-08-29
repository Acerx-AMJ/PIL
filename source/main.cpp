#include "pil.hpp"

int main(int argc, char *argv[]) {
   if (argc != 2) {
      printf("PIL::main: Expected single argument - input file.\n");
      exit(EXIT_FAILURE);
   }

   LexemeCache cache;
   Diagnostics diagnostics;

   PILFile file = readPIL(diagnostics, cache, argv[1]);
   log(cache, diagnostics, SEVERITY_ERROR);

   std::vector<Token> tokens = lexPILFile(diagnostics, cache, file);
   log(cache, diagnostics, SEVERITY_ERROR);
   file.code.clear(); // free up memory for the includes, which will read more files
   file.code.shrink_to_fit();

   handlePILFileIncludes(diagnostics, cache, file, tokens);
   log(cache, diagnostics, SEVERITY_ERROR);

   printf("Tokens:\n");
   for (Token &token: tokens) {
      printf("%s:%-5zu %s: '%s'.\n", getLexeme(cache, token.fileLexeme).c_str(), token.line, getTokenName(token.type), getLexeme(cache, token.lexeme).c_str());
   }

   ByteCode code;
   defineStandardBuiltins(cache, code);
   parsePIL(diagnostics, cache, code, tokens);
   log(cache, diagnostics, SEVERITY_ERROR);
   tokens.clear(); // tokens are no longer in use
   tokens.shrink_to_fit();

   printf("\nBytecode:\n");
   for (Command &command: code.code) {
      printf("%s:%-5zu %s: ", getLexeme(cache, command.file).c_str(), command.line, getLexeme(cache, command.lexeme).c_str());
      for (Value param: command.args) {
         printf("%s ", getValueName(param.type));
      }
      putchar('\n');
   }

   callPILFunction(diagnostics, cache, code, "main");
   log(cache, diagnostics, SEVERITY_ERROR);
}
