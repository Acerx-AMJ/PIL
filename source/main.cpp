#include "pil.hpp"

int main(int argc, char *argv[]) {
   if (argc != 2) {
      printf("PIL::main: Expected single argument - input file.\n");
      exit(EXIT_FAILURE);
   }

   LexemeCache cache;
   Diagnostics diagnostics;
   Executor executor (diagnostics, cache);

   measure();
   PILFile file = readPIL(diagnostics, cache, argv[1]);
   log(cache, diagnostics, SEVERITY_ERROR);
   float readTime = measureEnd();

   measure();
   std::vector<Token> tokens = lexPILFile(diagnostics, cache, file);
   log(cache, diagnostics, SEVERITY_ERROR);
   file.code.clear(); // free up memory for the includes, which will read more files
   file.code.shrink_to_fit();
   float lexTime = measureEnd();

   measure();
   translatePIL(executor, file, tokens);
   log(cache, diagnostics, SEVERITY_ERROR);
   float translatorTime = measureEnd();

   measure();
   parsePIL(executor, tokens);
   log(cache, diagnostics, SEVERITY_ERROR);
   tokens.clear(); // tokens are no longer in use
   tokens.shrink_to_fit();
   float parseTime = measureEnd();

   debugBytecode(executor);

   measure();
   callPILFunction(executor, "main", SEVERITY_ERROR);
   float runtime = measureEnd();

   logStackTrace(executor, SEVERITY_ERROR);
   logMemoryLeaks(executor);
   debugExecutionTime(readTime, lexTime, translatorTime, parseTime, runtime);
}
