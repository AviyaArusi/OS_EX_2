#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <poll.h>

#define MAX_MSG_LEN 1024

void print_usage() {
    printf("Usage: stnc [-c IP PORT | -s PORT]\n");
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        print_usage();
        return 1;
    }

    int is_server = 0;
    char* ip = NULL;
    int port = 0;

    // Parse command line arguments
    int opt;
    while ((opt = getopt(argc, argv, "c:s:")) != -1)
    {
        switch (opt)
        {
            case 'c':
                is_server = 0;
                ip = optarg;
                port = atoi(argv[optind]);
                break;
            case 's':
                is_server = 1;
                port = atoi(optarg);
                break;
            default:
                print_usage();
                return 1;
        }
    }

    // Create socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("socket");
        return 1;
    }

    // Bind to local address and port if server, otherwise connect to remote address and port
    if (is_server)
    {
        struct sockaddr_in addr = {0};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(port);
        if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
        {
            perror("bind");
            return 1;
        }
        if (listen(sockfd, 1) < 0)
        {
            perror("listen");
            return 1;
        }
        printf("Server started, listening on port %d...\n", port);
        // Accept client connection
        struct sockaddr_in client_addr = {0};
        socklen_t client_addrlen = sizeof(client_addr);
        int client_sockfd = accept(sockfd, (struct sockaddr*)&client_addr, &client_addrlen);
        if (client_sockfd < 0)
        {
            perror("accept");
            return 1;
        }
        printf("Client connected from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        sockfd = client_sockfd;
    }
    else
    {
        struct sockaddr_in addr = {0};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        if (inet_pton(AF_INET, ip, &addr.sin_addr) <= 0)
        {
            perror("inet_pton");
            return 1;
        }
        if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
        {
            perror("connect");
            return 1;
        }
        printf("Connected to server at %s:%d\n", ip, port);
    }

    // Create pollfd array for monitoring input from keyboard and socket
    struct pollfd fds[2];
    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;
    fds[1].fd = sockfd;
    fds[1].events = POLLIN;

    // Main loop
    while (1)
    {
        // Wait for events on both the keyboard and the socket
        if (poll(fds, 2, -1) < 0) 
        {
           perror("poll");
           return 1;
        }
	    // Check for events on the keyboard
    if (fds[0].revents & POLLIN)
    {
        // Read input from keyboard
        char msg[MAX_MSG_LEN] = {0};
        if (fgets(msg, MAX_MSG_LEN, stdin) == NULL)
        {
            perror("fgets");
            return 1;
        }
        // Send message to server/client
        if (send(sockfd, msg, strlen(msg), 0) < 0)
        {
            perror("send");
            return 1;
        }
    }

    // Check for events on the socket
    if (fds[1].revents & POLLIN)
    {
        // Receive message from server/client
        char msg[MAX_MSG_LEN] = {0};
        int len = recv(sockfd, msg, MAX_MSG_LEN, 0);
        if (len < 0) {
            perror("recv");
            return 1;
        }
        else if (len == 0)
        {
            printf("Connection closed by remote side.\n");
            break;
        }
        // Print received message
        printf("Received message: %s", msg);
    }
}
// Close socket
close(sockfd);
return 0;
}

//How to run the program:
// server- ./stnc -s 5555
//client- ./stnc -c 127.0.0.1 5555

