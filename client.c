#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "macros.h"
#include "utils.h"

int client_socket;


void *receive_msg(void *arg);
void get_localtime(char *buffer);

int split_command(const char *src, char *cmd, char **args);
void command_handler(char *cmd,char **args,int argc,char *input);
int input_mask=0;
void* get_input(void* input);

void show_progress_bar();

int main()
{
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0)
    {
        perror("error: socket()");
        exit(EXIT_FAILURE);
    }

    const struct sockaddr_in server_address = {AF_INET, PORT, inet_addr("127.0.0.1")};
    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(struct sockaddr)))
    {
        perror("error: connect()");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    //show_progress_bar();

    pthread_t thread;
    pthread_create(&thread, NULL, receive_msg, (void *)(intptr_t)client_socket);

    char name[30];
    printf("nickname: ");
    fgets(name,15,stdin);
    name[strcspn(name,"\n")]='\0';
    write(client_socket, name, sizeof(name));

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
        write(client_socket, strcat(input, "\0"), BUFFER_SIZE);
    }

    close(client_socket);
    return 0;
}

void *receive_msg(void *arg)
{
    int client_socket = (int)(intptr_t)arg;
    while (1)
    {
        char msg[BUFFER_SIZE];
        if (read(client_socket, msg, BUFFER_SIZE) == 0)
        {
            break;
        }
        printf("%s", msg);
        fflush(stdout);
    }
    pthread_exit(NULL);
}

void command_handler(char *cmd,char **args,int argc,char *input)
{
    if (strcmp(cmd, "exit")==0 && argc == 0)
    {
        close(client_socket);
        exit(0);
    }
    if (strcmp(cmd, "time")==0 && argc == 0)
    {
        char local_time[40];
        get_localtime(local_time);
        printf("%s", local_time);
    }
    if (strcmp(cmd, "help")==0 && argc == 0)
    {
        printf("11111");
    }
    if(strcmp(cmd,"get")==0 && argc ==1)
    {
        write(client_socket,input,BUFFER_SIZE);
    }
    if(strcmp(cmd, "register") == 0 && argc == 2) {
        write(client_socket,input,BUFFER_SIZE);
    }
    if(strcmp(cmd, "list_users") == 0 && argc == 0) {
        write(client_socket,input,BUFFER_SIZE);
    }
    if(strcmp(cmd, "private") == 0 && argc == 1) {
        char *msg = malloc(sizeof(char) * 100);
        pthread_t thread;
        pthread_create(&thread, NULL, get_input, msg);
        pthread_join(thread, NULL);
        input[strcspn(input,"\n")]=' ';
        wrap_msg(msg);
        strcat(input, msg);
        write(client_socket, input, BUFFER_SIZE);
        free(msg);
    }
}

void *get_input(void* arg) {
    char *msg = arg;
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