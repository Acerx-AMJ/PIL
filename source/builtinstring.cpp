#include "builtin.hpp"
#include "builtinhelpers.hpp"
#include <sstream>

// helpers
constexpr char toLower(char c) { return (c >= 'A' && c <= 'Z' ? c + ('a' - 'A') : c); }
constexpr char toUpper(char c) { return (c >= 'a' && c <= 'z' ? c - ('a' - 'A') : c); }

template<size_t(std::string::*Find)(const std::string&, size_t) const, size_t Default>
inline void stringFind(const Command &command, Executor &executor, const char *function) {
   const std::string *string, *find = nullptr;
   if (!constStringOrError(command, executor, function, string) || !constStringOrError(command, executor, function, find, 1)) return;
   size_t pos = (string->*Find)(*find, Default);
   if (pos == std::string::npos) storeInRegister(executor, command, NULL_VALUE, function);
   else storeNumber(executor, command, pos, false, function);
}

template<size_t(std::string::*SFind)(const std::string&, size_t) const, size_t(std::string::*CFind)(char, size_t) const>
inline void stringCharFind(const Command &command, Executor &executor, const char *function) {
   const std::string *string, *sfind = nullptr;
   const char *cfind = nullptr;
   if (!constStringOrError(command, executor, function, string) || !constStringOrCharOrError(command, executor, function, sfind, cfind, 1)) return;
   size_t start = getNum(executor, command, 2, function);
   if (start > string->size()) {
      error(executor.diagnostics, command.file, command.line, "%s: Start position %zu is out of bounds", function, start);
      return;
   }
   size_t find = (sfind ? (string->*SFind)(*sfind, start) : (string->*CFind)(*cfind, start));
   if (find == std::string::npos) storeInRegister(executor, command, NULL_VALUE, function);
   else storeNumber(executor, command, find, false, function);
}

// string ops
void builtinStringNew(const Command &command, Executor &executor) {
   std::string result;
   for (size_t i = 1; i < command.argCount; ++i) {
      result += toString(executor, arg(executor, command, i), "string-new", command.file, command.line);
   }
   storeString(executor, command, result, arg(executor, command, 0), "string-new");
}

void builtinStringFmt(const Command &command, Executor &executor) {
   storeString(executor, command, format(command, executor, "string-fmt", 1), arg(executor, command, 0), "string-fmt");
}

void builtinStringRepeat(const Command &command, Executor &executor) {
   std::string result;
   size_t n = getNum(executor, command, 1, "string-repeat");
   std::string fill = toString(executor, arg(executor, command, 2), "string-repeat", command.file, command.line);
   result.reserve(n * fill.size());
   for (size_t i = 0; i < n; ++i) {
      result += fill;
   }
   storeString(executor, command, result, arg(executor, command, 0), "string-repeat");
}

void builtinStringClear(const Command &command, Executor &executor) {
   std::string *string;
   if (!stringOrError(command, executor, "string-clear", string)) return;
   string->clear();
}

void builtinStringMemFree(const Command &command, Executor &executor) {
   std::string *string;
   if (!stringOrError(command, executor, "string-memfree", string)) return;
   string->clear();
   string->shrink_to_fit();
}

void builtinStringEmpty(const Command &command, Executor &executor) {
   std::string *string;
   if (!stringOrError(command, executor, "string-empty", string)) return;
   storeBoolean(executor, command, string->empty(), "string-empty");
}

void builtinStringSize(const Command &command, Executor &executor) {
   std::string *string;
   if (!stringOrError(command, executor, "string-size", string)) return;
   storeNumber(executor, command, string->size(), false, "string-size");
}

void builtinStringCapacity(const Command &command, Executor &executor) {
   std::string *string;
   if (!stringOrError(command, executor, "string-capacity", string)) return;
   storeNumber(executor, command, string->capacity(), false, "string-capacity");
}

void builtinStringReserve(const Command &command, Executor &executor) {
   std::string *string;
   if (!stringOrError(command, executor, "string-reserve", string)) return;
   string->reserve(getNum(executor, command, 1, "string-reserve"));
}

