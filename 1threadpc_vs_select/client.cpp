/* Client code in C */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <thread>
#include <unistd.h>
#include <iostream>
#include <vector>

#include <fcntl.h>

using namespace std;

char buffer[256];

struct sockaddr_in stSockAddr;

int n;
int port = 50002;

int num_clients = 1000;
int num_requests = 100000;

vector<int> sockets(num_clients, 0);

void client_proccess(int ConnectFD)
{

        //srand((unsigned)time(NULL));
        //int num = rand();

        string s_num = "55";

        n = write(ConnectFD, s_num.c_str(), s_num.size());
        if (n < 0)
                perror("ERROR writing to socket");

        bzero(buffer, 256);
        n = read(ConnectFD, buffer, 256);

        buffer[n] = '\0';

        cout << "server: " << buffer << endl;
}

int main(void)
{

        for (int i = 0; i < num_clients; i++)
        {
                int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
                if (-1 == SocketFD)
                {
                        perror("cannot create socket");
                        exit(EXIT_FAILURE);
                }

                memset(&stSockAddr, 0, sizeof(struct sockaddr_in));

                stSockAddr.sin_family = AF_INET;
                stSockAddr.sin_port = htons(port);
                int Res = inet_pton(AF_INET, "localhost", &stSockAddr.sin_addr);

                if (-1 == connect(SocketFD, (const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in)))
                {
                        perror("connect failed");
                        close(SocketFD);
                        exit(EXIT_FAILURE);
                }

                sockets[i] = SocketFD;
                cout<<i<<endl;
        }

        for (int i = 0; i < num_requests; i++)
        {

                srand((unsigned)time(NULL));
                int fd = rand() % num_clients;
                client_proccess(fd);
        }


        return 0;
}
