#include "error.hpp"
#include <format>
#include <iostream>

constexpr const char *ERROR_TEXT = "\e[0;31mError\e[0m";
constexpr const char *WARNING_TEXT = "\e[0;33mWarning\e[0m";

bool shouldError(Diagnostics &diagnostics, ErrorSeverity errorSeverity) {
   return diagnostics.severity >= errorSeverity;
}

void warn(Diagnostics &diagnostics, const std::string &message, size_t file, size_t line) {
   diagnostics.diagnostics.emplace_back(SEVERITY_WARNING, message, file, line);
   diagnostics.severity = std::max(diagnostics.severity, SEVERITY_WARNING);
}

void error(Diagnostics &diagnostics, const std::string &message, size_t file, size_t line) {
   diagnostics.diagnostics.emplace_back(SEVERITY_ERROR, message, file, line);
   diagnostics.severity = std::max(diagnostics.severity, SEVERITY_ERROR);
}

void clear(Diagnostics &diagnostics) {
   diagnostics.diagnostics.clear();
   diagnostics.severity = SEVERITY_NONE;
}

void printDiagnostic(LexemeCache &cache, Diagnostic &diagnostic) {
   const char *type = (diagnostic.severity == SEVERITY_ERROR ? ERROR_TEXT : WARNING_TEXT);
   std::cout << std::format("{}: {}", type, diagnostic.message);

   if (diagnostic.file == 0 && diagnostic.line == 0) {
      std::cout << ".\n";
   }
   else {
      std::cout << std::format(" at {}:{}.\n", getLexeme(cache, diagnostic.file), diagnostic.line);
   }
}

void log(LexemeCache &cache, Diagnostics &diagnostics, ErrorSeverity quitSeverity) {
   for (Diagnostic &diagnostic: diagnostics.diagnostics) {
      printDiagnostic(cache, diagnostic);
   }

   if (diagnostics.severity >= quitSeverity) {
      int exitCode = EXIT_FAILURE;
      std::cout << std::format("Program exited with error code {}.\n", exitCode);
      std::exit(exitCode);
   }
   clear(diagnostics);
}

void logDiagnostic(LexemeCache &cache, Diagnostic &diagnostic, ErrorSeverity quitSeverity) {
   printDiagnostic(cache, diagnostic);
   if (diagnostic.severity >= quitSeverity) {
      int exitCode = EXIT_FAILURE;
      std::cout << std::format("Program exited with error code {}.\n", exitCode);
      std::exit(exitCode);
   }
}
