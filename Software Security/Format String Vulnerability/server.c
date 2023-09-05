#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdint.h>  // Add this line to include <stdint.h>

#define PORT 9090

#ifndef DUMMY_SIZE
#define DUMMY_SIZE 100
#endif

char *secret = "A secret message\n";
unsigned int target = 0x11223344;

void myprintf(const char *msg)
{
    uintptr_t framep;
    asm("movq %%rbp, %0" : "=r"(framep));
    printf("The rbp value inside myprintf() is: 0x%.16lx\n", (unsigned long)framep);

    char dummy[DUMMY_SIZE];
    memset(dummy, 0, DUMMY_SIZE);

    printf("%s", msg); // Use %s to print the message safely
    printf("The value of the 'target' variable (after): 0x%.8x\n", target);
}

void helper()
{
    printf("The address of the secret: %p\n", (void *)secret);
    printf("The address of the 'target' variable: %p\n", (void *)&target);
    printf("The value of the 'target' variable (before): 0x%.8x\n", target);
}

int main()
{
    struct sockaddr_in server;
    struct sockaddr_in client;
    socklen_t clientLen;

    char buf[1500];

    char dummy[DUMMY_SIZE];
    memset(dummy, 0, DUMMY_SIZE);

    printf("The address of the input array: %p\n", (void *)buf);

    helper();

    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0)
    {
        perror("ERROR opening socket");
        return 1;
    }

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(PORT);

    if (bind(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("ERROR on binding");
        close(sock);
        return 1;
    }

    while (1)
    {
        bzero(buf, sizeof(buf));
        recvfrom(sock, buf, sizeof(buf) - 1, 0, (struct sockaddr *)&client, &clientLen);
        myprintf(buf);
    }

    close(sock);
    return 0;
}
