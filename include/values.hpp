#pragma once
#include <vector>

enum ValueType: char {
   VALUE_INTEGER, VALUE_FLOATING, VALUE_CHARACTER, VALUE_STRING, VALUE_IDENTIFIER, VALUE_REGISTER, VALUE_COUNT
};

constexpr const char *getValueName(ValueType value) {
   constexpr const char *valueTypeStrings[VALUE_COUNT + 1] = {
      "Integer", "Floating", "Character", "String", "Identifier", "Register", "Invalid Value"
   };

   if (value < 0 || value >= VALUE_COUNT) {
      return valueTypeStrings[VALUE_COUNT];
   }
   return valueTypeStrings[value];
}

struct Value {
   ValueType type = VALUE_COUNT;
   size_t line;
   size_t fileLexeme;
   union {
      long integer;
      double floating;
      char character;
      size_t string;
      size_t identifier;
      size_t reg;
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

typedef void (*NativeFunction)(const Command&, struct Diagnostics&, struct Executor&);

struct Function {
   bool init = false;
   bool native;
   bool variadic = false;
   bool reserved = false;
   std::vector<size_t> params;
   union {
      size_t function;
      NativeFunction nativeFunction;
   };
};
