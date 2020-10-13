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

#define PORT 50005
#define MAXLINE 1000

char buffer[MAXLINE];
string message_sent;
struct sockaddr_in servaddr, cliaddr;
int sockfd;
unsigned int len;
int n = 0;

map<string, struct sockaddr_in> ClientTable;
map<unsigned short, string> NicksTable;
map<string, struct sockaddr_in>::iterator it;

int main(void)
{

  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
  {
    perror("socket creation failed");
    exit(EXIT_FAILURE);
  }

  memset(&servaddr, 0, sizeof(servaddr));
  memset(&cliaddr, 0, sizeof(cliaddr));

  // Filling server information
  servaddr.sin_family = AF_INET; // IPv4
  servaddr.sin_addr.s_addr = INADDR_ANY;
  servaddr.sin_port = htons(PORT);

  if (bind(sockfd, (const struct sockaddr *)&servaddr,
           sizeof(servaddr)) < 0)
  {
    perror("bind failed");
    exit(EXIT_FAILURE);
  }

  len = sizeof(cliaddr);

  for (;;)
  {
    //if (n)
    //{
    n = recvfrom(sockfd, (char *)buffer, MAXLINE,
                 MSG_WAITALL, (struct sockaddr *)&cliaddr,
                 &len);
    buffer[n] = '\0';

    //string mynick = get_nick();


    if (buffer[0] == 'n')
    {
      message_sent = buffer;
      message_sent = message_sent.substr(1, message_sent.size() - 1);
      cout << message_sent << " connected" << endl;
      ClientTable[message_sent] = cliaddr;
      NicksTable[cliaddr.sin_port] = message_sent;
    }

    else if (buffer[0] == 'm')
    {
      message_sent = buffer;
      if (message_sent.size() > 1)
      {
        message_sent = message_sent.substr(1, message_sent.size() - 1);
        int i = message_sent.find(":");

        string nick = message_sent.substr(0, i);
        string m = message_sent.substr(i, message_sent.size() - 1);

    

        if (ClientTable.find(nick) != ClientTable.end())
        {

          message_sent = "m";
          message_sent += NicksTable[cliaddr.sin_port];
          message_sent += m;
          

          sendto(sockfd, message_sent.c_str(), message_sent.size(),
                 MSG_CONFIRM, (const struct sockaddr *)(&ClientTable[nick]),
                 len);
        }

        else
        {
          cout << "that client is not connected" << endl;
        }
      }
    }

    else if (buffer[0] == 'l')
    {

      string list = "lusers: ";
      for (it = ClientTable.begin(); it != ClientTable.end(); it++)
      {

        list += it->first + ", ";
      }

      sendto(sockfd, list.c_str(), list.size(),
             MSG_CONFIRM, (const struct sockaddr *)(&cliaddr),
             len);
    }

    else if (buffer[0] == 'a')
    {
      message_sent = buffer;
      if (message_sent.size() > 1)
      {
        message_sent = message_sent.substr(1, message_sent.size() - 1);
       
        message_sent = "a" + NicksTable[cliaddr.sin_port] + ": " + message_sent;
        
        for (it = ClientTable.begin(); it != ClientTable.end(); it++)
        {

          if (it->first != NicksTable[cliaddr.sin_port])
            sendto(sockfd, message_sent.c_str(), message_sent.size(),
                   MSG_CONFIRM, (const struct sockaddr *)(&(it->second)),
                   len);
        }
      }
    }

    else if(buffer[0] == 'q')
    {
      ClientTable.erase(NicksTable[cliaddr.sin_port]);
    }

    else if(buffer[0] == 'f')
    {
      message_sent = buffer;
  
      if (message_sent.size() > 1)
      {
       
        int i = message_sent.find(":");

        string nick = message_sent.substr(1, i-1);
        string m = message_sent.substr(i, message_sent.size() - 1);
   

    

        if (ClientTable.find(nick) != ClientTable.end())
        {

          message_sent = "f";
          message_sent += NicksTable[cliaddr.sin_port];
          message_sent += m;

     
          

          sendto(sockfd, message_sent.c_str(), message_sent.size(),
                 MSG_CONFIRM, (const struct sockaddr *)(&ClientTable[nick]),
                 len);
        }

        else
        {
          message_sent = "that client is not connected";
          sendto(sockfd, message_sent.c_str(), message_sent.size(),
                 MSG_CONFIRM, (const struct sockaddr *)(&ClientTable[nick]),
                 len);
        }
      }
    }

    

    //std::thread first(read_s,buffer);

    //first.detach();
    //second.join();
    //}
  }

  //close(SocketFD);
  return 0;
}
