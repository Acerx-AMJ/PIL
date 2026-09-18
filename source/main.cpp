#include "pil.hpp"
#include <cstring>
#include <filesystem>

void printHelp();

int main(int argc, char *argv[]) {
   bool debug = false;
   bool debugLexer = false;
   
   if (argc == 3 && strcmp(argv[1], "run") == 0) {
      std::filesystem::path path (argv[2]);
      if (path.has_extension() && path.extension() == ".pil") {
         Executor executor;
         std::vector<Token> tokens;
         PILFile file;

         measure();
         readPIL(executor.diagnostics, executor.cache, path.string(), file, 0, 0);
         log(executor, SEVERITY_ERROR);
         float readTime = measureEnd();

         measure();
         lexPILFile(executor.diagnostics, executor.cache, file, tokens);
         log(executor, SEVERITY_ERROR);
         file.code.clear(); // free up memory for the includes, which will read more files
         file.code.shrink_to_fit();
         float lexTime = measureEnd();

         measure();
         translatePIL(executor, file, tokens);
         log(executor, SEVERITY_ERROR);
         float translatorTime = measureEnd();

         debugTokens(debugLexer, executor.cache, tokens);

         measure();
         parsePIL(executor, tokens);
         log(executor, SEVERITY_ERROR);
         tokens.clear(); // tokens are no longer in use
         tokens.shrink_to_fit();
         float parseTime = measureEnd();

         debugBytecode(debug, executor);

         measure();
         callMain(executor, SEVERITY_ERROR);
         float runtime = measureEnd();

         logStackTrace(executor, SEVERITY_ERROR);
         logMemoryLeaks(executor);
         debugExecutionTime(debug, readTime, lexTime, translatorTime, parseTime, runtime);
      }
      else if (path.has_extension() && path.extension() == ".pilo") {
         Executor executor;
         measure();
         readCachedBytecode(executor, path.string());
         log(executor, SEVERITY_ERROR);
         float readTime = measureEnd();

         measure();
         callMain(executor, SEVERITY_ERROR);
         float runtime = measureEnd();

         logStackTrace(executor, SEVERITY_ERROR);
         logMemoryLeaks(executor);
         debugCacheExecutionTime(debug, readTime, runtime);
      }
      else {
         printf("PIL::run: Expected second argument to be either a '.pil' or .'pilo' file.\n");
         printHelp();
         exit(EXIT_FAILURE);
      }
   }
   else if (argc == 4 && strcmp(argv[1], "compile") == 0) {
      std::filesystem::path in (argv[2]);
      std::filesystem::path out (argv[3]);
      if (!in.has_extension() || in.extension() != ".pil" || !out.has_extension() || out.extension() != ".pilo") {
         printf("PIL::compile: Expected second argument to be a '.pil' file and the third to have the '.pilo' extension.\n");
         printHelp();
         exit(EXIT_FAILURE);
      }
      Executor executor;
      std::vector<Token> tokens;
      PILFile file;

      measure();
      readPIL(executor.diagnostics, executor.cache, in.string(), file, 0, 0);
      log(executor, SEVERITY_ERROR);
      float readTime = measureEnd();

      measure();
      lexPILFile(executor.diagnostics, executor.cache, file, tokens);
      log(executor, SEVERITY_ERROR);
      file.code.clear(); // free up memory for the includes, which will read more files
      file.code.shrink_to_fit();
      float lexTime = measureEnd();

      measure();
      translatePIL(executor, file, tokens);
      log(executor, SEVERITY_ERROR);
      float translatorTime = measureEnd();

      debugTokens(debugLexer, executor.cache, tokens);

      measure();
      parsePIL(executor, tokens);
      log(executor, SEVERITY_ERROR);
      tokens.clear(); // tokens are no longer in use
      tokens.shrink_to_fit();
      float parseTime = measureEnd();

      debugBytecode(debug, executor);

      printf("Writing to '%s'...\n", out.string().c_str());
      measure();
      writeToFile(executor, out.string());
      float writeTime = measureEnd();

      log(executor, SEVERITY_ERROR);
      printf("Wrote %zuB to '%s'.\n", std::filesystem::file_size(out), out.string().c_str());
      debugCompilationTime(debug, readTime, lexTime, translatorTime, parseTime, writeTime);
   }
   else {
      printHelp();
   }
}

void printHelp() {
   printf(
      "Usage:\n"
      "\t./pil [FLAGS...] [COMMAND] [ARGS...]\n"
      "Commands:\n"
      "\trun      [FILE/EXECUTABLE] run a file/executable\n"
      "\tcompile  [FILE] [OUTPUT]   compile a file into output\n"
      "Flags:\n"
      "\t--debug         output bytecode and compile/runtime time\n"
      "\t--debug-tokens  output tokens after translation\n"
   );
}
