#include "pil.hpp"
#include <chrono>

static inline std::chrono::steady_clock::time_point point;

void measure() {
   point = std::chrono::steady_clock::now();
}

float measureEnd() {
   return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - point).count() / 1000.0f;
}

void debugTokens(LexemeCache &cache, const std::vector<Token> &tokens) {
   printf("Tokens:\n");
   for (const Token &token: tokens) {
      printf("%s:%-5zu %s: '%s'.\n", getLexeme(cache, token.file).c_str(), token.line, getTokenName(token.type), getLexeme(cache, token.lexeme).c_str());
   }
}

void debugBytecode(Executor &executor) {
   printf("\nBytecode:\n");
   for (Command &command: executor.code) {
      printf("%s:%-5zu %s: ", getLexeme(executor.cache, command.file).c_str(), command.line, getLexeme(executor.cache, command.lexeme).c_str());
      for (size_t i = command.argStart; i < command.argStart + command.argCount; ++i) {
         printf("%s, ", getValueName(executor.arguments[i].type));
      }
      putchar('\n');
   }
}

void debugExecutionTime(float file, float lexer, float translator, float parser, float runtime) {
   printf("\nExecution time:\n");
   printf("File Read: %.3fms.\n", file + translator);
   printf("Lexer: %.3fms.\n", lexer);
   printf("Parser: %.3fms.\n", parser);
   printf("Runtime: %.3fms.\n", runtime);
}
