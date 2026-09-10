#include "builtinhelpers.hpp"
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

// this source file exists for the sole reason of windows.h being so fucking large and bloated
// that clangd struggles to parse it
void setEcho(bool on) {
#ifdef _WIN32
   HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
   DWORD mode;
   GetConsoleMode(hStdin, &mode);
   mode = on ? (mode | ENABLE_ECHO_INPUT) : (mode & ~ENABLE_ECHO_INPUT);
   SetConsoleMode(hStdin, mode);
#else
   termios tty;
   tcgetattr(STDIN_FILENO, &tty);
   if (on) tty.c_lflag |= ECHO;
   else    tty.c_lflag &= ~ECHO;
   tcsetattr(STDIN_FILENO, TCSANOW, &tty);
#endif
}
