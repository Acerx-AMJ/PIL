#pragma once
#include <cstddef>

enum ValueType {
   VALUE_NULL, VALUE_INTEGER, VALUE_FLOATING, VALUE_CHARACTER, VALUE_STRING, VALUE_FUNCTION, VALUE_NATIVE_FUNCTION,
   VALUE_REGISTER, VALUE_REFERENCE
};

struct Value {
   ValueType type = VALUE_NULL;
   union {
      long integer;
      double floating;
      char character;
      size_t string;
      size_t function;
      size_t nativeFunction;
      size_t reg;
      size_t reference;
   };
};
