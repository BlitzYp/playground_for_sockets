#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>

#define PORT 4269
#define HOME "127.0.0.1"

pthread_mutex_t lock_for_sending = PTHREAD_MUTEX_INITIALIZER;

typedef struct fmt 
{
    char* data;
    short len;
    int fd;
    char* store_into;
}fmt;

int create_client() 
{
    int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fd == -1) 
    {
        perror("Couldn't create socket");
        exit(1);
    }
    struct sockaddr_in client = {.sin_family = AF_INET, .sin_addr.s_addr = inet_addr(HOME), .sin_port = htons(PORT)};
    if (connect(fd, (struct sockaddr*)&client, sizeof(client)) == -1) 
    {
        perror("Could not connect to the server");
        exit(1);
    }
    return fd;
}


void* client_handle(void* args) 
{
    fmt* format = (fmt*)args;
    pthread_mutex_lock(&lock_for_sending);
    if (send(format->fd, (void*)format->data, format->len, 0) == -1) 
    {
        perror("Could not send data to server");
        exit(1);
    }
    pthread_mutex_unlock(&lock_for_sending);
    
    if (recv(format->fd, (void*)format->store_into, format->len, 0) == -1) 
    {
        perror("Couldn't receive data from server");
        exit(1);
    }

    puts(format->store_into);
}

int main(int argc, char** argv) 
{
    int client_fd = create_client();
    pthread_t client;
    char* input = malloc(sizeof(char) * 255);
    char* res = malloc(sizeof(char) * 255);
    for (;;) 
    {   
        fscanf(stdin, "%s", input);
        fmt input_data = {.data = input, .fd = client_fd, .len = strlen(input) + 1, .store_into = res};
        pthread_create(&client, NULL, client_handle, (void*)&input_data);
    }
    return 0;
}