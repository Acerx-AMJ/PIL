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
   Command() = default;
   Command(size_t lexeme, const std::vector<Value> &values)
      : lexeme(lexeme), values(values) {}

   size_t lexeme;
   std::vector<Value> values;
};

struct Block {
   Block() = default;
   Block(size_t lexeme, size_t tokenPosition)
      : lexeme(lexeme), tokenPosition(tokenPosition) {}

   size_t lexeme;
   size_t tokenPosition;
   std::vector<size_t> params;
   std::vector<Command> commands;
};

typedef void (*NativeFunction)(const std::vector<Value> &args);
struct Function {
   bool init = false;
   bool native;
   union {
      size_t function;
      NativeFunction nativeFunction;
   };
};
