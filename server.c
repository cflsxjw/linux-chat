#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "utils.h"
#include "macros.h"

typedef struct USER{
    int id;
    int socket_fd;
    char name[MAX_NAME_LEN];
} user;

user* users[MAX_CONNECTIONS];
int id_pool[MAX_CONNECTIONS];

void *new_user_thread(void *arg);
void broadcast(const char* msg);
void command_handler(char *cmd,char **args,int argc);

int pool_head = 0;

int main() {
    // init users list
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        users[i] = malloc(sizeof(user));
        users[i]->socket_fd = 0;
        strcpy(users[i]->name, "");
    }
    for (int i = 0; i < MAX_CONNECTIONS; i++) {id_pool[i] = i;}
    const int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("error: socket()");
    }
    struct sockaddr_in serverAddress = {AF_INET, PORT, inet_addr(IP)};
    if(bind(server_socket, (struct sockaddr*)&serverAddress, sizeof(serverAddress))) {
        perror("error: bind()");
    }
    listen(server_socket, 5);
    while (1) {
        struct sockaddr_in client_address;
        int len = sizeof(client_address);
        const int socket_fd = accept(server_socket, (struct sockaddr *) &client_address, (socklen_t *)&len);
        if (pool_head >= MAX_CONNECTIONS) {
            const char* msg = "\nchannel is full...\nconnection closed\n"; //NOLINT
            write(socket_fd, msg ,strlen(msg));
            close(socket_fd);
            continue;
        }
        const int curr_id = id_pool[pool_head]; // assign id
        pool_head++; // new connection created! pool--

        // new user log in
        users[curr_id]->id = curr_id;
        users[curr_id]->socket_fd = socket_fd;
        read(users[curr_id]->socket_fd, users[curr_id]->name, MAX_NAME_LEN);

        char msg[64];
        sprintf(msg, "%s has joined in\n", users[curr_id]->name);
        broadcast(msg);
        // create new thread
        pthread_t thread;
        pthread_create(&thread, NULL, new_user_thread, users[curr_id]);
    }
    close(server_socket);
}
void *new_user_thread(void *arg) { //NOLINT
    const user* curr_user = arg;
    while(1) {
        if(curr_user->socket_fd < 0) {
            perror("error: accept()");
        }
        char buffer[BUFFER_SIZE];
        if(read(curr_user->socket_fd, buffer, BUFFER_SIZE) == 0) {
            break;
        }
        if (strcmp(buffer, "") != 0) {

            if (buffer[0] == '/')
            {
                char **args = malloc(sizeof(char *) * 9);
                char *cmd = malloc(sizeof(char) * 16);
                int argc = split_command(buffer, cmd, args);
                
                command_handler(cmd,args,argc);

                continue;
            }

            char msg[BUFFER_SIZE];
            char* time_str = malloc(40 * sizeof(char));
            get_localtime(time_str);

            snprintf(msg, BUFFER_SIZE+128, "\033[36m[%s]\033[0m \033[32m%s\033[0m: %s\n", time_str, curr_user->name, buffer);


            free(time_str);
            printf("%s",msg);
            broadcast(msg);
        }
    }
    pool_head--; // connection exit: pool++
    id_pool[pool_head] = curr_user->id;
    char* time_str = malloc(40 * sizeof(char));
    get_localtime(time_str);
    printf("%s quit  [%s]\n", curr_user->name, time_str);
    free(time_str);
    pthread_exit(NULL);
}

void broadcast(const char* msg) {
    for(int i = 0; i < MAX_CONNECTIONS; i++) {
        if(users[i]->socket_fd > 0) {
            write(users[i]->socket_fd, msg, BUFFER_SIZE);
        }
    }
}

void command_handler(char *cmd,char **args,int argc)
{
    if(strcmp(cmd,"get")==0 && argc ==1 )
    {
        broadcast(strcat(args[0], "\n"));
    }
}