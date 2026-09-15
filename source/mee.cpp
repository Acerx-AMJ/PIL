#include "pil.hpp"
#include <cmath>
#include <numeric>

static bool isFloating;
double parseOr(Executor&, std::vector<Token>&, size_t&);

struct MEEFunc {
   union { // just works
      double(*f0)();
      double(*f1)(double);
      double(*f2)(double, double);
      double(*f3)(double, double, double);
   };
   int args;
   bool flt = false;
};

static const std::unordered_map<std::string, MEEFunc> MEEFuncMap {
   {"abs", {.f1=fabs, .args=1}},
   {"min", {.f2=fmin, .args=2}},
   {"max", {.f2=fmax, .args=2}},
   {"sqrt", {.f1=sqrt, .args=1, .flt=true}},
   {"cbrt", {.f1=cbrt, .args=1, .flt=true}},
   {"sin", {.f1=sin, .args=1, .flt=true}},
   {"cos", {.f1=cos, .args=1, .flt=true}},
   {"tan", {.f1=tan, .args=1, .flt=true}},
   {"asin", {.f1=asin, .args=1, .flt=true}},
   {"acos", {.f1=acos, .args=1, .flt=true}},
   {"atan", {.f1=atan, .args=1, .flt=true}},
   {"atan2", {.f2=atan2, .args=2, .flt=true}},
   {"asinh", {.f1=asinh, .args=1, .flt=true}},
   {"acosh", {.f1=acosh, .args=1, .flt=true}},
   {"atanh", {.f1=atanh, .args=1, .flt=true}},
   {"sinh", {.f1=sinh, .args=1, .flt=true}},
   {"cosh", {.f1=cosh, .args=1, .flt=true}},
   {"tanh", {.f1=tanh, .args=1, .flt=true}},
   {"clamp", {.f3=[](double x, double lo, double hi){ return (x < lo ? lo : x > hi ? hi : x); }, .args=3}},
   {"sign", {.f1=[](double x){ return (x == 0.0 ? 0.0 : x > 0.0 ? 1.0 : -1.0);}, .args=1}},
   {"trunc", {.f1=trunc, .args=1}},
   {"ceil", {.f1=ceil, .args=1}},
   {"floor", {.f1=floor, .args=1}},
   {"round", {.f1=round, .args=1}},
   {"exp", {.f1=exp, .args=1, .flt=true}},
   {"ln", {.f1=log, .args=1, .flt=true}},
   {"log", {.f2=[](double a, double b){ return log(a) / log(b); }, .args=2, .flt=true}},
   {"log2", {.f1=log2, .args=1, .flt=true}},
   {"log10", {.f1=log10, .args=1, .flt=true}},
   {"lerp", {.f3=[](double a, double b, double t){ return a + (b - a) * t; }, .args=3, .flt=true}},
   {"if", {.f3=[](double cond, double yes, double no){ return cond != 0.0 ? yes : no; }, .args=3}},
   {"pi", {.f0=[]{ return M_PI; }, .args=0, .flt=true}},
   {"tau", {.f0=[]{ return M_PI * 2.0; }, .args=0, .flt=true}},
   {"e", {.f0=[]{ return M_E; }, .args=0, .flt=true}},
   {"hypot", {.f2=hypot, .args=2, .flt=true}},
   {"gcd", {.f2=[](double a, double b) -> double { return std::gcd((long)a, (long)b); }, .args=2}},
   {"lcm", {.f2=[](double a, double b) -> double { return std::lcm((long)a, (long)b); }, .args=2}},
};

