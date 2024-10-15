#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "utils.h"

void get_localtime(char* buffer) {
    const time_t curr_time = time(NULL);
    const struct tm* curr_localtime = localtime(&curr_time);
    strftime(buffer, 40, "%H:%M:%S", curr_localtime);
}

int split_command(const char *src, char *cmd, char **args)
{
    if (args == NULL)
    {
        printf("split_command: args is NULL");
        return -1;
    }
    int args_index = 0;
    int curr_arg_head = 1;
    int curr_index = 0;
    int has_arg = 0;
    // read command
    while (src[curr_index] != '\0')
    {
        if (src[curr_index] == ' ')
        {
            has_arg = 1;
            curr_arg_head = curr_index + 1;
            break;
        }
        curr_index++;
    }
    strncpy(cmd, src + 1, curr_index - 1);
    if (!has_arg)
    {
        return 0;
    }
    while (src[curr_index] != '\0')
    {
        if (src[curr_index] == ' ')
        {
            if (curr_arg_head < curr_index)
            {
                args[args_index] = malloc(sizeof(char) * (curr_index - curr_arg_head + 1));
                strncpy(args[args_index], src + curr_arg_head, curr_index - curr_arg_head);
                args[args_index][curr_index - curr_arg_head] = 0;
                args_index++;
            }
            curr_arg_head = curr_index + 1;
        }
        curr_index++;
    }
    if (curr_arg_head < curr_index)
    {
        args[args_index] = malloc(sizeof(char) * (curr_index - curr_arg_head + 1));
        strncpy(args[args_index], src + curr_arg_head, curr_index - curr_arg_head);
        args[args_index][curr_index - curr_arg_head] = 0;
    }
    return args_index + 1;
}

void wrap_msg(char *msg) {
    for (int i = 0; i < strlen(msg); i++) {
        if (msg[i] == ' ') {
            msg[i] = 127;
        }
    }
}

void unwrap_msg(char *msg) {
    for (int i = 0; i < strlen(msg); i++) {
        if (msg[i] == 127) {
            msg[i] = ' ';
        }
    }
}