void builtinStringResize(const Command &command, Executor &executor) {
   std::string *string;
   if (!stringOrError(command, executor, "string-resize", string)) return;
   string->resize(getNum(executor, command, 1, "string-resize"), getChar(command, executor, "string-resize", 2));
}

void builtinStringSet(const Command &command, Executor &executor) {
   std::string *string;
   if (!stringOrError(command, executor, "string-set", string)) return;
   size_t id = getNum(executor, command, 1, "string-set");
   if (id < 0 || id >= string->size()) {
      error(executor.diagnostics, command.file, command.line, "string-set: Index %zu is out of bounds", id);
      return;
   }
   (*string)[id] = getChar(command, executor, "string-set", 2);
}

void builtinStringIdx(const Command &command, Executor &executor) {
   std::string *string;
   if (!stringOrError(command, executor, "string-idx", string)) return;
   size_t id = getNum(executor, command, 1, "string-idx");
   if (id < 0 || id >= string->size()) {
      error(executor.diagnostics, command.file, command.line, "string-idx: Index %zu is out of bounds", id);
      return;
   }
   Value value {VALUE_CHARACTER};
   value.character = (*string)[id];
   storeInRegister(executor, command, value, "string-idx");
}

void builtinStringBack(const Command &command, Executor &executor) {
   std::string *string;
   if (!stringOrError(command, executor, "string-back", string)) return;
   if (string->empty()) {
      error(executor.diagnostics, command.file, command.line, "string-back: Cannot get the back character of string since the string is empty");
      return;
   }
   Value value {VALUE_CHARACTER};
   value.character = string->back();
   storeInRegister(executor, command, value, "string-back");
}

void builtinStringFront(const Command &command, Executor &executor) {
   std::string *string;
   if (!stringOrError(command, executor, "string-front", string)) return;
   if (string->empty()) {
      error(executor.diagnostics, command.file, command.line, "string-front: Cannot get the front character of string since the string is empty");
      return;
   }
   Value value {VALUE_CHARACTER};
   value.character = string->front();
   storeInRegister(executor, command, value, "string-front");
}

void builtinStringPush(const Command &command, Executor &executor) {
   std::string *string;
   if (!stringOrError(command, executor, "string-push", string)) return;
   string->push_back(getChar(command, executor, "string-push", 1));
}

void builtinStringInsert(const Command &command, Executor &executor) {
   std::string *string;
   if (!stringOrError(command, executor, "string-insert", string)) return;
   size_t id = getNum(executor, command, 1, "string-insert");
   if (id < 0 || id > string->size()) {
      error(executor.diagnostics, command.file, command.line, "string-insert: Index %zu is out of bounds", id);
      return;
   }
   string->insert(string->begin() + id, getChar(command, executor, "string-insert", 2));
}

void builtinStringPop(const Command &command, Executor &executor) {
   std::string *string;
   if (!stringOrError(command, executor, "string-pop", string)) return;
   if (string->empty()) {
      error(executor.diagnostics, command.file, command.line, "string-pop: Cannot pop from an empty string");
      return;
   }
   string->pop_back();
}

void builtinStringErase(const Command &command, Executor &executor) {
   std::string *string;
   if (!stringOrError(command, executor, "string-erase", string)) return;
   size_t id = getNum(executor, command, 1, "string-erase");
   if (id < 0 || id >= string->size()) {
      error(executor.diagnostics, command.file, command.line, "string-erase: Index %zu is out of bounds", id);
      return;
   }
   string->erase(string->begin() + id);
}

void builtinStringFree(const Command &command, Executor &executor) {
   for (size_t i = 0; i < command.argCount; ++i) {
      Value a = arg(executor, command, i);
      Value &string = resolveVariableByRef(executor, a);
      if (string.type != VALUE_STRING) {
         error(executor.diagnostics, command.file, command.line, "string-free: Expected string, got %s instead", getValueName(string.type));
         return;
      }
      executor.strings.erase(string.string);
      string = NULL_VALUE;
   }
}

