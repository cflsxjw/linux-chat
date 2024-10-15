#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>
#include "c_command.h"
#include "macros.h"

int client_socket;


void *receive_msg(void *arg);
void get_localtime(char *buffer);

int split_command(const char *src, char *cmd, char **args);
void command_handler(char *cmd,char **args,int argc,char *input);

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
                printf(COLOR_YELLOW "▮" COLOR_RESET); // Print yellow block
            }
            else
            {
                printf(" "); // Empty space
            }
        }
        printf("] %.2f%%", percent);
        fflush(stdout);
        usleep(20000); // 20 ms delay for a total of 1 second
    }
    printf("\n");
}

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

    show_progress_bar();

    pthread_t thread;
    pthread_create(&thread, NULL, receive_msg, (void *)(intptr_t)client_socket);

    char name[30];
    printf("nickname: ");
    fgets(name,15,stdin);
    name[strcspn(name,"\n")]='\0';
    write(client_socket, name, sizeof(name));

    // 显示进度条前清空行
    //printf("\r\n"); // 清空当前行并换行

    // 获取输入
    while (1)
    {
        char input[BUFFER_SIZE];
        fgets(input, BUFFER_SIZE, stdin);
        input[strcspn(input,"\n")]='\0';
        if (input[0] == '/')
        {
            char **args = malloc(sizeof(char *) * 9);
            char *cmd = malloc(sizeof(char) * 16);
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
        cmd_exit();
    }
    if (strcmp(cmd, "time")==0 && argc == 0)
    {
        cmd_time();
    }
    if (strcmp(cmd, "help")==0 && argc == 0)
    {
        printf("11111");
    }
    if(strcmp(cmd,"get")==0 && argc ==1)
    {
        cmd_get(input);
    }
}