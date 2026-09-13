#include "builtin.hpp"
#include "pil.hpp"

// we only define built-in functions that actually get used. thanks, cache. there are reserved built-ins that
// always get pushed
void pushBuiltin(Executor &executor, const BuiltinDef &def) {
   size_t functionId = executor.functions.size();
   Function function;
   function.init = true;
   function.native = true;
   function.variadic = def.variadic;
   function.nativeFunction = def.fn;

   Value value {VALUE_FUNCTION};
   value.function = functionId;

   if (def.reserved) {
      size_t cached = cacheLexeme(executor.cache, def.name);
      function.lexeme = cached;
      function.params.resize(def.params);
      executor.functions.push_back(function);
      executor.constants[cached] = value;
   }
   else if (auto it = executor.cache.lexemeCache.find(def.name); it != executor.cache.lexemeCache.end()) {
      function.lexeme = it->second;
      function.params.resize(def.params);
      executor.functions.push_back(function);
      executor.constants[it->second] = value;
   }
}

Value parseToken(Executor &executor, Token token, const std::unordered_map<size_t, size_t> &functionParamMap, const std::unordered_map<size_t, Value> &constantMap) {
   Value value {VALUE_COUNT};
   switch (token.type) {
   case TOKEN_IDENTIFIER:
      if (auto it = functionParamMap.find(token.lexeme); it != functionParamMap.end()) {
         value.type = VALUE_LOCAL;
         value.local = it->second;
      }
      else if (auto it = constantMap.find(token.lexeme); it != constantMap.end()) {
         return it->second;
      }
      else {
         error(executor.diagnostics, token.file, token.line, "Variable '%s' does not exist", getLexeme(executor.cache, token.lexeme).c_str());
      }
      break;
   case TOKEN_INTEGER:
      value.type = VALUE_INTEGER;
      try {
         value.integer = std::stol(getLexeme(executor.cache, token.lexeme));
      }
      catch (...) {
         value.integer = 0;
         error(executor.diagnostics, token.file, token.line, "Invalid integer: %s", getLexeme(executor.cache, token.lexeme).c_str());
      }
      break;
   case TOKEN_FLOATING:
      value.type = VALUE_FLOATING;
      try {
         value.floating = std::stod(getLexeme(executor.cache, token.lexeme));
      }
      catch (...) {
         value.floating = 0;
         error(executor.diagnostics, token.file, token.line, "Invalid floating point number: %s", getLexeme(executor.cache, token.lexeme).c_str());
      }
      break;
   case TOKEN_STRING:
      value.type = VALUE_CSTRING;
      value.string = token.lexeme;
      break;
   case TOKEN_CHARACTER:
      value.type = VALUE_CHARACTER;
      value.character = getLexeme(executor.cache, token.lexeme).front();
      break;
   case TOKEN_RETURN_REGISTER:
   case TOKEN_REGISTER: {
      std::vector<Value> &container = (token.type == TOKEN_RETURN_REGISTER ? executor.returnRegisters : executor.registers);
      size_t maxDefaultValue = (token.type == TOKEN_RETURN_REGISTER ? DEFAULT_RETURN_REGISTER_COUNT : DEFAULT_REGISTER_COUNT);
      size_t maxValue = (container.empty() ? maxDefaultValue : container.size());

      value.type = (token.type == TOKEN_RETURN_REGISTER ? VALUE_RETURN_REGISTER : VALUE_REGISTER);
      try {
         value.reg = std::stoull(getLexeme(executor.cache, token.lexeme));
      }
      catch (...) {
         value.reg = 0;
         error(executor.diagnostics, token.file, token.line, "Invalid register: %s$%s", token.type == TOKEN_RETURN_REGISTER ? "R" : "", getLexeme(executor.cache, token.lexeme).c_str());
      }

      if (value.reg >= maxValue) {
         error(executor.diagnostics, token.file, token.line, "Register %s$%zu is out of bounds", token.type == TOKEN_RETURN_REGISTER ? "R" : "", value.reg);
      }
      break;
   }
   default:
      error(executor.diagnostics, token.file, token.line, "Unexpected token %s in function call", getLexeme(executor.cache, token.lexeme).c_str());
   }
   return value;
}

