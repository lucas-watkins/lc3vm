//
// Created by Lucas Watkins on 8/17/25.
//

#ifndef LC3VM_PLATFORMSPECIFIC_HPP
#define LC3VM_PLATFORMSPECIFIC_HPP

#if defined(__unix__) || defined(__linux__) || defined(__APPLE__)
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/termios.h>
#include <sys/mman.h>
#elif defined(_WIN32)
#include <Windows.h>
#include <conio.h>
#endif

#if defined(__unix__) || defined(__linux__) || defined(__APPLE__)
//inline termios original_tio {};
#elif defined(_WIN32)
HANDLE hStdin = INVALID_HANDLE_VALUE;
DWORD fdwMode, fdwOldMode;
#else
#error Unrecognized OS
#endif

/*inline std::uint16_t check_key() {
#if defined(__unix__) || defined(__linux__) || defined(__APPLE__)
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    return select(1, &readfds, NULL, NULL, &timeout) != 0;
#elif defined(_WIN32)
    return WaitForSingleObject(hStdin, 1000) == WAIT_OBJECT_0 && _kbhit();
#else
#error Unrecognized OS
#endif
}*/

/*inline void disable_input_buffering() {
#if defined(__unix__) || defined(__linux__) || defined(__APPLE__)
    tcgetattr(STDIN_FILENO, &original_tio);
    struct termios new_tio = original_tio;
    new_tio.c_lflag &= ~ICANON & ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);
#elif defined(_WIN32)
    hStdin = GetStdHandle(STD_INPUT_HANDLE);
    GetConsoleMode(hStdin, &fdwOldMode); // save old mode
    fdwMode = fdwOldMode
            ^ ENABLE_ECHO_INPUT  // no input echo
            ^ ENABLE_LINE_INPUT; // return when one or more characters are available
    SetConsoleMode(hStdin, fdwMode); // set new mode
    FlushConsoleInputBuffer(hStdin); // clear buffer
#else
#error Unrecognized OS
#endif
}*/

/*inline void restore_input_buffering() {
#if defined(__unix__) || defined(__linux__) || defined(__APPLE__)
    tcsetattr(STDIN_FILENO, TCSANOW, &original_tio);
#elif defined(_WIN32)
    SetConsoleMode(hStdin, fdwOldMode);
#else
#error Unrecognized OS
#endif
}*/

#endif //LC3VM_PLATFORMSPECIFIC_HPP