#pragma once
#include "cache.hpp"

enum ErrorSeverity: char {
   SEVERITY_NONE, SEVERITY_WARNING, SEVERITY_ERROR, SEVERITY_IGNORE
};

struct Diagnostic {
   Diagnostic(ErrorSeverity severity, char *message, size_t file, size_t line)
      : severity(severity), message(message), file(file), line(line) {}

   ErrorSeverity severity;
   char *message;
   size_t file;
   size_t line;
};

struct Diagnostics {
   std::vector<Diagnostic> diagnostics;
   ErrorSeverity severity = SEVERITY_NONE;
};

struct LexemeCache;
struct Executor;

bool shouldError(Diagnostics &diagnostics, ErrorSeverity errorSeverity);

void warn(Diagnostics &diagnostics, size_t file, size_t line, const char *msg, ...);
void error(Diagnostics &diagnostics, size_t file, size_t line, const char *msg, ...);
void clear(Diagnostics &diagnostics);

void log(LexemeCache &cache, Diagnostics &diagnostics, ErrorSeverity quitSeverity);
void logStackTrace(Executor &executor, ErrorSeverity quitSeverity);
void logDiagnostic(LexemeCache &cache, Diagnostic &diagnostic, ErrorSeverity quitSeverity);
