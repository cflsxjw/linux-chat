#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "utils.h"
#include "macros.h"

typedef struct USER {
    int id;
    int socket_fd;
    char name[MAX_NAME_LEN];
    char **mute;
    int verified;
    int valid;
} user;

typedef struct REG_USER {
    char name[MAX_NAME_LEN];
    char password[MAX_PASSWORD_LEN];
} reg_user;

reg_user *reg_users[MAX_REG_USERS];
user *users[MAX_CONNECTIONS];
int id_pool[MAX_CONNECTIONS];

int reuse = 1;

void *new_user_thread(void *arg);

void logout_user(user *curr_user);

void broadcast(const char *msg);

void command_handler(char *cmd, char **args, int argc, user *curr_user);

int occupancy_test(char* name, char* password, int socket_fd);

int check_userinfo(char *userinfo, char *name, char *password);

int reg_count = 0;
int pool_head = 0;

int main() {
    // 读取注册用户
    FILE *reg_user_list = fopen("users", "r");
    if (reg_user_list == NULL) {
        perror("fopen");
        exit(1);
    }
    for (reg_count = 0; ; reg_count++) {
        reg_users[reg_count] = malloc(sizeof(reg_user));
        if (fscanf(reg_user_list, "%s %s", reg_users[reg_count]->name, reg_users[reg_count]->password) != 2)
            break;
    }
    fclose(reg_user_list);

    // init users list
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        users[i] = malloc(sizeof(user));
        users[i]->socket_fd = 0;
        strcpy(users[i]->name, "");
    }
    for (int i = 0; i < MAX_CONNECTIONS; i++) { id_pool[i] = i; }
    const int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("error: socket()");
    }
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    struct sockaddr_in serverAddress = {AF_INET, PORT, inet_addr(IP)};
    if (bind(server_socket, (struct sockaddr *) &serverAddress, sizeof(serverAddress))) {
        perror("error: bind()");
    }
    listen(server_socket, 5);
    while (1) {
        struct sockaddr_in client_address;
        int len = sizeof(client_address);
        const int socket_fd = accept(server_socket, (struct sockaddr *) &client_address, (socklen_t *) &len);
        if (pool_head >= MAX_CONNECTIONS) {
            const char *msg = "\nchannel is full...\nconnection closed\n"; //NOLINT
            write(socket_fd, msg, strlen(msg));
            close(socket_fd);
            continue;
        }

        // read userinfo

        char user_info[(MAX_NAME_LEN + MAX_PASSWORD_LEN + 3) * sizeof(char)];
        char input_name[MAX_NAME_LEN];
        char input_password[MAX_PASSWORD_LEN];
        read(socket_fd, user_info, MAX_NAME_LEN + MAX_PASSWORD_LEN + 3);

        // refuse to connect if auth failed
        if (!check_userinfo(user_info, input_name, input_password)) {
            shutdown(socket_fd, SHUT_RDWR);
            continue;
        }

        if(!occupancy_test(input_name, input_password, socket_fd)) {
            continue;
        }

        const int curr_id = id_pool[pool_head]; // assign id
        pool_head++; // new connection created! pool--

        // new user log in
        users[curr_id]->id = curr_id;
        users[curr_id]->socket_fd = socket_fd;
        strcpy(users[curr_id]->name, input_name);
        users[curr_id]->verified = (strcmp(input_password, "") == 0);
        users[curr_id]->valid = 1;
        char msg[64];
        sprintf(msg, "\033[38;5;159m%s\033[0m has \033[38;5;40mjoined in\033[0m\n", users[curr_id]->name);
        broadcast(msg);
        // create new thread
        pthread_t thread;
        pthread_create(&thread, NULL, new_user_thread, users[curr_id]);
    }
    close(server_socket);
}


