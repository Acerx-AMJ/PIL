#include "pil.hpp"
#include <cstdarg>

constexpr const char *PROGRAM_EXITED = "\e[0;31mProgram exited with error code %d.\e[0m\n";
constexpr const char *STACKTRACE_INFO = "\e[0;34mStack Trace\e[0m";
constexpr const char *ERROR_TEXT = "\e[0;31mError\e[0m";
constexpr const char *WARNING_TEXT = "\e[0;33mWarning\e[0m";

constexpr const char *LEAKED_STRINGS = "\e[0;31mProgram leaked %zu strings.\e[0m\n";
constexpr const char *LEAKED_ARRAYS = "\e[0;31mProgram leaked %zu arrays.\e[0m\n";
constexpr const char *AND_N_OTHERS = "\e[0;34mAnd %zu others...\e[0m\n";
constexpr size_t MAX_LEAK_TRACE = 5;

bool shouldError(Diagnostics &diagnostics, ErrorSeverity errorSeverity) {
   return diagnostics.severity >= errorSeverity;
}

void warn(Diagnostics &diagnostics, size_t file, size_t line, const char *msg, ...) {
   va_list args;
   va_start(args, msg);
   int len = vsnprintf(nullptr, 0, msg, args) + 1; // null-terminator
   char *message = (char*)malloc(len);
   vsnprintf(message, len, msg, args);
   va_end(args);

   diagnostics.diagnostics.emplace_back(SEVERITY_WARNING, message, file, line);
   diagnostics.severity = std::max(diagnostics.severity, SEVERITY_WARNING);
}

void error(Diagnostics &diagnostics, size_t file, size_t line, const char *msg, ...) {
   va_list args;
   va_start(args, msg);
   int len = vsnprintf(nullptr, 0, msg, args) + 1; // null-terminator
   char *message = (char*)malloc(len);
   vsnprintf(message, len, msg, args);
   va_end(args);

   diagnostics.diagnostics.emplace_back(SEVERITY_ERROR, message, file, line);
   diagnostics.severity = std::max(diagnostics.severity, SEVERITY_ERROR);
}

void clear(Diagnostics &diagnostics) {
   for (Diagnostic &diagnostic: diagnostics.diagnostics) {
      free(diagnostic.message);
   }

   diagnostics.diagnostics.clear();
   diagnostics.severity = SEVERITY_NONE;
}

void printDiagnostic(LexemeCache &cache, Diagnostic &diagnostic) {
   const char *type = (diagnostic.severity == SEVERITY_ERROR ? ERROR_TEXT : WARNING_TEXT);
   const char *message = diagnostic.message;

   if (diagnostic.file == 0 && diagnostic.line == 0) {
      printf("%s: %s.\n", type, message);
   }
   else {
      const char *lexeme = getLexeme(cache, diagnostic.file).c_str();
      printf("%s: %s at %s:%zu.\n", type, message, lexeme, diagnostic.line);
   }
}

void errorIfSevereEnough(ErrorSeverity severity, ErrorSeverity quitSeverity) {
   if (severity >= quitSeverity) {
      int exitCode = EXIT_FAILURE;
      printf(PROGRAM_EXITED, exitCode);
      exit(exitCode);
   }
}

void log(LexemeCache &cache, Diagnostics &diagnostics, ErrorSeverity quitSeverity) {
   for (Diagnostic &diagnostic: diagnostics.diagnostics) {
      printDiagnostic(cache, diagnostic);
   }
   errorIfSevereEnough(diagnostics.severity, quitSeverity);
   clear(diagnostics);
}

void logStackTrace(Executor &executor, ErrorSeverity quitSeverity) {
   ErrorSeverity severity = executor.diagnostics.severity;
   log(executor.cache, executor.diagnostics, SEVERITY_IGNORE);

   if (!executor.stackTrace.empty()) {
      printf("%s (newest first):\n", STACKTRACE_INFO);
   }

   while (!executor.stackTrace.empty()) {
      Trace trace = executor.stackTrace.top();
      executor.stackTrace.pop();
   
      Command &command = executor.code[trace.position];
      const char *functionLexeme = getLexeme(executor.cache, executor.code[trace.position].lexeme).c_str();
      const char *fileLexeme = getLexeme(executor.cache, command.file).c_str();
      printf("%s:%zu @ %s:%zu.\n", functionLexeme, trace.position, fileLexeme, command.line);
   }
   errorIfSevereEnough(severity, quitSeverity);
   clear(executor.diagnostics);
}

void logMemoryLeaks(Executor &executor) {
   size_t strings = executor.strings.size();
   size_t arrays = executor.arrays.size();
   if (strings == 0 && arrays == 0) {
      return;
   }

   if (strings != 0) {
      size_t size = std::min(strings, MAX_LEAK_TRACE);
      size_t i = 0;

      printf(LEAKED_STRINGS, strings);
      for (const auto &[id, string]: executor.strings) {
         printf("%zu: '%s', mark %d.\n", id, string.string.c_str(), string.mark);
         i += 1;
         if (i >= size) {
            break;
         }
      }

      if (size < strings) {
         printf(AND_N_OTHERS, strings - size);
      }
   }
   if (arrays != 0) {
      size_t size = std::min(arrays, MAX_LEAK_TRACE);
      size_t i = 0;

      printf(LEAKED_ARRAYS, arrays);
      for (const auto &[id, array]: executor.arrays) {
         printf("%zu: Array with size %zu, mark %d.\n", id, array.array.size(), array.mark);
         i += 1;
         if (i >= size) {
            break;
         }
      }

      if (size < arrays) {
         printf(AND_N_OTHERS, arrays - size);
      }
   }
}

void logDiagnostic(LexemeCache &cache, Diagnostic &diagnostic, ErrorSeverity quitSeverity) {
   printDiagnostic(cache, diagnostic);
   errorIfSevereEnough(diagnostic.severity, quitSeverity);
}
