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
#include <fcntl.h>
#include <netdb.h>
#include <errno.h>
#include <sys/time.h>


#define PORT 50005
#define MAXLINE 1000

using namespace std;

char buffer[MAXLINE];
string message_sent = "";
int n;
unsigned len;
int SocketFD;

struct hostent *host;
struct sockaddr_in servaddr;

string ack;
string last_m;



void read_s(int ConnectFD)
{
  while (message_sent != "chau")
  {

    n = recvfrom(ConnectFD, (char *)buffer, MAXLINE,
                 MSG_WAITALL, (struct sockaddr *)&servaddr,
                 &len);
    buffer[n] = '\0';

    if(buffer[0]=='A'){
      message_sent = buffer;
      message_sent = message_sent.substr(1, message_sent.size() - 1);
      ack=message_sent;
      cout<<"ACK received"<<endl;
    }

    else if (buffer[0] == 'm' || buffer[0] == 'l' || buffer[0] == 'a')
    {
      message_sent = buffer;
      message_sent = message_sent.substr(1, message_sent.size() - 1);
      printf("[Recieved] %s\n", message_sent.data());
      
    }

    else if (buffer[0] == 'f')
    {

      message_sent = buffer;
      

      if (message_sent.size() > 1)
      {

        int i = message_sent.find(":");

        string nick = message_sent.substr(1, i);
        string m = message_sent.substr(i+1, message_sent.size() - 1);

        i = m.find(":");


        string file_name = m.substr(0, i);
        string file = m.substr(i + 1, m.size() - 1);


        cout<<"file " << file_name << " received from: "<<nick;
        int fd = open(file_name.data(), O_WRONLY | O_CREAT | O_TRUNC, 0644);

        write(fd, file.data(), file.size());
      }
    }
  }
}


void sendRDT(int s, string m){ 

  struct timeval start, end;
    

    long mtime, seconds, useconds;    

    gettimeofday(&start, NULL);

      sendto(s, m.data(), m.size(),
             MSG_CONFIRM, (const struct sockaddr *)&servaddr,
             sizeof(servaddr));

    last_m=m;

    //cout<<"last m"<<last_m;

    bool received=false;
             
    while(mtime<=300){


      if(ack==m){
        received=true;
        break;
      }

    gettimeofday(&end, NULL);

    seconds  = end.tv_sec  - start.tv_sec;
    useconds = end.tv_usec - start.tv_usec;

    mtime = ((seconds) * 1000 + useconds/1000.0) + 0.5;
     //printf("Elapsed time: %ld milliseconds\n", mtime);
    }

     //cout<<ack<<m<<endl;
    if(ack==m){
       
        received=true;
      
      }

    if(!received){
      sendRDT(s,m);
    }

}

void write_s(int ConnectFD)
{
  char action = 'L';
  cout << "Enter N to write your nick\n M to write a message\nQ to quit\nF to send a file\nL to list the users\nA to message all users: ";

  while (action != 'Q')
  {
    cin >> action;

    if (action == 'N')
    {
      cout << "write your nickname: ";
      cin >> buffer;
      message_sent = "n";
      message_sent += buffer;
       ack="";
      sendRDT(ConnectFD,message_sent);
      
    }

    else if (action == 'M')
    {
      cout << "to: ";
      cin >> buffer;
      message_sent = "m";
      message_sent += buffer;
      message_sent += ":";

      cout << "write your message: ";
      cin >> buffer;
      message_sent += buffer;

      ack="";
      sendRDT(ConnectFD,message_sent);
    }

    else if (action == 'Q')
    {

      message_sent = "q";
      ack="";
      sendRDT(ConnectFD,message_sent);

      exit(1);
    }

    else if (action == 'A')
    {
      cout << "write your message: ";
      cin >> buffer;
      message_sent = "a";
      message_sent += buffer;
      ack="";
      sendRDT(ConnectFD,message_sent);
    }

    else if (action == 'L')
    {

      message_sent = "l";
      cout << message_sent;
      ack="";
      sendRDT(ConnectFD,message_sent);
    }

    else if (action == 'F')
    {
      int fs = 900;
      char buffer_file[fs];
      string fn;
      cout << "file: ";
      cin >> fn;
      int fd = open(fn.data(), O_RDONLY | O_CREAT);
      n = read(fd, buffer_file, fs);
      buffer_file[n] = '\0';
      cout << "to: ";
      string dest, file;
      cin >> dest;

      file = buffer_file;

      message_sent = "f" + dest + ":" + fn + ":" + file;

      ack="";
      sendRDT(ConnectFD,message_sent);
    }
  }
}

int main(void)
{

  host = (struct hostent *)gethostbyname((char *)"localhost");

  // Creating socket file descriptor
  if ((SocketFD = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
  {
    perror("socket creation failed");
    exit(EXIT_FAILURE);
  }

  memset(&servaddr, 0, sizeof(servaddr));

  // Filling server information
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(PORT);
  servaddr.sin_addr = *((struct in_addr *)host->h_addr);

  std::thread first(read_s, SocketFD);
  std::thread second(write_s, SocketFD);

  first.join(); // pauses until first finishes
  second.join();

  //close(SocketFD);
  return 0;
}
