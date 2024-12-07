#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#define PORT 8000
#define MAXCLIENTS 5
#define TRUE 1
#define FALSE 0

struct Client {
    char username[100];
    int sock;
};

int running = TRUE, server_sock, activity, max_fd, num_clients = 0;
struct Client clients[MAXCLIENTS] = { 0 };

struct sockaddr_in address;
char buffer[1024] = { 0 };
socklen_t addr_size = sizeof(address);
fd_set readfds;

void close_server() {
    printf("\nkilling server");
    running = FALSE;
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
    signal(SIGINT, close_server);

    if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    else {
        printf("socket created\n");
        fflush(stdout);
    }

    max_fd = server_sock;

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_sock, (struct sockaddr *) &address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    else {
        printf("socket binded\n");
        fflush(stdout);
    }

    if (listen(server_sock, MAXCLIENTS) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }
    else {
        printf("listening for connections...\n");
        fflush(stdout);
    }
    while (running) {
        FD_ZERO(&readfds);
        FD_SET(server_sock, &readfds);

        for (int i = 0; i < num_clients; i++) {
            FD_SET(clients[i].sock, &readfds);
        }

        max_fd = server_sock;

        for (int i = 0; i < num_clients; i++) {
            if (clients[i].sock > max_fd) {
                max_fd = clients[i].sock;
            }
        }

        if ((activity = select(max_fd + 1, &readfds, NULL, NULL, NULL)) < 0) {
            perror("select failed\n");
            exit(EXIT_FAILURE);
        }

        if (FD_ISSET(server_sock, &readfds)) {
            struct Client new_client;
            if ((new_client.sock = accept(server_sock, (struct sockaddr * ) &address, &addr_size)) < 0) {
                perror("accept failed\n");
                exit(EXIT_FAILURE);
            }
            else {
                int valread = read(new_client.sock, new_client.username, sizeof(new_client.username) - 1);
                if (valread == 0) {
                    perror("username read failed");
                    exit(EXIT_FAILURE);
                }
                new_client.username[valread] = '\0';
                printf("connection from %s accepted\n", new_client.username);
                fflush(stdout);
            }
            FD_SET(new_client.sock, &readfds);

            if (new_client.sock > max_fd) {
                max_fd = new_client.sock;
            }

            clients[num_clients] = new_client;
            num_clients++;
        }

        for (int i = 0; i < num_clients; i++) {
            struct Client client = clients[i];
            if (FD_ISSET(client.sock, &readfds)) {
                int valread = read(client.sock, buffer, sizeof(buffer) - 1);
                if (valread == 0) {
                    close(client.sock);

                    FD_CLR(client.sock, &readfds);

                    for (int j = i; j < num_clients - 1; j++) {
                        clients[j] = clients[j + 1];
                    }

                    num_clients--;
                    printf("client '%s' disconnected\n", client.username);
                    fflush(stdout);
                }
                else if (valread > 1) {
                    buffer[valread] = '\0';
                    printf("[%s] : %s", client.username, buffer);
                    fflush(stdout);
                }
                memset(buffer, 0, sizeof(buffer));
            }
        }
    }

    for (int i = 0; i < num_clients; i++) {
        send(clients[i].sock, "shutdown", strlen("hello"), 0);
        close(clients[i].sock);
    }

    close(server_sock);
    
    return 0;
}