#ifndef C_COMMAND_H
#define C_COMMAND_H

extern int client_socket;

void cmd_exit();
void cmd_time();
void cmd_get(char* input);

#endif //C_COMMAND_H
