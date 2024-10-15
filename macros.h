#ifndef CONSTS_H
#define CONSTS_H

#define PORT 57260
#define BUFFER_SIZE 1024
#define PROGRESS_BAR_LENGTH 50
#define IP "127.0.0.1"
#define MAX_NAME_LEN 30
#define MAX_PASSWORD_LEN 30
#define MAX_CONNECTIONS 100
#define MAX_REG_USERS 1000
#define MAX_COMMAND_LEN 15
#define MAX_ARG_COUNT 20

// terminal colors
#define COLOR_RESET "\033[0m"
#define COLOR_BLACK "\033[30m"
#define COLOR_RED "\033[31m"
#define COLOR_GREEN "\033[32m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_BLUE "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN "\033[36m"
#define COLOR_WHITE "\033[37m"

#define COLOR(x) "\033[38;5;xm"

#endif //CONSTS_H