// take the tokens and turn them into executable function blocks and commands. we have 3 levels here: file -> functions ->
// commands. there can be no commands in the file level and no functions in the command level.
void parsePIL(Executor &executor, std::vector<Token> &tokens) {   
   // estimate code size. some rough estimates
   size_t size = tokens.size();
   executor.code.reserve(size / 3);
   executor.arguments.reserve(size / 4);
   executor.functions.reserve(size / 16 + 4); // I don't remember why I put it as this anymore

   for (const BuiltinDef &def: BUILTIN_DEFINITIONS) {
      pushBuiltin(executor, def);
   }

   // function name and label prepass
   std::unordered_map<size_t, size_t> functionParamMap;

   for (size_t i = 0; i < size; ++i) {
      if (tokens[i].type == TOKEN_IDENTIFIER && (tokens[i + 1].type == TOKEN_L_PAREN || tokens[i + 1].type == TOKEN_LABEL)) {
         size_t position = tokens[i].lexeme;
         if (auto it = executor.constants.find(position); it != executor.constants.end()) {
            error(executor.diagnostics, tokens[i].file, tokens[i].line, "%s '%s' redefined", getValueName(it->second.type), getLexeme(executor.cache, position).c_str());
         }

         size_t functionId = executor.functions.size();
         Function function;
         function.init = true;
         function.isLabel = (tokens[i + 1].type == TOKEN_LABEL);
         function.lexeme = position;

         if (function.isLabel) {
            Value value {VALUE_LABEL};
            value.label = functionId;
            executor.constants[position] = value;
         }
         else {
            Value value {VALUE_FUNCTION};
            value.function = functionId;
            executor.constants[position] = value;
         }
         executor.functions.push_back(function);
      }
   }

   // real parsing
   size_t returnLexeme = cacheLexeme(executor.cache, "return");
   size_t returnId = executor.constants[returnLexeme].function;
   size_t returnRegisterCount = (executor.returnRegisters.empty() ? DEFAULT_RETURN_REGISTER_COUNT : executor.returnRegisters.size());

   size_t defineLexeme = cacheLexeme(executor.cache, "let");
   size_t callLexeme = cacheLexeme(executor.cache, "call");
   size_t constLexeme = cacheLexeme(executor.cache, "const");
   bool firstFunction = true;

   for (size_t i = 0; i < size && tokens[i].type != TOKEN_EOF; ++i) {
      // skip extraneous newlines
      while (i < size && tokens[i].type == TOKEN_NEWLINE) ++i;
      if (tokens[i].type == TOKEN_EOF) break;

      // labels
      if (tokens[i].type == TOKEN_IDENTIFIER && tokens[i + 1].type == TOKEN_LABEL) {
         size_t start = i;
         Function &label = executor.functions[executor.constants[tokens[i].lexeme].label];
         label.position = executor.code.size();

         i += 2;
         if (i >= size || tokens[i].type != TOKEN_NEWLINE) {
            error(executor.diagnostics, tokens[start].file, tokens[start].line, "Excess tokens (or EOF) after label");
         }
      }
      // function declarations
      else if (tokens[i].type == TOKEN_IDENTIFIER && tokens[i + 1].type == TOKEN_L_PAREN) {
         size_t functionId = executor.constants[tokens[i].lexeme].function;
         if (!firstFunction && (executor.code.empty() || executor.code.back().lexeme != returnLexeme)) {
            executor.code.emplace_back(returnLexeme, tokens[i-1].file, tokens[i-1].line, 0, 0, returnId);
         }
         size_t start = i;
         Function &function = executor.functions[functionId];
         bool variadic = false;
         firstFunction = false;
         functionParamMap.clear();

         for (i += 2; i < size && tokens[i].type != TOKEN_EOF && tokens[i].type != TOKEN_R_PAREN; ++i) {
            if (tokens[i].type == TOKEN_VARIADIC) {
               variadic = true;
               i += 1;
               break;
            }

            if (tokens[i].type != TOKEN_IDENTIFIER) {
               error(executor.diagnostics, tokens[i].file, tokens[i].line, "Expected Identifier, got %s instead", getTokenName(tokens[i].type));
            }

            if (functionParamMap.find(tokens[i].lexeme) != functionParamMap.end() || executor.constants.find(tokens[i].lexeme) != executor.constants.end()) {
               error(executor.diagnostics, tokens[i].file, tokens[i].line, "Redefined function parameter '%s'", getLexeme(executor.cache, tokens[i].lexeme).c_str());
            }
            function.params.push_back(tokens[i].lexeme);
            functionParamMap[tokens[i].lexeme] = functionParamMap.size();
         }

         if (variadic && tokens[i].type != TOKEN_R_PAREN) {
            error(executor.diagnostics, tokens[i].file, tokens[i].line, "Variadic parameter (...) should be at the end of the parameter list");
         }
         else if (!variadic && tokens[i].type != TOKEN_R_PAREN) {
            error(executor.diagnostics, tokens[start].file, tokens[start].line, "Unterminated function parameters");
         }

         // variable declarations
         i += 1;
         if (tokens[i].type == TOKEN_IDENTIFIER && tokens[i].lexeme == defineLexeme) {
            for (++i; i < size && tokens[i].type != TOKEN_EOF && tokens[i].type != TOKEN_NEWLINE; ++i) {
               if (tokens[i].type != TOKEN_IDENTIFIER) {
                  error(executor.diagnostics, tokens[i].file, tokens[i].line, "Expected unique Identifier, got %s instead", getTokenName(tokens[i].type));
                  continue;
               }

               if (functionParamMap.find(tokens[i].lexeme) != functionParamMap.end() || executor.constants.find(tokens[i].lexeme) != executor.constants.end()) {
                  error(executor.diagnostics, tokens[i].file, tokens[i].line, "Redefined define '%s'", getLexeme(executor.cache, tokens[i].lexeme).c_str());
                  continue;
               }
               functionParamMap[tokens[i].lexeme] = functionParamMap.size();
            }
         }
         function.variadic = variadic;
         function.position = executor.code.size();
         function.localCount = functionParamMap.size();

         if (i >= size || tokens[i].type != TOKEN_NEWLINE) {
            error(executor.diagnostics, tokens[start].file, tokens[start].line, "Excess tokens (or EOF) after function definition");
         }
      }
      // const declaration
      else if (tokens[i].type == TOKEN_IDENTIFIER && tokens[i].lexeme == constLexeme) {
         i += 1;
         size_t lexeme = tokens[i].lexeme;
         if (tokens[i].type != TOKEN_IDENTIFIER) {
            error(executor.diagnostics, tokens[i].file, tokens[i].line, "Expected identifier after const keyword, got %s instead", getTokenName(tokens[i].type));
            continue; // might be EOF
         }
         i += 1;
         TokenType type = tokens[i].type;
         if (type == TOKEN_NEWLINE || type == TOKEN_EOF || type == TOKEN_REGISTER || type == TOKEN_RETURN_REGISTER) {
            error(executor.diagnostics, tokens[i].file, tokens[i].line, "Expected a constant value in the constant declaration, got %s instead", getTokenName(tokens[i].type));
            continue;
         }

         if (type == TOKEN_L_BRACKET) {
            executor.constants[lexeme] = evaluateMath(executor, executor.constants, tokens, i);
         }
         else {
            executor.constants[lexeme] = parseToken(executor, tokens[i], {}, executor.constants); // functionParamMap handles runtime values, not constants;
         }
      }
      // function calls
      else {
         auto it = executor.constants.find(tokens[i].lexeme);
         if (it == executor.constants.end()) {
            if (tokens[i].type == TOKEN_IDENTIFIER) {
               error(executor.diagnostics, tokens[i].file, tokens[i].line, "No such function '%s'", getLexeme(executor.cache, tokens[i].lexeme).c_str());
            }
            else {
               error(executor.diagnostics, tokens[i].file, tokens[i].line, "Expected a function call, got %s instead", getTokenName(tokens[i].type));
            }
            // to not spiral errors out of control
            while (i < size && tokens[i].type != TOKEN_EOF && tokens[i].type != TOKEN_NEWLINE) i += 1;
            i -= 1;
            continue;
         }

         executor.code.emplace_back(tokens[i].lexeme, tokens[i].file, tokens[i].line, executor.arguments.size(), 0, it->second.function);
         Command &command = executor.code.back();
         bool isCall = (tokens[i].lexeme == callLexeme);
         bool isReturn = (tokens[i].lexeme == returnLexeme);
         size_t start = i + 1;

         for (++i; i < size && tokens[i].type != TOKEN_EOF && tokens[i].type != TOKEN_NEWLINE; ++i) {
            Value value;
            if (tokens[i].type == TOKEN_L_BRACKET) {
               value = evaluateMath(executor, executor.constants, tokens, i);
            }
            else {
               value = parseToken(executor, tokens[i], functionParamMap, executor.constants);
            }

            if (isCall && value.type == VALUE_FUNCTION) {
               if (command.callee != std::string::npos) {
                  error(executor.diagnostics, command.file, command.line, "call: Cannot call multiple functions in a single call");
               }
               command.callee = i - start;
            }

            executor.arguments.push_back(value);
            command.argCount += 1;
         }

         if (isCall && command.callee == std::string::npos) {
            error(executor.diagnostics, command.file, command.line, "call: Expected function name to call");
         }

         size_t args = command.argCount;
         size_t params = executor.functions[it->second.function].params.size();
         bool variadic = executor.functions[it->second.function].variadic;

         if ((!variadic && args != params) || (variadic && args < params)) {
            error(executor.diagnostics, command.file, command.line, "Function '%s' expected %s%zu parameters, but received %zu arguments", getLexeme(executor.cache, command.lexeme).c_str(), (variadic ? ">" : ""), params, args);
         }

         if (isCall) {
            size_t idx = executor.arguments[command.argStart + command.callee].function;
            Function &function = executor.functions[idx];
            args = command.argCount - command.callee - 1;
            params = function.params.size();
            variadic = function.variadic;
            if ((!variadic && args != params) || (variadic && args < params)) {
               error(executor.diagnostics, command.file, command.line, "call: Function '%s' expected %s%zu parameters, but received %zu arguments", getLexeme(executor.cache, function.lexeme).c_str(), (variadic ? ">" : ""), params, args);
            }
         }
         else if (isReturn && args > returnRegisterCount) {
            error(executor.diagnostics, command.file, command.line, "return: Can return at maximum %zu values. Define 'return-register-count %zu' directive to mitigate. Error", returnRegisterCount, args);
         }
      }
   }
   if (!tokens.empty()) {
      executor.code.emplace_back(returnLexeme, tokens.back().file, tokens.back().line, 0, 0, returnId);
   }
}
