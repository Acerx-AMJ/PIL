#include "pil.hpp"
#include <chrono>

int main(int argc, char *argv[]) {
   if (argc != 2) {
      printf("PIL::main: Expected single argument - input file.\n");
      exit(EXIT_FAILURE);
   }

   LexemeCache cache;
   Diagnostics diagnostics;
   Executor executor (diagnostics, cache);

   auto fbegin = std::chrono::steady_clock::now();
   PILFile file = readPIL(diagnostics, cache, argv[1]);
   log(cache, diagnostics, SEVERITY_ERROR);
   auto fend = std::chrono::steady_clock::now();

   auto lbegin = std::chrono::steady_clock::now();
   std::vector<Token> tokens = lexPILFile(diagnostics, cache, file);
   log(cache, diagnostics, SEVERITY_ERROR);
   file.code.clear(); // free up memory for the includes, which will read more files
   file.code.shrink_to_fit();
   auto lend = std::chrono::steady_clock::now();

   auto tbegin = std::chrono::steady_clock::now();
   translatePIL(executor, file, tokens);
   log(cache, diagnostics, SEVERITY_ERROR);
   auto tend = std::chrono::steady_clock::now();

   // printf("Tokens:\n");
   // for (Token &token: tokens) {
   //    printf("%s:%-5zu %s: '%s'.\n", getLexeme(cache, token.fileLexeme).c_str(), token.line, getTokenName(token.type), getLexeme(cache, token.lexeme).c_str());
   // }

   auto dbegin = std::chrono::steady_clock::now();
   defineStandardBuiltins(executor);
   auto dend = std::chrono::steady_clock::now();

   auto pbegin = std::chrono::steady_clock::now();
   parsePIL(executor, tokens);
   log(cache, diagnostics, SEVERITY_ERROR);
   tokens.clear(); // tokens are no longer in use
   tokens.shrink_to_fit();
   auto pend = std::chrono::steady_clock::now();

   printf("\nBytecode:\n");
   for (Command &command: executor.code) {
      printf("%s:%-5zu %s: ", getLexeme(cache, command.file).c_str(), command.line, getLexeme(cache, command.lexeme).c_str());
      for (Value param: command.args) {
         printf("%s ", getValueName(param.type));
      }
      putchar('\n');
   }

   auto rbegin = std::chrono::steady_clock::now();
   callPILFunction(executor, "main", SEVERITY_ERROR);
   logStackTrace(executor, SEVERITY_ERROR);
   auto rend = std::chrono::steady_clock::now();

   printf("\nExecution time:\n");
   printf("File read: %f.\n", std::chrono::duration_cast<std::chrono::microseconds>(fend - fbegin).count() / 1000.0f);
   printf("Lexer: %f.\n", std::chrono::duration_cast<std::chrono::microseconds>(lend - lbegin).count() / 1000.0f);
   printf("translator: %f.\n", std::chrono::duration_cast<std::chrono::microseconds>(tend - tbegin).count() / 1000.0f);
   printf("definitions: %f.\n", std::chrono::duration_cast<std::chrono::microseconds>(dend - dbegin).count() / 1000.0f);
   printf("parser: %f.\n", std::chrono::duration_cast<std::chrono::microseconds>(pend - pbegin).count() / 1000.0f);
   printf("runtime: %f.\n", std::chrono::duration_cast<std::chrono::microseconds>(rend - rbegin).count() / 1000.0f);
}
