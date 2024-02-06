// CLASS: 4300 - Perouli
// PROJECT: 4
// STUDENTS: Colby Gustafson & Timi Ladeinde

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

//charInit
char name[20];

//intInit
int PORT;
int numYesVotes;
int numClients;

//voidInit
void *receive_thread(void *fileDesc);
void receiveMsg(int fileDesc);
void sendBlock();

//LLInit
LinkedList* blockChain;

//client def
typedef struct 
{
    char* clientPort;
} 

ClientInfo;
ClientInfo clients[FD_SETSIZE];

int main(int argc, char const *argv[])
{
    numClients = 0;
    srand(time(NULL));
    blockChain = createLinkedList();

    int fileDesc;
    struct sockaddr_in addr;

    if ((fileDesc = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Error: Couldn't use socket");
        exit(EXIT_FAILURE);
    }
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;

    for(PORT = START_PORT; PORT< END_PORT; PORT++){
        addr.sin_port = htons(PORT);
        if (bind(fileDesc, (struct sockaddr *)&addr, sizeof(addr)) >= 0)
        {
            break;
        }
    }
    if(PORT >= END_PORT){
        printf("Exiting: No open port\n");
        exit(EXIT_FAILURE);
    }
    if(PORT == START_PORT){
        char* blockOne = simpleHash("0");
        printf("Init hash: %s\n", blockOne);
        insertBlock(blockChain, blockOne);
        printLinkedList(blockChain);
        printf("First connection made. You have been designated as the leader.\n");
    }
    printf("Your port number is: %d\n", (int)ntohs(addr.sin_port));

    if (listen(fileDesc, 5) < 0)
    {
        perror("Error");
        exit(EXIT_FAILURE);
    }
    int modal;
    pthread_t tid;
    pthread_create(&tid, NULL, &receive_thread, &fileDesc);

    if(PORT != START_PORT)
    {
    char buffer[BUFFER_SIZE] = {0};
    int serverPort = SERVER_PORT;
    int sock = 0, readVal;
    struct sockaddr_in serv_addr;
    char twoPowerTen[1024] = {0};
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\nError: Socket creation faulty\n");
        return 0;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(serverPort);

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nNo connection made\n");
        return 0;
    }

    sprintf(buffer, "connection @ %d", PORT);
    send(sock, buffer, sizeof(buffer), 0);
    close(sock);

    printf("\n --Press \"0\" at any time to exit\n --Press \"1\" at any time to send the current block\n");

    do
    {
        scanf("%d", &modal);

        switch (modal)
        {
        case 1:
            printf("\nSending\n");
            sendBlock();
            break;
        case 0:

            int sock = 0, readVal;
            struct sockaddr_in serv_addr;

            if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
            {
                printf("\nError: Socket creation faulty\n");
                return 0;
            }

            serv_addr.sin_family = AF_INET;
            serv_addr.sin_addr.s_addr = INADDR_ANY;
            serv_addr.sin_port = htons(serverPort);
            
            if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
            {
                printf("\nFailed to Connect\n");
                return 0;
            }
            
            sprintf(buffer, "disconnection @ %d", PORT);
            send(sock, buffer, sizeof(buffer), 0);
            close(sock);
            
            printf("\nExiting...\n");

            break;
        default:
            printf("\nError: Invalid Option\n");
        }
    } while (modal);

    close(fileDesc);

    return 0;
    }

    else{
        do{
        printf("\n --Press \"0\" at any time to exit\n");
            
        scanf("%d", &modal);
        switch (modal)
        {
        case 0:
            printf("\nExiting...\n");
            break;
        default:
            printf("\nError: Invalid Option\n");
        }

        }while(modal);
        close(fileDesc);
        return 0;
    }
}

void sendBlock()
{
    int serverPort = SERVER_PORT;
    char buffer[BUFFER_SIZE] = {0};

    int sock = 0, readVal;
    struct sockaddr_in serv_addr;
    char twoPowerTen[1024] = {0};
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(serverPort);

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return;
    }
    
    char* new_blocksum;
    char str[50];
    
    strcpy(new_blocksum, simpleHash(blockChain->head->data));
    sprintf(str, "%d", rand());
    strcat(new_blocksum, str);

    sprintf(buffer, "added @ %d with block %s", PORT, new_blocksum);
    send(sock, buffer, sizeof(buffer), 0);
    printf("\nSend was successful\n");
    close(sock);
}

void *receive_thread(void *fileDesc)
{
    int s_fd = *((int *)fileDesc);
    while (1)
    {
        sleep(2);
        receiveMsg(s_fd);
    }
}

