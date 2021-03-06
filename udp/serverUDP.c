
// Server side implementation of UDP client-server model
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <iostream>

using namespace std;

#define PORT     50005
#define MAXLINE 1000

// Driver code
int main() {

        int sockfd;
        char buffer[MAXLINE];
        char *hello = "Hello from server";
        struct sockaddr_in servaddr, cliaddr;

        // Creating socket file descriptor
        if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
                perror("socket creation failed");
                exit(EXIT_FAILURE);
        }

        memset(&servaddr, 0, sizeof(servaddr));
        memset(&cliaddr, 0, sizeof(cliaddr));

        // Filling server information
        servaddr.sin_family = AF_INET; // IPv4
        servaddr.sin_addr.s_addr = INADDR_ANY;
        servaddr.sin_port = htons(PORT);

        // Bind the socket with the server address
        if ( bind(sockfd, (const struct sockaddr *)&servaddr,
                        sizeof(servaddr)) < 0 )
        {
                perror("bind failed");
                exit(EXIT_FAILURE);
        }

        unsigned int len;
        int n;

        len = sizeof(cliaddr); //len is value/resuslt

  while(1){

        n = recvfrom(sockfd, (char *)buffer, MAXLINE,
                                MSG_WAITALL, ( struct sockaddr *) &cliaddr,
                                &len);
        buffer[n] = '\0';
        printf("Client : %s\n", buffer);
        cout<< cliaddr.sin_family << endl << cliaddr.sin_addr.s_addr << endl << cliaddr.sin_port <<endl;

        printf("\nType Something (q or Q to quit):");
        cin>>buffer;
        sendto(sockfd, buffer, strlen(hello),
                MSG_CONFIRM, (const struct sockaddr *) &cliaddr,
                        len);
        printf("Hello message sent.\n");
}

        return 0;
}

