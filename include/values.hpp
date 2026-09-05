#pragma once
#include <vector>

enum ValueType: char {
   VALUE_INTEGER, VALUE_FLOATING, VALUE_CHARACTER, VALUE_CSTRING, VALUE_STRING, VALUE_IDENTIFIER, VALUE_LOCAL, VALUE_REGISTER, VALUE_RETURN_REGISTER, VALUE_COUNT
};

enum ParseValueType: char {
   FUNCTION, NATIVE_FUNCTION, LABEL, GLOBAL, PARSE_VALUE_COUNT
};

constexpr const char *getValueName(ValueType value) {
   constexpr const char *valueTypeStrings[VALUE_COUNT + 1] = {
      "Integer", "Floating", "Character", "CString", "String", "Identifier", "Local Variable", "Register", "Return Register", "Invalid Value"
   };

   if (value < 0 || value >= VALUE_COUNT) {
      return valueTypeStrings[VALUE_COUNT];
   }
   return valueTypeStrings[value];
}

constexpr const char *getParseValueName(ParseValueType value) {
   constexpr const char *parseValueTypeStrings[PARSE_VALUE_COUNT + 1] = {
      "Function", "Native Function", "Label", "Global Variable", "Invalid Parse Value"
   };

   if (value < 0 || value >= PARSE_VALUE_COUNT) {
      return parseValueTypeStrings[PARSE_VALUE_COUNT];
   }
   return parseValueTypeStrings[value];
}

struct Value {
   ValueType type;
   size_t line;
   size_t file;
   union {
      long integer;
      double floating;
      char character;
      size_t string; // reused for strings and cstrings
      size_t identifier;
      size_t local;
      size_t reg; // reused for registers and return registers
   };
};

struct Command {
   Command(size_t lexeme, size_t file, size_t line, const std::vector<Value> &args)
      : lexeme(lexeme), file(file), line(line), args(args) {}

   size_t lexeme;
   size_t file;
   size_t line;
   std::vector<Value> args;
};

typedef void (*NativeFunction)(const Command&, struct Executor&);

struct ParseValue {
   ParseValueType type;
   bool init = false;
   bool variadic = false;
   bool reserved = false;
   std::vector<size_t> params;
   union {
      size_t label;
      Value local;
      Value global;
      struct { size_t function, localCount; };
      NativeFunction nativeFunction;
   };
};
