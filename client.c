#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#define PORT 8000
#define TRUE 1
#define FALSE 0
#define STRING_LIMIT 1024

int client_sock;
int sent_bytes;
char* username;
struct sockaddr_in server_addr;
char buffer[STRING_LIMIT] = { 0 };
socklen_t addr_size = sizeof(server_addr);

void close_client() {
    close(client_sock);
    printf("\nkilling client");
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: ./client <username>");
        exit(EXIT_FAILURE);
    }

    username = argv[1];

    signal(SIGINT, close_client);
    if ((client_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");    
        exit(1);
    }
    else {
        printf("socket created\n");
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "192.168.1.241", &server_addr.sin_addr) < 0) {
        perror("address conversion failed");
        exit(1);
    }
    else {
        printf("address name converted\n");
    }

    if (connect(client_sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        perror("connect failed");
        exit(1);
    }
    else {
        printf("connection to server established\n");
    }
    
    if ((send(client_sock, username, sizeof(username), 0)) < 0) {
        perror("send failed");
        exit(1);
    }


    while (TRUE) {
        printf("[%s] : ", username);
        fflush(stdout);
        fgets(buffer, STRING_LIMIT, stdin);
        if ((sent_bytes = send(client_sock, buffer, sizeof(buffer), 0)) < 1) {
            perror("send failed");
            close_client();
        }
    }
    
    return 0;
}