double parseExpression(Executor &executor, std::vector<Token> &tokens, size_t &i) {
   if (tokens[i].type == TOKEN_L_PAREN) {
      i += 1;
      double v = parseOr(executor, tokens, i);
      if (tokens[i].type != TOKEN_R_PAREN) {
         error(executor.diagnostics, tokens[i].file, tokens[i].line, "Expected Right Parentheses, got %s instead", getTokenName(tokens[i].type));
      }
      i += 1;
      return v;
   }
   else if (tokens[i].type == TOKEN_IDENTIFIER && tokens[i + 1].type == TOKEN_L_PAREN) {
      std::string &lexeme = getLexeme(executor.cache, tokens[i].lexeme);
      auto it = MEEFuncMap.find(lexeme);
      if (it == MEEFuncMap.end()) {
         error(executor.diagnostics, tokens[i].file, tokens[i].line, "No such constant evaluator function '%s'", lexeme.c_str());
         return 0.0;
      }

      std::vector<double> args;
      for (i += 2; i < tokens.size() && tokens[i].type != TOKEN_EOF && tokens[i].type != TOKEN_R_PAREN;) {
         args.push_back(parseOr(executor, tokens, i));
      }

      if (tokens[i].type != TOKEN_R_PAREN) {
         error(executor.diagnostics, tokens[i].file, tokens[i].line, "Unterminated parentheses");
         return 0.0;
      }

      i += 1;
      if (args.size() != (size_t)it->second.args) {
         error(executor.diagnostics, tokens[i].file, tokens[i].line, "%s: Expected %d parameters, got %zu instead", lexeme.c_str(), it->second.args, args.size());
         return 0.0;
      }

      if (it->second.flt) isFloating = true;
      switch (it->second.args) {
      case 0: return it->second.f0();
      case 1: return it->second.f1(args[0]);
      case 2: return it->second.f2(args[0], args[1]);
      case 3: return it->second.f3(args[0], args[1], args[2]);
      default:
         printf("PIL::parseExpression: Invalid internal constant evaluator function argument count %d", it->second.args);
         exit(EXIT_FAILURE);
      }
   }

   Value value = parseToken(executor, tokens[i], {});
   i += 1;
   if (value.type != VALUE_INTEGER && value.type != VALUE_FLOATING) {
      error(executor.diagnostics, tokens[i].file, tokens[i].line, "Invalid value in constant evaluator - %s", getValueName(value.type));
      return 0.0;
   }
   if (value.type == VALUE_FLOATING) isFloating = true;
   return (value.type == VALUE_FLOATING ? value.floating : value.integer);
}

double parseUnary(Executor &executor, std::vector<Token> &tokens, size_t &i) {
   if (tokens[i].type == TOKEN_MINUS || tokens[i].type == TOKEN_PLUS || tokens[i].type == TOKEN_BNOT || tokens[i].type == TOKEN_LNOT) {
      TokenType type = tokens[i].type;
      i += 1;
      double a = parseUnary(executor, tokens, i);
      if (type == TOKEN_MINUS) return -a;
      else if (type == TOKEN_BNOT) return (double)~(long)a;
      else if (type == TOKEN_LNOT) return a == 0.0;
      else return a;
   }
   return parseExpression(executor, tokens, i);
}

double parseExponentiative(Executor &executor, std::vector<Token> &tokens, size_t &i) {
   double left = parseUnary(executor, tokens, i);
   if (tokens[i].type == TOKEN_STAR_STAR) {
      i += 1;
      return pow(left, parseExponentiative(executor, tokens, i));
   }
   return left;
}

double parseMultiplicative(Executor &executor, std::vector<Token> &tokens, size_t &i) {
   double left = parseExponentiative(executor, tokens, i);
   while (tokens[i].type == TOKEN_STAR || tokens[i].type == TOKEN_SLASH || tokens[i].type == TOKEN_PERCENT) {
      TokenType type = tokens[i].type;
      i += 1;
      double right = parseExponentiative(executor, tokens, i);
      if (right == 0.0 && type != TOKEN_STAR) {
         error(executor.diagnostics, tokens[i].file, tokens[i].line, "Division by zero in constant evaluator");
         left = 0.0;
      }
      else left = (type == TOKEN_STAR ? left * right : (type == TOKEN_SLASH ? left / right : fmod(left, right)));
   }
   return left;
}

double parseAdditive(Executor &executor, std::vector<Token> &tokens, size_t &i) {
   double left = parseMultiplicative(executor, tokens, i);
   while (tokens[i].type == TOKEN_PLUS || tokens[i].type == TOKEN_MINUS) {
      TokenType type = tokens[i].type;
      i += 1;
      double right = parseMultiplicative(executor, tokens, i);
      left = (type == TOKEN_PLUS ? left + right : left - right);
   }
   return left;
}

