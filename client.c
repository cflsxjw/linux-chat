#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "macros.h"
#include "utils.h"

int client_socket;
char name[MAX_NAME_LEN];

void *get_input(void *);

void command_handler(char *cmd,char **args,int argc,char *input);

void* get_message(void* input);

void* check_connection(void*);

void show_progress_bar();


int main()
{
    printf("  _      _                          _           _   \n");
    printf(" | |    (_)                        | |         | |  \n");
    printf(" | |     _ _ __  _   ___  __    ___| |__   __ _| |_ \n");
    printf(" | |    | | '_ \\| | | \\ \\/ /   / __| '_ \\ / _` | __|\n");
    printf(" | |____| | | | | |_| |>  <   | (__| | | | (_| | |_ \n");
    printf(" |______|_|_| |_|\\__,_/_/\\_\\   \\___|_| |_|\\__,_|\\__|\n\n");
    printf("nickname: ");
    fgets(name,MAX_NAME_LEN,stdin);
    name[strcspn(name,"\n")]='\0';
    char *password = malloc(MAX_PASSWORD_LEN * sizeof(char));
    disable_echo();
    printf("password: ");
    fgets(password,MAX_NAME_LEN,stdin);
    password[strcspn(password,"\n")]='\0';
    enable_echo();
    if (strcmp(name,"") == 0) {
        printf("Please enter a username");
        exit(0);
    }

    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0)
    {
        perror("error: socket()");
        exit(EXIT_FAILURE);
    }


    const struct sockaddr_in server_address = {AF_INET, PORT, inet_addr(IP)};
    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(struct sockaddr)))
    {
        perror("error: connect()");
        close(client_socket);
        exit(EXIT_FAILURE);
    }
    //show_progress_bar();

    pthread_t thread;
    pthread_create(&thread, NULL, get_input, (void *)(intptr_t)client_socket);
    char *user_info = malloc((MAX_NAME_LEN + MAX_PASSWORD_LEN + 3) * sizeof(char));
    sprintf(user_info, "%s %s", name, password);
    write(client_socket, user_info, MAX_NAME_LEN + MAX_PASSWORD_LEN + 3 * sizeof(char));
    printf("\n");
    free(password);
    while (1)
    {
        char msg[BUFFER_SIZE];
        if (recv(client_socket, msg, BUFFER_SIZE, 0) == 0)
        {
            printf("oops!\n");
            pthread_cancel(thread);
            getchar();
            exit(0);
        }
        printf("%s", msg);
        fflush(stdout);
    }

    close(client_socket);
    return 0;
}



void *get_input(void *)
{
    // 获取输入
    while (1)
    {
        char input[BUFFER_SIZE];
        fgets(input, BUFFER_SIZE, stdin);
        input[strcspn(input,"\n")]='\0';
        if (input[0] == '/')
        {
            char **args = malloc(sizeof(char *) * MAX_ARG_COUNT);
            char *cmd = malloc(sizeof(char) * MAX_COMMAND_LEN);
            int argc = split_command(input, cmd, args);
            command_handler(cmd,args,argc,input);
            continue;
        }
        send(client_socket, strcat(input, "\0"), BUFFER_SIZE, 0);
    }
    pthread_exit(NULL);
}

void command_handler(char *cmd,char **args,int argc,char *input)
{
    if (strcmp(cmd, "help")==0 && argc == 0)
    {
        printf("    * exit: 退出程序\n");
        printf("    * time: 打印当前时间\n");
        printf("    * ls: 列出所有在线用户\n");
        printf("    * private: 私信用户\n");
    }
    if (strcmp(cmd, "exit")==0 && argc == 0)
    {
        close(client_socket);
        exit(0);
    }
    if (strcmp(cmd, "time")==0 && argc == 0)
    {
        char local_time[40];
        get_localtime(local_time);
        printf("%s\n", local_time);
    }
    if(strcmp(cmd, "register") == 0 && argc == 2) {
        write(client_socket,input,BUFFER_SIZE);
    }
    if(strcmp(cmd, "ls") == 0 && argc == 0) {
        write(client_socket,input,BUFFER_SIZE);
    }
    if(strcmp(cmd, "private") == 0 && argc == 1) {
        char *msg = malloc(sizeof(char) * 100);
        pthread_t thread;
        pthread_create(&thread, NULL, get_message, msg);
        pthread_join(thread, NULL);
        input[strcspn(input,"\n")]=' ';
        wrap_msg(msg);
        strcat(input, msg);
        write(client_socket, input, BUFFER_SIZE);
        free(msg);
    }
}

void *get_message(void* input) {
    char *msg = input;
    printf("input massage: ");
    fgets(msg, 100, stdin);
    msg[strcspn(msg,"\n")]=0;
    pthread_exit(NULL);
}

void show_progress_bar()
{
    for (int i = 0; i <= PROGRESS_BAR_LENGTH; i++)
    {
        float percent = (float)i / PROGRESS_BAR_LENGTH * 100;
        printf("\rProgress: [");
        for (int j = 0; j < PROGRESS_BAR_LENGTH; j++)
        {
            if (j < i)
            {
                printf(COLOR_YELLOW "▮" COLOR_RESET);
            }
            else
            {
                printf(" ");
            }
        }
        printf("] %.2f%%", percent);
        fflush(stdout);
        usleep(20000); // 20 ms delay for a total of 1 second
    }
    printf("\n");
}