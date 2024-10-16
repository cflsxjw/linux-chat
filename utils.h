

#ifndef UTILS_H
#define UTILS_H



void get_localtime(char* buffer);
int split_command(const char *src, char *cmd, char **args);
void wrap_msg(char *msg);
void unwrap_msg(char *msg);
void disable_echo();
void enable_echo();

#endif //UTILS_H
