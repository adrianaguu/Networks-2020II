/* Server code in C */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <map>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <thread>
#include <iostream>
using namespace std;

char buffer[256];

struct sockaddr_in stSockAddr;
int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
int n;
int port = 50002;


void read_s(int ConnectFD, string mynick)
{
  string message_sent;
  while (1)
  {
    bzero(buffer, 256);
    n = read(ConnectFD, buffer, 256);

    if (n>0){
    buffer[n]='\0';

    int num = stoi(buffer);
    num = num*num*num;

    message_sent = to_string(num);

    n = write(ConnectFD,message_sent.c_str(),message_sent.size());
    if (n < 0) perror("ERROR writing to socket");
    }

  } // msg to all

}

int main(void)
{

  if (-1 == SocketFD)
  {
    perror("can not create socket");
    exit(EXIT_FAILURE);
  }

  memset(&stSockAddr, 0, sizeof(struct sockaddr_in));

  stSockAddr.sin_family = AF_INET;
  stSockAddr.sin_port = htons(port);
  stSockAddr.sin_addr.s_addr = INADDR_ANY;

  if (-1 == bind(SocketFD, (const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in)))
  {
    perror("error bind failed");
    close(SocketFD);
    exit(EXIT_FAILURE);
  }

  if (-1 == listen(SocketFD, 10))
  {
    perror("error listen failed");
    close(SocketFD);
    exit(EXIT_FAILURE);
  }

  for (;;)
  {
    int ConnectFD = accept(SocketFD, NULL, NULL);

    std::thread first(read_s, ConnectFD, buffer);

    first.detach();
  }

  close(SocketFD);
  return 0;
}
