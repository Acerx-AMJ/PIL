#include "pil.hpp"

int main(int argc, char *argv[]) {
   if (argc != 2) {
      printf("Expected single argument - input file.\n");
      exit(EXIT_FAILURE);
   }

   LexemeCache cache;
   PILFile file = readPIL(cache, argv[1]);
   std::vector<Token> tokens = lexPILFile(cache, file);

   // free up memory for the includes, which will read more files
   file.code.clear();
   file.code.shrink_to_fit();
   handlePILFileIncludes(cache, file, tokens);

   printf("Tokens:\n");
   for (Token &token: tokens) {
      printf("%s:%-5llu %s: '%s'.\n", getLexeme(cache, token.fileLexeme).c_str(), token.line, getTokenName(token.type), getLexeme(cache, token.lexeme).c_str());
   }

   FunctionData functions;
   defineStandardBuiltins(cache, functions);
   parsePIL(cache, functions, tokens);

   printf("\nBlocks:\n");
   for (Block &block: functions.blocks) {
      printf("%s:%zu.\n", getLexeme(cache, block.lexeme).c_str(), tokens[block.tokenPosition].line);
      printf("Params: ");
      for (size_t param: block.params) {
         printf("%s, ", getLexeme(cache, param).c_str());
      }
      putchar('\n');
      for (auto &[lexeme, values]: block.commands) {
         printf("\t%s: ", getLexeme(cache, lexeme).c_str());
         for (Value &value: values) {
            printf("%s, ", getValueName(value.type));
         }
         putchar('\n');
      }
   }
}