void builtinStringMark(const Command &command, Executor &executor) {
   Value string = resolveVariable(executor, arg(executor, command, 0));
   if (string.type != VALUE_STRING) {
      error(executor.diagnostics, command.file, command.line, "string-mark: Expected string, got %s instead", getValueName(string.type));
      return;
   }
   auto it = executor.strings.find(string.string);
   if (it == executor.strings.end()) {
      error(executor.diagnostics, command.file, command.line, "Invalid string ID %zu. Use after free", string.string);
      return;
   }
   it->second.mark = getNum(executor, command, 1, "string-mark");
}

void builtinStringFreeMarked(const Command &command, Executor &executor) {
   int mark = getNum(executor, command, 0, "string-free-marked");
   for (auto &[id, string]: executor.strings) {
      if (string.mark == mark) {
         executor.strings.erase(id);
      }
   }
}

void builtinStringSplit(const Command &command, Executor &executor) {
   const std::string *string;
   const std::string *sdelim = nullptr;
   const char *cdelim = nullptr;
   if (!constStringOrError(command, executor, "string-split", string) || !constStringOrCharOrError(command, executor, "string-split", sdelim, cdelim, 1)) return;
   std::vector<Value> output;

   if (sdelim) {
      if (sdelim->empty()) {
         error(executor.diagnostics, command.file, command.line, "string-split: Delimiter cannot be empty");
         return;
      }

      size_t delimCount = 0;
      size_t sdelimSize = sdelim->size();
      for (size_t offset = string->find(*sdelim); offset != std::string::npos; offset = string->find(*sdelim, offset + sdelimSize)) {
         delimCount += 1;
      }
      output.reserve(delimCount + 1);

      size_t last = 0;
      for (size_t pos = string->find(*sdelim); pos != std::string::npos; pos = string->find(*sdelim, last)) {
         Value value {VALUE_STRING};
         value.string = allocateString(executor, std::string(string->begin() + last, string->begin() + pos));
         output.push_back(value);
         last = pos + sdelimSize;
      }

      Value lastValue {VALUE_STRING};
      if (last != string->size()) {
         lastValue.string = allocateString(executor, std::string(string->begin() + last, string->end()));
      }
      else {
         lastValue.string = allocateString(executor, "");
      }
      output.push_back(lastValue);
   }
   else if (cdelim) {
      size_t delimCount = std::count(string->begin(), string->end(), *cdelim);
      output.reserve(delimCount + 1);

      std::stringstream stream (*string);
      std::string piece;

      while (std::getline(stream, piece, *cdelim)) {
         Value value {VALUE_STRING};
         value.string = allocateString(executor, piece);
         output.push_back(value);
      }

      if (!string->empty() && string->back() == *cdelim) {
         Value value {VALUE_STRING};
         value.string = allocateString(executor, "");
         output.push_back(value);
      }
   }
   storeArray(executor, command, output, back(executor, command), "string-split");
}

void builtinStringConcat(const Command &command, Executor &executor) {
   std::string *string1;
   if (!stringOrError(command, executor, "string-concat", string1)) return;
   std::string result = *string1;
   for (size_t i = 1; i < command.argCount; ++i) {
      result += toString(executor, arg(executor, command, i), "string-concat", command.file, command.line);
   }
   *string1 = std::move(result);
}

void builtinStringSubstr(const Command &command, Executor &executor) {
   const std::string *string;
   if (!constStringOrError(command, executor, "string-substr", string)) return;
   size_t start = getNum(executor, command, 1, "string-substr");
   size_t end = getNum(executor, command, 2, "string-substr");
   if (start < 0 || start >= string->size() || end < 0 || end > string->size() || start >= end) {
      error(executor.diagnostics, command.file, command.line, "string-substr: Invalid substring range %zu-%zu", start, end);
      return;
   }
   std::string substring = string->substr(start, end - start);
   storeString(executor, command, substring, back(executor, command), "string-substr");
}

