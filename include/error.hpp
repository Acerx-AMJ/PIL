#pragma once
#include "cache.hpp"

// SEVERITY_NONE for use in log and logDiagnostic to not call exit
enum ErrorSeverity: char {
   SEVERITY_WARNING, SEVERITY_ERROR, SEVERITY_NONE
};

struct Diagnostic {
   Diagnostic(ErrorSeverity severity, const std::string &message, size_t file, size_t line)
      : severity(severity), message(message), file(file), line(line) {}

   ErrorSeverity severity;
   std::string message;
   size_t file;
   size_t line;
};

struct Diagnostics {
   std::vector<Diagnostic> diagnostics;
   bool errors = false;
   bool warnings = false;
};

bool hasErrors(Diagnostics &diagnostics);
bool hasWarnings(Diagnostics &diagnostics);

void warn(Diagnostics &diagnostics, const std::string &message, size_t file, size_t line);
void error(Diagnostics &diagnostics, const std::string &message, size_t file, size_t line);
void clear(Diagnostics &diagnostics);

struct LexemeCache;
void log(LexemeCache &cache, Diagnostics &diagnostics, ErrorSeverity quitSeverity);
void logDiagnostic(LexemeCache &cache, Diagnostic &diagnostic, ErrorSeverity quitSeverity);
