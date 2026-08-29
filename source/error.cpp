#include "error.hpp"
#include <format>
#include <iostream>

constexpr const char *ERROR_TEXT = "\e[0;31mError\e[0m";
constexpr const char *WARNING_TEXT = "\e[0;33mWarning\e[0m";

bool hasErrors(Diagnostics &diagnostics) {
   return diagnostics.errors;
}

bool hasWarnings(Diagnostics &diagnostics) {
   return diagnostics.warnings;
}

void warn(Diagnostics &diagnostics, const std::string &message, size_t file, size_t line) {
   diagnostics.diagnostics.emplace_back(SEVERITY_WARNING, message, file, line);
   diagnostics.warnings = true;
}

void error(Diagnostics &diagnostics, const std::string &message, size_t file, size_t line) {
   diagnostics.diagnostics.emplace_back(SEVERITY_ERROR, message, file, line);
   diagnostics.errors = true;
}

void clear(Diagnostics &diagnostics) {
   diagnostics.diagnostics.clear();
   diagnostics.errors = false;
   diagnostics.warnings = false;
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
   ErrorSeverity severity = ErrorSeverity(SEVERITY_WARNING - 1);
   for (Diagnostic &diagnostic: diagnostics.diagnostics) {
      printDiagnostic(cache, diagnostic);
      severity = std::max(severity, diagnostic.severity);
   }

   clear(diagnostics);
   if (severity >= quitSeverity) {
      int exitCode = EXIT_FAILURE;
      std::cout << std::format("Program exited with error code {}.\n", exitCode);
      std::exit(exitCode);
   }
}

void logDiagnostic(LexemeCache &cache, Diagnostic &diagnostic, ErrorSeverity quitSeverity) {
   printDiagnostic(cache, diagnostic);
   if (diagnostic.severity >= quitSeverity) {
      int exitCode = EXIT_FAILURE;
      std::cout << std::format("Program exited with error code {}.\n", exitCode);
      std::exit(exitCode);
   }
}
