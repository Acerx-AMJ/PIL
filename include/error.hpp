#pragma once
#include "cache.hpp"

enum ErrorSeverity: char {
   SEVERITY_NONE, SEVERITY_WARNING, SEVERITY_ERROR, SEVERITY_IGNORE
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
   ErrorSeverity severity = SEVERITY_NONE;
};

struct LexemeCache;
struct Executor;

bool shouldError(Diagnostics &diagnostics, ErrorSeverity errorSeverity);

void warn(Diagnostics &diagnostics, const std::string &message, size_t file, size_t line);
void error(Diagnostics &diagnostics, const std::string &message, size_t file, size_t line);
void clear(Diagnostics &diagnostics);

void log(LexemeCache &cache, Diagnostics &diagnostics, ErrorSeverity quitSeverity);
void logStackTrace(Executor &executor, ErrorSeverity quitSeverity);
void logDiagnostic(LexemeCache &cache, Diagnostic &diagnostic, ErrorSeverity quitSeverity);
