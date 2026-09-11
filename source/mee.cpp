#include "pil.hpp"
#include <cmath>

static bool isFloating;

double parseAdditive(Executor&, const std::unordered_map<size_t, Value>&, std::vector<Token>&, size_t&);
double parseExpression(Executor &executor, const std::unordered_map<size_t, Value> &constantMap, std::vector<Token> &tokens, size_t &i) {
   if (tokens[i].type == TOKEN_L_PAREN) {
      i += 1;
      double v = parseAdditive(executor, constantMap, tokens, i);
      if (tokens[i].type != TOKEN_R_PAREN) {
         error(executor.diagnostics, tokens[i].file, tokens[i].line, "Expected Right Parentheses, got %s instead", getTokenName(tokens[i].type));
      }
      i += 1;
      return v;
   }
   
   Value value = parseToken(executor, tokens[i], {}, constantMap);
   i += 1;
   if (value.type != VALUE_INTEGER && value.type != VALUE_FLOATING) {
      error(executor.diagnostics, tokens[i].file, tokens[i].line, "Invalid value in constant evaluator - %s", getValueName(value.type));
      return 0.0;
   }
   if (value.type == VALUE_FLOATING) isFloating = true;
   return (value.type == VALUE_FLOATING ? value.floating : value.integer);
}

double parseUnary(Executor &executor, const std::unordered_map<size_t, Value> &constantMap, std::vector<Token> &tokens, size_t &i) {
   if (tokens[i].type == TOKEN_MINUS || tokens[i].type == TOKEN_PLUS) {
      TokenType type = tokens[i].type;
      i += 1;
      double a = parseUnary(executor, constantMap, tokens, i);
      return (type == TOKEN_MINUS ? -a : a);
   }
   return parseExpression(executor, constantMap, tokens, i);
}

double parseExponentiative(Executor &executor, const std::unordered_map<size_t, Value> &constantMap, std::vector<Token> &tokens, size_t &i) {
   double left = parseUnary(executor, constantMap, tokens, i);
   if (tokens[i].type == TOKEN_CARET) {
      i += 1;
      return pow(left, parseExponentiative(executor, constantMap, tokens, i));
   }
   return left;
}

double parseMultiplicative(Executor &executor, const std::unordered_map<size_t, Value> &constantMap, std::vector<Token> &tokens, size_t &i) {
   double left = parseExponentiative(executor, constantMap, tokens, i);
   while (tokens[i].type == TOKEN_STAR || tokens[i].type == TOKEN_SLASH || tokens[i].type == TOKEN_PERCENT) {
      TokenType type = tokens[i].type;
      i += 1;
      double right = parseExponentiative(executor, constantMap, tokens, i);
      if (right == 0.0 && type != TOKEN_STAR) left = 0.0;
      else left = (type == TOKEN_STAR ? left * right : (type == TOKEN_SLASH ? left / right : fmod(left, right)));
   }
   return left;
}

double parseAdditive(Executor &executor, const std::unordered_map<size_t, Value> &constantMap, std::vector<Token> &tokens, size_t &i) {
   double left = parseMultiplicative(executor, constantMap, tokens, i);
   while (tokens[i].type == TOKEN_PLUS || tokens[i].type == TOKEN_MINUS) {
      TokenType type = tokens[i].type;
      i += 1;
      double right = parseMultiplicative(executor, constantMap, tokens, i);
      left = (type == TOKEN_PLUS ? left + right : left - right);
   }
   return left;
}

Value evaluateMath(Executor &executor, const std::unordered_map<size_t, Value> &constantMap, std::vector<Token> &tokens, size_t &i) {
   isFloating = false;
   i += 1;
   double result = parseAdditive(executor, constantMap, tokens, i);
   if (tokens[i].type != TOKEN_R_BRACKET) {
      error(executor.diagnostics, tokens[i].file, tokens[i].line, "Unterminated constant evaluator");
   }
   Value value {isFloating ? VALUE_FLOATING : VALUE_INTEGER};
   if (isFloating) value.floating = result;
   else value.integer = result;
   return value;
}