void builtinStringCount(const Command &command, Executor &executor) {
   const std::string *string, *scount = nullptr;
   const char *ccount = nullptr;
   if (!constStringOrError(command, executor, "string-count", string) || !constStringOrCharOrError(command, executor, "string-count", scount, ccount, 1)) return;

   if (scount) {
      // KMP algorithm
      // https://www.geeksforgeeks.org/dsa/frequency-substring-string/#expected-approach-using-kmp-algorithm-os1-s2-time-and-os2-space
      size_t m = scount->size();
      size_t n = string->size();
      if (n < m) {
         storeNumber(executor, command, 0, false, "string-count");
         return;
      }

      std::vector<int> lps (m, 0);
      size_t length = 0;
      size_t count = 0;

      for (size_t i = 1; i < m; ++i) {
         while (length > 0 && (*scount)[i] != (*scount)[length]) {
            length = lps[length - 1];
         }
         if ((*scount)[i] == (*scount)[length]) length += 1;
         lps[i] = length;
      }

      for (size_t i = 0, j = 0; i < n; ++i) {
         while (j > 0 && (*string)[i] != (*scount)[j]) {
            j = lps[j - 1];
         }
         if ((*string)[i] == (*scount)[j]) j += 1;
         if (j == m) {
            count += 1;
            j = lps[j - 1];
         }
      }
      storeNumber(executor, command, count, false, "string-count");
   }
   else if (ccount) {
      storeNumber(executor, command, std::count(string->begin(), string->end(), *ccount), false, "string-count");
   }
}

void builtinStringReverse(const Command &command, Executor &executor) {
   std::string *string;
   if (!stringOrError(command, executor, "string-reverse", string)) return;
   std::reverse(string->begin(), string->end());
}

void builtinStringFind(const Command &command, Executor &executor) {
   stringCharFind<&std::string::find, &std::string::find>(command, executor, "string-find");
}

void builtinStringRfind(const Command &command, Executor &executor) {
   stringCharFind<&std::string::rfind, &std::string::rfind>(command, executor, "string-rfind");
}

void builtinStringFindFirstOf(const Command &command, Executor &executor) {
   stringFind<&std::string::find_first_of, 0>(command, executor, "string-find-first-of");
}

void builtinStringFindFirstNotOf(const Command &command, Executor &executor) {
   stringFind<&std::string::find_first_not_of, 0>(command, executor, "string-find-first-not-of");
}

void builtinStringFindLastOf(const Command &command, Executor &executor) {
   stringFind<&std::string::find_last_of, std::string::npos>(command, executor, "string-find-last-of");
}

void builtinStringFindLastNotOf(const Command &command, Executor &executor) {
   stringFind<&std::string::find_last_not_of, std::string::npos>(command, executor, "string-find-last-not-of");
}

void builtinStringReplace(const Command &command, Executor &executor) {
   std::string *string;
   const std::string *sfind = nullptr, *sreplace = nullptr;
   const char *cfind = nullptr, *creplace = nullptr;
   if (!stringOrError(command, executor, "string-replace", string) || !constStringOrCharOrError(command, executor, "string-replace", sfind, cfind, 1) || !constStringOrCharOrError(command, executor, "string-replace", sreplace, creplace, 2)) return;
   size_t start = getNum(executor, command, 3, "string-replace");
   if (start > string->size()) {
      error(executor.diagnostics, command.file, command.line, "string-replace: Start position %zu is out of bounds", start);
      return;
   }

   size_t findSize = sfind ? sfind->size() : 1;
   size_t find = sfind ? string->find(*sfind, start) : string->find(*cfind, start);
   if (find != std::string::npos) {
      string->replace(find, findSize, sreplace ? *sreplace : std::string(1, *creplace));
   }
}

void builtinStringReplaceAll(const Command &command, Executor &executor) {
   std::string *string;
   const std::string *sfind = nullptr, *sreplace = nullptr;
   const char *cfind = nullptr, *creplace = nullptr;
   if (!stringOrError(command, executor, "string-replace-all", string) || !constStringOrCharOrError(command, executor, "string-replace-all", sfind, cfind, 1) || !constStringOrCharOrError(command, executor, "string-replace-all", sreplace, creplace, 2)) return;

   size_t findSize = sfind ? sfind->size() : 1;
   if (findSize == 0) {
      error(executor.diagnostics, command.file, command.line, "string-replace-all: Find pattern cannot be empty");
      return;
   }
   std::string find = sfind ? *sfind : std::string(1, *cfind);
   std::string replace = sreplace ? *sreplace : std::string(1, *creplace);
   size_t pos = 0;
   while ((pos = string->find(find, pos)) != std::string::npos) {
      string->replace(pos, find.size(), replace);
      pos += replace.size();
   }
}

