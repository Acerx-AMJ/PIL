#pragma once
#include <vector>

enum ValueType {
   VALUE_NULL, VALUE_INTEGER, VALUE_FLOATING, VALUE_CHARACTER, VALUE_STRING, VALUE_REGISTER, VALUE_REFERENCE, VALUE_COUNT
};

constexpr const char *getValueName(ValueType value) {
   constexpr const char *valueTypeStrings[VALUE_COUNT + 1] = {
      "Null", "Integer", "Floating", "Character", "String", "Register", "Reference", "Invalid Value"
   };

   if (value < 0 || value >= VALUE_COUNT) {
      return valueTypeStrings[VALUE_COUNT];
   }
   return valueTypeStrings[value];
}

struct Value {
   ValueType type = VALUE_NULL;
   union {
      long integer;
      double floating;
      char character;
      size_t string;
      size_t reg;
      size_t reference;
   };
};

struct Command {
   Command(size_t lexeme, const std::vector<Value> &values)
      : lexeme(lexeme), values(values) {}

   size_t lexeme;
   std::vector<Value> values;
};

struct Block {
   Block(size_t lexeme, size_t tokenPosition)
      : lexeme(lexeme), tokenPosition(tokenPosition) {}
   Block(size_t lexeme, size_t tokenPosition, const std::vector<size_t>& params, const std::vector<Command>& commands)
      : lexeme(lexeme), tokenPosition(tokenPosition), params(params), commands(commands) {}

   size_t lexeme;
   size_t tokenPosition;
   std::vector<size_t> params;
   std::vector<Command> commands;
};

typedef void (*NativeFunction)(const std::vector<Value> &args);
struct Function {
   bool native;
   union {
      size_t function;
      NativeFunction nativeFunction;
   };
};
