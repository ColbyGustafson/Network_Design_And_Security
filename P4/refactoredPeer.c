#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/select.h>
#include "linkedlist.h"
#include "blockchain.h"

#define MAX_CLIENTS FD_SETSIZE
#define START_PORT 27500
#define END_PORT 27510
#define SERVER_PORT 27500
#define BUFFER_SIZE 2000

typedef struct {
    char* port;
} Client;

// Function Declarations
void initializeServer(int *server_fd, struct sockaddr_in *address);
void handleConnection(int server_fd);
void *receiveMessages(void *server_fd);
void handleClientMessages(int server_fd);
void sendBlock();
void addClient(Client clients[], const char *client_port);
void removeClient(Client clients[], const char *client_port);
void processClientMessage(char *message, Client clients[], LinkedList *blockchain);
void updateClients(Client clients[], LinkedList *blockchain);

// Main function
int main(int argc, char const *argv[]) {
    int server_fd;
    struct sockaddr_in address;
    initializeServer(&server_fd, &address);

    pthread_t receiver_thread;
    pthread_create(&receiver_thread, NULL, receiveMessages, &server_fd);

    handleConnection(server_fd);
    pthread_join(receiver_thread, NULL);

    close(server_fd);
    return 0;
}

void initializeServer(int *server_fd, struct sockaddr_in *address) {
    *server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (*server_fd == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    address->sin_family = AF_INET;
    address->sin_addr.s_addr = INADDR_ANY;

    // Try binding to ports within the range [START_PORT, END_PORT)
    for (int port = START_PORT; port < END_PORT; port++) {
        address->sin_port = htons(port);
        if (bind(*server_fd, (struct sockaddr *)address, sizeof(*address)) >= 0) {
            printf("Bound to port %d\n", port);
            return;
        }
    }

    // If no port is available
    fprintf(stderr, "Failed to bind to any port in the range %d - %d\n", START_PORT, END_PORT);
    exit(EXIT_FAILURE);
}


void handleConnection(int server_fd) {
    int userInput;
    printf("Enter '1' to send a block, '0' to exit: ");
    while (scanf("%d", &userInput) && userInput != 0) {
        if (userInput == 1) {
            sendBlock();
        } else {
            printf("Invalid input. Enter '1' to send a block, '0' to exit: ");
        }
    }
    printf("Exiting...\n");
}


void *receiveMessages(void *server_fd_ptr) {
    int server_fd = *((int *)server_fd_ptr);
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];
    int client_fd;

    while (1) {
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
        if (client_fd < 0) {
            perror("Accept failed");
            continue;
        }

        memset(buffer, 0, BUFFER_SIZE);
        int read_bytes = recv(client_fd, buffer, BUFFER_SIZE, 0);
        if (read_bytes > 0) {
            printf("Received message: %s\n", buffer);
            // Process the received message here
        }
        close(client_fd);
    }
    return NULL;
}


void handleClientMessages(int server_fd) {
    char buffer[BUFFER_SIZE];
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int client_fd;

    client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
    if (client_fd < 0) {
        perror("Failed to accept client connection");
        return;
    }

    memset(buffer, 0, BUFFER_SIZE);
    int read_bytes = recv(client_fd, buffer, BUFFER_SIZE, 0);
    if (read_bytes <= 0) {
        close(client_fd);
        return;
    }

    printf("Received message from client: %s\n", buffer);

    // Process the message here

    close(client_fd);
}


void sendBlock() {
    int sockfd;
    struct sockaddr_in serv_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERVER_PORT);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Failed to connect to the server");
        exit(EXIT_FAILURE);
    }

    // Here, you should prepare the block data to be sent
    const char* block_data = "Your block data here";

    send(sockfd, block_data, strlen(block_data), 0);
    printf("Block sent to server\n");

    close(sockfd);
}


void addClient(Client clients[], const char *client_port) {
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i].port == NULL) {
            clients[i].port = strdup(client_port);
            printf("Added new client on port: %s\n", client_port);
            return;
        }
    }
    printf("Failed to add client: Client array is full\n");
}


void removeClient(Client clients[], const char *client_port) {
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i].port != NULL && strcmp(clients[i].port, client_port) == 0) {
            free(clients[i].port);
            clients[i].port = NULL;
            printf("Client on port %s removed.\n", client_port);
            return;
        }
    }
    printf("Client on port %s not found.\n", client_port);
}


void processClientMessage(char *message, Client clients[], LinkedList *blockchain) {
    // Example processing logic
    if (strcmp(message, "request_blockchain") == 0) {
        // Client requests the entire blockchain
        // Send the blockchain to the client
    } else if (strncmp(message, "add_block:", 10) == 0) {
        // Client sends a new block to be added
        char *new_block_data = message + 10;  // Extract the block data from the message
        // Add the new block to the blockchain
        // Update all clients with the new blockchain
    } else {
        printf("Unknown message received: %s\n", message);
    }
}


void updateClients(Client clients[], LinkedList *blockchain) {
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i].port != NULL) {
            int sockfd;
            struct sockaddr_in serv_addr;

            sockfd = socket(AF_INET, SOCK_STREAM, 0);
            if (sockfd < 0) {
                perror("Socket creation failed");
                continue;
            }

            memset(&serv_addr, 0, sizeof(serv_addr));
            serv_addr.sin_family = AF_INET;
            serv_addr.sin_port = htons(atoi(clients[i].port));
            serv_addr.sin_addr.s_addr = INADDR_ANY;

            if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
                perror("Failed to connect to the client");
                close(sockfd);
                continue;
            }

            // Convert the entire blockchain to a string or a serialized format
            char *blockchain_data = "Serialized blockchain data here";
            send(sockfd, blockchain_data, strlen(blockchain_data), 0);
            printf("Blockchain updated to client on port %s\n", clients[i].port);

            close(sockfd);
        }
    }
}
