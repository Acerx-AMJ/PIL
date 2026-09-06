#pragma once
#include <vector>

enum ValueType: char {
   VALUE_INTEGER, VALUE_FLOATING, VALUE_CHARACTER, VALUE_CSTRING, VALUE_STRING, VALUE_IDENTIFIER, VALUE_LOCAL, VALUE_REGISTER, VALUE_RETURN_REGISTER, VALUE_COUNT
};

constexpr const char *valueTypeStrings[VALUE_COUNT + 1] = {
   "Integer", "Floating", "Character", "CString", "String", "Identifier", "Local Variable", "Register", "Return Register", "Invalid Value"
};

constexpr const char *getValueName(ValueType value) {
   if (value < 0 || value >= VALUE_COUNT) {
      return valueTypeStrings[VALUE_COUNT];
   }
   return valueTypeStrings[value];
}

struct Value {
   ValueType type;
   size_t allocations;
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

enum ParseValueType: char {
   FUNCTION, NATIVE_FUNCTION, LABEL, GLOBAL, PARSE_VALUE_COUNT
};

constexpr const char *parseValueTypeStrings[PARSE_VALUE_COUNT + 1] = {
   "Function", "Native Function", "Label", "Global Variable", "Invalid Parse Value"
};

constexpr const char *getParseValueName(ParseValueType value) {
   if (value < 0 || value >= PARSE_VALUE_COUNT) {
      return parseValueTypeStrings[PARSE_VALUE_COUNT];
   }
   return parseValueTypeStrings[value];
}

typedef void (*NativeFunction)(const struct Command&, struct Executor&);

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