void receiveMsg(int fileDesc)
{
    struct sockaddr_in addr;
    int readVal;
    char buffer[BUFFER_SIZE] = {0};
    int addrlen = sizeof(addr);
    fd_set current_sockets, ready_sockets;

    FD_ZERO(&current_sockets);
    FD_SET(fileDesc, &current_sockets);
    int k = 0;
    while (1)
    {
        k++;
        ready_sockets = current_sockets;

        if (select(FD_SETSIZE, &ready_sockets, NULL, NULL, NULL) < 0)
        {
            perror("Error");
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < FD_SETSIZE; i++)
        {
            if (FD_ISSET(i, &ready_sockets))
            {

                if (i == fileDesc)
                {
                    int client_socket;

                    if ((client_socket = accept(fileDesc, (struct sockaddr *)&addr,
                                                (socklen_t *)&addrlen)) < 0)
                    {
                        perror("accept");
                        exit(EXIT_FAILURE);
                    }
                    FD_SET(client_socket, &current_sockets);
                }
                else
                {
                    readVal = recv(i, buffer, sizeof(buffer), 0);
                    printf("\n%s\n", buffer);
                    
                    char * array[3];
                    char * p = strtok(buffer,"|");
                    int u = 0;
                    while(p != NULL){
                        array[u++] = p;
                        p = strtok(NULL, "|");
                    }
                    
                    if(strcmp(array[0], "connect") == 0){
                        u=0;
                        while(clients[u].clientPort != NULL)
                            u++;
                        clients[u].clientPort = (char*)malloc(sizeof(char)*strlen(array[1]));
                        strcpy(clients[u].clientPort, array[1]);

                        int sock = 0, readVal;
                        struct sockaddr_in serv_addr;
                        char twoPowerTen[1024] = {0};
                        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
                        {
                            printf("\n Socket creation error \n");
                            return;
                        }

                        serv_addr.sin_family = AF_INET;
                        serv_addr.sin_addr.s_addr = INADDR_ANY; 
                        serv_addr.sin_port = htons(atoi(clients[u].clientPort));
                        
                        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
                        {
                            perror("\nConnection Failed \n");
                            return;
                        }

                        Node* head = blockChain->head;

                        char* current = malloc(sizeof(char)*strlen(blockChain->head->data)*100);
                        strcpy(current, blockChain->head->data);
                        
                        while(blockChain->head->next != NULL){
                            blockChain->head = blockChain->head->next;
                            strcat(current, ",");
                            strcat(current, blockChain->head->data);
                        }
                        strcat(current, ",");
                        blockChain->head = head;

                        sprintf(buffer, "start|%s", current);
                        send(sock, buffer, sizeof(buffer), 0);
                        close(sock);
                        free(current);
                        numClients++;
                    }
                    
                    if(strcmp(array[0], "start") == 0 && PORT != START_PORT){
                        destroyLinkedList(blockChain);
                        u=0;
                        char* current = (char*)malloc(sizeof(char)*strlen(array[1]));
                        strcpy(current, array[1]);
                        char* array2[6000];
                        
                        char* p = strtok(current, ",");
                        while(p != NULL){
                            array2[u++] = p;
                            p = strtok(NULL, ",");
                        }
                        while(u!=0){
                            insertBlock(blockChain, array2[u-1]);
                            u--;
                        }
                    }

                    if(strcmp(array[0], "add") == 0 && PORT == START_PORT){
                        numYesVotes = 1;
                        u=0;
                        char* ContestedBlock = (char*)malloc(sizeof(char)*strlen(array[2]));
                        strcpy(ContestedBlock, array[2]);
                        
                        if(numClients != 1)
                            while(clients[u].clientPort != NULL){
                                if(strcmp(clients[u].clientPort, array[1]) != 0){
                                    int sock = 0, readVal;
                                    struct sockaddr_in serv_addr;
                                    char twoPowerTen[1024] = {0};
                                    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
                                    {
                                        printf("\n Socket creation error \n");
                                        return;
                                    }

                                    serv_addr.sin_family = AF_INET;
                                    serv_addr.sin_addr.s_addr = INADDR_ANY;
                                    serv_addr.sin_port = htons(atoi(clients[u].clientPort));

                                    
                                    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
                                    {
                                        perror("\nConnection Failed \n");
                                        return;
                                    }
                                    
                                    sprintf(buffer, "vote|%s", ContestedBlock);
                                    send(sock, buffer, sizeof(buffer), 0);
                                    close(sock);
                                }
                                u++;
                            }

                        if(numClients == 1){
                            insertBlock(blockChain, ContestedBlock);
                            free(ContestedBlock);
                            u=0;
                            int sock = 0, readVal;
                            struct sockaddr_in serv_addr;
                            char twoPowerTen[1024] = {0};
                            if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
                            {
                                printf("\n Socket creation error \n");
                                return;
                            }
                            
                            serv_addr.sin_family = AF_INET;
                            serv_addr.sin_addr.s_addr = INADDR_ANY;
                            serv_addr.sin_port = htons(atoi(clients[u].clientPort));

                      
                            
                            if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
                            {
                                perror("\nConnection Failed \n");
                                return;
                            }

                            Node* head = blockChain->head;

                            char* current = malloc(sizeof(char)*strlen(blockChain->head->data)*100);
                            strcpy(current, blockChain->head->data);
                            
                            while(blockChain->head->next != NULL){
                                blockChain->head = blockChain->head->next;
                                strcat(current, ",");
                                strcat(current, blockChain->head->data);
                            }
                            strcat(current, ",");
                            blockChain->head = head;

                            sprintf(buffer, "start|%s", current);
                            send(sock, buffer, sizeof(buffer), 0);
                            close(sock);
                            free(current);
                            numYesVotes = 0;

                        }
                    }

                    if(strcmp(array[0], "vote") == 0){
                        char* ContestedBlock = (char*)malloc(sizeof(char)*strlen(array[1]));
                        strcpy(ContestedBlock, array[1]);
                        char* hash = simpleHash(blockChain->head->data);

                        if(strstr(ContestedBlock, hash) != NULL){
                            int sock = 0, readVal;
                            struct sockaddr_in serv_addr;
                            char twoPowerTen[1024] = {0};
                            if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
                            {
                                printf("\n Socket creation error \n");
                                return;
                            }
                           
                            serv_addr.sin_family = AF_INET;
                            serv_addr.sin_addr.s_addr = INADDR_ANY;
                            
                            if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
                            {
                                perror("\nConnection Failed \n");
                                return;
                            }

                            sprintf(buffer, "approve|%s", ContestedBlock);
                            send(sock, buffer, sizeof(buffer), 0);
                            close(sock);
                        }
                        else{
                            int sock = 0, readVal;
                            struct sockaddr_in serv_addr;
                            char twoPowerTen[1024] = {0};
                            if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
                            {
                                printf("\n Socket creation error \n");
                                return;
                            }
                           
                            serv_addr.sin_family = AF_INET;
                            serv_addr.sin_addr.s_addr = INADDR_ANY;
                            serv_addr.sin_port = htons(START_PORT);
                            
                            if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
                            {
                                perror("\nConnection Failed \n");
                                return;
                            }

                            sprintf(buffer, "deny|%s", ContestedBlock);
                            send(sock, buffer, sizeof(buffer), 0);
                            close(sock);
                        }
                    }

                    if(strcmp(array[0], "approve") == 0){ 
                        numYesVotes++; 

                        if(numYesVotes >= numClients / 2){
                            insertBlock(blockChain, array[1]);
                            numYesVotes = 0;
                            u=0;
                            while(clients[u].clientPort != NULL){
                                if(strcmp(clients[u].clientPort, array[1]) != 0){
                                    int sock = 0, readVal;
                                    struct sockaddr_in serv_addr;
                                    char twoPowerTen[1024] = {0};
                                    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
                                    {
                                        printf("\n Socket creation error \n");
                                        return;
                                    }

                                    serv_addr.sin_family = AF_INET;
                                    serv_addr.sin_addr.s_addr = INADDR_ANY;
                                    serv_addr.sin_port = htons(atoi(clients[u].clientPort));
                                    
                                    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
                                    {
                                        perror("\nConnection Failed \n");
                                        return;
                                    }

                                    Node* head = blockChain->head;

                                    char* current = malloc(sizeof(char)*strlen(blockChain->head->data)*100);
                                    strcpy(current, blockChain->head->data);
                                    
                                    while(blockChain->head->next != NULL){
                                        blockChain->head = blockChain->head->next;
                                        strcat(current, ",");
                                        strcat(current, blockChain->head->data);
                                    }
                                    strcat(current, ",");
                                    blockChain->head = head;

                                    sprintf(buffer, "start|%s", current);
                                    send(sock, buffer, sizeof(buffer), 0);
                                    close(sock);
                                    free(current);
                                }
                                u++;
                            }
                            
                        }
                    }

                    if(strcmp(array[0], "deny") == 0){
                        numYesVotes--;
                    }


                    if(strcmp(array[0], "disconnect") == 0){
                        u=0;
                        while(clients[u].clientPort != NULL){
                            if(strcmp(clients[u].clientPort, array[1]) == 0){
                                while(clients[u].clientPort != NULL){
                                    clients[u].clientPort = clients[u+1].clientPort;
                                    u++;
                                }
                            }
                            u++;
                        }
                        numClients--;

                    }
                    if(PORT == START_PORT)
                        printf("\nTotal Clients: %d\n", numClients);
                    printf("\nblockChain: ");
                    printLinkedList(blockChain);
                    FD_CLR(i, &current_sockets);
                }
            }
        }
        if (k == (FD_SETSIZE * 2))
        break;
    }
}