double parseShifts(Executor &executor, std::vector<Token> &tokens, size_t &i) {
   double left = parseAdditive(executor, tokens, i);
   while (tokens[i].type == TOKEN_BSHL || tokens[i].type == TOKEN_BSHR) {
      TokenType type = tokens[i].type;
      i += 1;
      double right = parseAdditive(executor, tokens, i);
      left = (type == TOKEN_BSHL ? (long)left << (long)right : (long)left >> (long)right);
   }
   return left;
}

double parseBand(Executor &executor, std::vector<Token> &tokens, size_t &i) {
   double left = parseShifts(executor, tokens, i);
   while (tokens[i].type == TOKEN_BAND) {
      i += 1;
      double right = parseShifts(executor, tokens, i);
      left = (long)left & (long)right;
   }
   return left;
}

double parseBxor(Executor &executor, std::vector<Token> &tokens, size_t &i) {
   double left = parseBand(executor, tokens, i);
   while (tokens[i].type == TOKEN_BXOR) {
      i += 1;
      double right = parseBand(executor, tokens, i);
      left = (long)left ^ (long)right;
   }
   return left;
}

double parseBor(Executor &executor, std::vector<Token> &tokens, size_t &i) {
   double left = parseBxor(executor, tokens, i);
   while (tokens[i].type == TOKEN_BOR) {
      i += 1;
      double right = parseBxor(executor, tokens, i);
      left = (long)left | (long)right;
   }
   return left;
}

double parseRelationalOps(Executor &executor, std::vector<Token> &tokens, size_t &i) {
   double left = parseBor(executor, tokens, i);
   while (tokens[i].type == TOKEN_LESSER || tokens[i].type == TOKEN_GREATER || tokens[i].type == TOKEN_LESSER_EQUAL || tokens[i].type == TOKEN_GREATER_EQUAL) {
      TokenType type = tokens[i].type;
      i += 1;
      double right = parseBor(executor, tokens, i);
      if (type == TOKEN_LESSER) left = (left < right);
      else if (type == TOKEN_GREATER) left = (left > right);
      else if (type == TOKEN_LESSER_EQUAL) left = (left <= right);
      else left = (left >= right);
   }
   return left;
}

double parseEqualityOps(Executor &executor, std::vector<Token> &tokens, size_t &i) {
   double left = parseRelationalOps(executor, tokens, i);
   while (tokens[i].type == TOKEN_EQUAL || tokens[i].type == TOKEN_INEQUAL) {
      TokenType type = tokens[i].type;
      i += 1;
      double right = parseRelationalOps(executor, tokens, i);
      left = (left == right) == (type == TOKEN_EQUAL);
   }
   return left;
}

double parseAnd(Executor &executor, std::vector<Token> &tokens, size_t &i) {
   double left = parseEqualityOps(executor, tokens, i);
   while (tokens[i].type == TOKEN_LAND) {
      i += 1;
      double right = parseEqualityOps(executor, tokens, i);
      left = (left != 0.0 && right != 0.0);
   }
   return left;
}

double parseOr(Executor &executor, std::vector<Token> &tokens, size_t &i) {
   double left = parseAnd(executor, tokens, i);
   while (tokens[i].type == TOKEN_LOR) {
      i += 1;
      double right = parseAnd(executor, tokens, i);
      left = (left != 0.0 || right != 0.0);
   }
   return left;
}

Value evaluateMath(Executor &executor, std::vector<Token> &tokens, size_t &i) {
   isFloating = false;
   i += 1;
   double result = parseOr(executor, tokens, i);
   if (tokens[i].type != TOKEN_R_BRACKET && tokens[i].type != TOKEN_EVAL_END) {
      error(executor.diagnostics, tokens[i].file, tokens[i].line, "Unterminated constant evaluator. Expected Right Bracket, got %s instead", getTokenName(tokens[i].type));
   }
   Value value {isFloating ? VALUE_FLOATING : VALUE_INTEGER};
   if (isFloating) value.floating = result;
   else value.integer = result;
   return value;
}
