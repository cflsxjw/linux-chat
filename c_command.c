#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "utils.h"
#include "c_command.h"
#include "macros.h"

void cmd_exit() {
    close(client_socket);
    exit(0);
}
void cmd_time() {
    char local_time[40];
    get_localtime(local_time);
    printf("%s", local_time);
}
void cmd_get(char* input) {
    write(client_socket,input,BUFFER_SIZE);
}