void *new_user_thread(void *arg) {
    //NOLINT
    printf("New user thread started\n");
    user *curr_user = arg;
    while (1) {
        /*
        if (curr_user->socket_fd < 0) {
            perror("error: accept()");
        }
        */
        char buffer[BUFFER_SIZE];
        if (curr_user->valid && read(curr_user->socket_fd, buffer, BUFFER_SIZE) == 0) {
            break;
        }
        if (strcmp(buffer, "") != 0) {
            if (buffer[0] == '/') {
                char **args = malloc(sizeof(char *) * 9);
                char *cmd = malloc(sizeof(char) * 16);
                int argc = split_command(buffer, cmd, args);
                command_handler(cmd, args, argc, curr_user);
                continue;
            }
            char msg[BUFFER_SIZE];
            char *time_str = malloc(40 * sizeof(char));
            get_localtime(time_str);
            snprintf(msg, BUFFER_SIZE + 128, "\033[36m[%s]\033[0m \033[32m%s\033[0m: %s\n", time_str, curr_user->name,
                     buffer);
            free(time_str);
            printf("%s", msg);
            broadcast(msg);
        }
    }
    logout_user(curr_user);
    char *time_str = malloc(40 * sizeof(char));
    get_localtime(time_str);
    char msg[BUFFER_SIZE];
    sprintf(msg, "\033[38;5;159m%s\033[0m \033[38;5;160mquit\033[0m  [%s]\n", curr_user->name, time_str); //NOLINT
    broadcast(msg);
    free(time_str);
    pthread_exit(NULL);
}

void logout_user(user *curr_user) {
    pool_head--; // connection exit: pool++
    id_pool[pool_head] = curr_user->id;
    users[curr_user->id]->valid = 0;
    printf("exit\n");
}

int check_userinfo(char *userinfo, char *name, char *password) {
    int p;
    for (p = 0; userinfo[p] != ' ' && userinfo[p] != 0; p++) {
    }
    strncpy(name, userinfo, p);
    strncpy(password, userinfo + p + 1, strlen(userinfo) - p - 1);
    name[p] = '\0';
    password[strlen(userinfo) - p - 1] = '\0';
    if (strcmp(password, "") == 0) return 1;
    for (int i = 0; i < reg_count; i++) {
        if (strcmp(name, reg_users[i]->name) == 0) {
            if (strcmp(reg_users[i]->password, password) == 0) return 1;
        }
    }
    return 0;
}

void broadcast(const char *msg) {
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        if (users[i]->valid && users[i]->socket_fd > 0) {
            write(users[i]->socket_fd, msg, BUFFER_SIZE);
        }
    }
}

void send_to(const char *msg, user *curr_user) {
    if (curr_user->socket_fd > 0) {
        write(curr_user->socket_fd, msg, BUFFER_SIZE);
    }
}

int occupancy_test(char* name, char* password, int socket_fd) {
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        if (users[i]->valid && strcmp(name, users[i]->name) == 0) {
            if (strcmp(password, "") == 0) {
                char msg[40] = "name occupied\n";
                send(socket_fd, msg, strlen(msg), 0);
                shutdown(socket_fd, SHUT_RDWR);
                return 0;
            }
            // if verified
            char msg[40] = "name occupied\n";
            send(users[i]->socket_fd, msg, strlen(msg), 0);
            shutdown(users[i]->socket_fd, SHUT_RDWR);
            close(users[i]->socket_fd);
            return 1;
        }
    }
    return 1;
}

void command_handler(char *cmd, char **args, int argc, user *curr_user) //NOLINT
{
    if (strcmp(cmd, "register") == 0 && argc == 2) {
        reg_users[reg_count] = malloc(sizeof(reg_user));
        strcpy(reg_users[reg_count]->name, args[0]); //NOLINT
        strcpy(reg_users[reg_count]->password, args[1]);
        FILE *reg_user_list = fopen("users", "w+");
        for (int i = 0; i <= reg_count; i++) {
            fprintf(reg_user_list, "%s %s\n", reg_users[i]->name, reg_users[i]->password);
        }
        fclose(reg_user_list);
        reg_count++;
    }
    if (strcmp(cmd, "ls") == 0 && argc == 0) {
        char buffer[BUFFER_SIZE] = "";
        for (int i = 0; i < pool_head; i++) {
            int len = (int) strlen(users[i]->name);
            if (len > 0) {
                strcat(buffer, users[i]->name);
                strcat(buffer, "\n");
            }
        }
        write(curr_user->socket_fd, buffer, BUFFER_SIZE);
    }
    if (strcmp(cmd, "private") == 0 && argc == 2) {
        unwrap_msg(args[1]);
        char time_str[40];
        get_localtime(time_str);
        char msg[BUFFER_SIZE];
        sprintf(msg, "\033[36m%s\033[0m:%s  \033[38;5;114m[%s] \033[38;5;222m[private]\033[0m\n", curr_user->name,
                args[1], time_str);
        for (int i = 0; i < pool_head; i++) {
            if (strcmp(users[i]->name, args[0]) == 0) {
                send_to(msg, users[i]);
            }
        }
    }
}
