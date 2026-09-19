#include "pil.hpp"
#include <fstream>

typedef unsigned short file_string_size_t;

// write cache bytecode
void writeToFile(Executor &executor, const std::string &out) {
   std::ofstream file (out, std::ios::binary);
   if (!file.is_open()) {
      error(executor.diagnostics, 0, 0, "Cannot write to file '%s'", out.c_str());
      return;
   }
   size_t regSize = (executor.registers.empty() ? DEFAULT_REGISTER_COUNT : executor.registers.size());
   size_t returnRegSize = (executor.returnRegisters.empty() ? DEFAULT_RETURN_REGISTER_COUNT : executor.returnRegisters.size());

   file.write(reinterpret_cast<const char*>(&FILE_VERSION), sizeof(FILE_VERSION));
   file.write(reinterpret_cast<const char*>(&regSize), sizeof(regSize));
   file.write(reinterpret_cast<const char*>(&returnRegSize), sizeof(returnRegSize));
   file.write(reinterpret_cast<const char*>(&executor.main), sizeof(executor.main));

   size_t lexemeCount = executor.cache.lexemes.size();
   file.write(reinterpret_cast<const char*>(&lexemeCount), sizeof(lexemeCount));

   for (const std::string &lexeme: executor.cache.lexemes) {
      file_string_size_t size = lexeme.size();
      file.write(reinterpret_cast<const char*>(&size), sizeof(size));
      file.write(reinterpret_cast<const char*>(lexeme.data()), size);
   }

   size_t functionCount = executor.functions.size();
   file.write(reinterpret_cast<const char*>(&functionCount), sizeof(functionCount));
   file.write(reinterpret_cast<const char*>(executor.functions.data()), sizeof(Function) * functionCount);

   size_t argumentCount = executor.arguments.size();
   file.write(reinterpret_cast<const char*>(&argumentCount), sizeof(argumentCount));
   file.write(reinterpret_cast<const char*>(executor.arguments.data()), sizeof(Value) * argumentCount);

   size_t commandCount = executor.code.size();
   file.write(reinterpret_cast<const char*>(&commandCount), sizeof(commandCount));
   file.write(reinterpret_cast<const char*>(executor.code.data()), sizeof(Command) * commandCount);
}

// read cached bytecode
void readCachedBytecode(Executor &executor, const std::string &in) {
   std::ifstream file (in, std::ios::binary);
   if (!file.is_open()) {
      error(executor.diagnostics, 0, 0, "Cannot read file '%s'", in.c_str());
      return;
   }

   size_t version, regSize, returnRegSize;
   file.read(reinterpret_cast<char*>(&version), sizeof(version));
   if (version != FILE_VERSION) {
      error(executor.diagnostics, 0, 0, "File '%s' is using an outdated version %zu. The newest version is %zu. Recompile the project", in.c_str(), version, FILE_VERSION);
      return;
   }

   file.read(reinterpret_cast<char*>(&regSize), sizeof(regSize));
   file.read(reinterpret_cast<char*>(&returnRegSize), sizeof(returnRegSize));
   file.read(reinterpret_cast<char*>(&executor.main), sizeof(executor.main));

   size_t lexemeCount;
   file.read(reinterpret_cast<char*>(&lexemeCount), sizeof(lexemeCount));
   executor.cache.lexemes.resize(lexemeCount);

   for (size_t i = 0; i < lexemeCount; ++i) {
      file_string_size_t size;
      file.read(reinterpret_cast<char*>(&size), sizeof(size));
      executor.cache.lexemes[i].resize(size);
      file.read(reinterpret_cast<char*>(executor.cache.lexemes[i].data()), size);
   }

   size_t functionCount;
   file.read(reinterpret_cast<char*>(&functionCount), sizeof(functionCount));
   executor.functions.resize(functionCount);
   file.read(reinterpret_cast<char*>(executor.functions.data()), sizeof(Function) * functionCount);
   
   size_t argumentCount;
   file.read(reinterpret_cast<char*>(&argumentCount), sizeof(argumentCount));
   executor.arguments.resize(argumentCount);
   file.read(reinterpret_cast<char*>(executor.arguments.data()), sizeof(Value) * argumentCount);
   
   size_t commandCount;
   file.read(reinterpret_cast<char*>(&commandCount), sizeof(commandCount));
   executor.code.resize(commandCount);
   file.read(reinterpret_cast<char*>(executor.code.data()), sizeof(Command) * commandCount);
}
