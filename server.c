#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define PORT 4269
volatile static int user_count = 0;

int create_server() 
{
    int fd;

    struct sockaddr_in server = {
        .sin_family = AF_INET, // IPv4
        .sin_addr.s_addr = htonl(INADDR_ANY),
        .sin_port = htons(PORT)
    };
    
    if ((fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) 
    {
        perror("Couldn't init socket");
        exit(1);        
    }

    if (bind(fd, (struct sockaddr*)&server, sizeof(server)) == -1)
    {
        perror("Couldn't bind server");
        exit(1);
    }

    if(listen(fd, 10) == -1) 
    {
        perror("Couldn't start listening to servers");
        exit(1);
    }

    return fd;
}


void* handle_client(void* fd) 
{
    char* message = malloc(sizeof(char) * 1024);
    for (;;) 
    {
        if (recv(*(int*)fd, (void*)message, sizeof(message), 0) == -1) 
        {
            perror("Couldn't receive message from client");
            exit(1);
        }
        if (send(*(int*)fd, (void*)message, strlen(message), 0) == -1) 
        {
            perror("Couldn't send message back");
            exit(1);
        } 
    }
    free(message);
    pthread_exit(NULL);
}

int main(int argc, char** argv) 
{
    int server_fd = create_server();
    char welcome_message[] = "Hello there stranger!";
    pthread_t client_handles[10];
    struct sockaddr_in* client = malloc(sizeof(struct sockaddr_in));
    for (;;) 
    {
        puts("[Waiting for incoming connections....]");
        int client_sock = accept(server_fd, (struct sockaddr*)&client, (socklen_t*)sizeof(struct sockaddr_in));
        if (client_sock == -1) 
        {
            perror("Couldn't establish a connection with the client");
            exit(1);
        }

        puts("[A new client has joined the server!]");
        if (send(client_sock, (void*)welcome_message, strlen(welcome_message), 0) == -1)
        {
            perror("Couldn't send welcome message");
            exit(1);
        }
        pthread_create(&client_handles[user_count], NULL, handle_client, (void*)&client_sock);
        user_count++;
    }

    close(server_fd);
}