void builtinStringContains(const Command &command, Executor &executor) {
   const std::string *string, *scontains = nullptr;
   const char *ccontains = nullptr;
   if (!constStringOrError(command, executor, "string-contains", string) || !constStringOrCharOrError(command, executor, "string-contains", scontains, ccontains, 1)) return;
   size_t find = (scontains ? string->find(*scontains) : string->find(*ccontains));
   storeBoolean(executor, command, find != std::string::npos, "string-contains");
}

void builtinStringEraseAll(const Command &command, Executor &executor) {
   std::string *string;
   const std::string *serase = nullptr;
   const char *cerase = nullptr;
   if (!stringOrError(command, executor, "string-erase-all", string) || !constStringOrCharOrError(command, executor, "string-erase-all", serase, cerase, 1)) return;

   if (serase) {
      size_t erasedCount = 0;
      for (char ch: *serase) {
         auto end = std::remove(string->begin(), string->end() - erasedCount, ch);
         erasedCount = std::distance(end, string->end());
      }
      string->erase(string->begin() + (string->size() - erasedCount), string->end());
   }
   else if (cerase) {
      string->erase(std::remove_if(string->begin(), string->end(), [cerase](char ch){ return ch == *cerase; }), string->end());
   }
}

void builtinStringStartsWith(const Command &command, Executor &executor) {
   const std::string *string, *ssubstr = nullptr;
   const char *csubstr = nullptr;
   if (!constStringOrError(command, executor, "string-starts-with", string) || !constStringOrCharOrError(command, executor, "string-starts-with", ssubstr, csubstr, 1)) return;
   if (ssubstr) {
      storeBoolean(executor, command, string->find(*ssubstr) == 0, "string-starts-with");
   }
   else if (csubstr) {
      storeBoolean(executor, command, !string->empty() && string->front() == *csubstr, "string-starts-with");
   }
}

void builtinStringEndsWith(const Command &command, Executor &executor) {
   const std::string *string, *ssubstr = nullptr;
   const char *csubstr = nullptr;
   if (!constStringOrError(command, executor, "string-ends-with", string) || !constStringOrCharOrError(command, executor, "string-ends-with", ssubstr, csubstr, 1)) return;
   if (ssubstr) {
      storeBoolean(executor, command, ssubstr->size() <= string->size() && string->rfind(*ssubstr) == string->size() - ssubstr->size(), "string-ends-with");
   }
   else if (csubstr) {
      storeBoolean(executor, command, !string->empty() && string->back() == *csubstr, "string-ends-with");
   }
}

void builtinStringTrim(const Command &command, Executor &executor) {
   std::string *string;
   if (!stringOrError(command, executor, "string-trim", string)) return;
   string->erase(0, string->find_first_not_of(" \n\r\t\v\f"));
   string->erase(string->find_last_not_of(" \n\r\t\v\f") + 1);
}

void builtinStringTolower(const Command &command, Executor &executor) {
   std::string *string;
   if (!stringOrError(command, executor, "string-to-lower", string)) return;
   std::transform(string->begin(), string->end(), string->begin(), toLower);
}

void builtinStringToupper(const Command &command, Executor &executor) {
   std::string *string;
   if (!stringOrError(command, executor, "string-to-upper", string)) return;
   std::transform(string->begin(), string->end(), string->begin(), toUpper);
}

void builtinStringCopy(const Command &command, Executor &executor) {
   const std::string *string;
   if (!constStringOrError(command, executor, "string-copy", string)) return;
   storeString(executor, command, *string, back(executor, command), "string-copy");
}
