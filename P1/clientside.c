#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>  // new

#define PORT "43096"
#define MAXDATASIZE 4096

void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[]) {
    int sockfd, numbytes;
    char buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    struct timespec start, end;  // new

    if (argc != 3) {
        fprintf(stderr,"usage: client hostname command\n");
        exit(1);
    }

    if (strcmp(argv[2], "Get") != 0) {
        fprintf(stderr, "Command not recognized.\n");
        exit(1);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
    printf("client: connecting to %s\n", s);
    freeaddrinfo(servinfo);

    clock_gettime(CLOCK_MONOTONIC, &start);  // new

    if (send(sockfd, "Get\n", 4, 0) == -1) {
        perror("send");
        close(sockfd);
        return 1;
    }

    while ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) > 0) {
        buf[numbytes] = '\0';
        printf("%s", buf);

    }

    clock_gettime(CLOCK_MONOTONIC, &end);  // new

    double dt = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;  //new
    printf("Time: %f seconds\n", dt);

    if (numbytes == -1) {
        perror("recv");
        close(sockfd);
        return 1;
    }

    close(sockfd);
    return 0;
}
