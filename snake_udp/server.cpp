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
#include <vector>
#include <ctype.h>
using namespace std;

#define PORT 50005
#define MAXLINE 1000

char buffer[MAXLINE];
string message_sent;
struct sockaddr_in servaddr;
int sockfd;
unsigned int len;
int n = 0;

map<char, struct sockaddr_in> ClientTable;
map<unsigned short, map<unsigned short, char>> NicksTable;
map<char, struct sockaddr_in>::iterator it;

map<char, vector<int>> players;
map<char, int> players_points;
map<char, vector<int>>::iterator it_players;

bool gameover;

const int width = 30;

const int height = 30;

char matrix[width][height];


string intToString(int num, int size)
{
  string res = to_string(num);
  res.insert(res.begin(), size - res.size(), '0');
  return res;
}

void setup()
{
  
  for (int i = 0; i < height; i++)
  {

    for (int j = 0; j < width; j++)
    {
      if (j == 0 || j == width - 1 || i == 0 || i == height - 1)

        matrix[i][j] = '#';

      else
      {
        matrix[i][j] = ' ';
      }
    }
  }
}



void Draw()
{

  for (int i = 0; i < height; i++)
  {

    for (int j = 0; j < width; j++)
    {

      cout << matrix[i][j];
    }
    cout << endl;
  }
}

void updatePlayerMatrix(char N)
{
  vector<int> nTail = players[N];

  matrix[nTail[0]][nTail[1]] = N;

  cout << nTail[0] << "," << nTail[1] << endl;

  for (int i = 2; i < 14; i += 2)
  {
    cout << nTail[i] << "," << nTail[i + 1] << endl;
    matrix[nTail[i]][nTail[i + 1]] = '*';
  }
}

string playerString(char N)
{
  string res = "";
  res += N;

  for (int i = 0; i < players[N].size(); i++)
  {
    res += intToString(players[N][i], 3);
  }

  return res;
}

void broadcastAll(string m){
  for (it = ClientTable.begin(); it != ClientTable.end(); it++)
  {
    sendto(sockfd, m.c_str(), m.size(),
           MSG_CONFIRM, (const struct sockaddr *)(&(it->second)),
           len);
  }
}

void sendToU(char N, string m){
  sendto(sockfd, m.c_str(), m.size(),
           MSG_CONFIRM, (const struct sockaddr *)(&ClientTable[N]),
           len);
}


void algorithm(char N, char M)
{

    vector<int> nTail = players[N];
    int prev0 = nTail[0];
    int prev1 = nTail[1];
    int last0 = nTail[12];
    int last1 = nTail[13];
    string m;

    switch (M)
    {

    case 'L':

        prev1--;

        break;

    case 'R':

        prev1++;

        break;

    case 'U':

        prev0--;

        break;

    case 'D':

        prev0++;

        break;

    default:

        break;
    }

    if (isalpha(matrix[prev0][prev1]))
    {
        players_points[N] += 5;
        string spoints = intToString(players_points[N],3);
        m = "W";
        m+= "660";
        m+=spoints;
        sendToU(N,m);
    }
    else if (matrix[prev0][prev1] == '#'){

      players_points[N] -= 1;
         string spoints = intToString(players_points[N],3);
        m = "W";
        m+= "600";
        m+=spoints;
        sendToU(N,m);

    }

    else if ( matrix[prev0][prev1] == '*')
    {
        players_points[N] -= 1;
         string spoints = intToString(players_points[N],3);
        m = "W";
        m+= "650";
        m+=spoints;
        sendToU(N,m);
    }

    else
    {
        nTail.pop_back();
        nTail.pop_back();

        nTail.insert(nTail.begin(), prev1);
        nTail.insert(nTail.begin(), prev0);

        matrix[nTail[0]][nTail[1]] = N;
        matrix[last0][last1] = ' ';

        for (int i = 2; i < 14; i += 2)
        {
            matrix[nTail[i]][nTail[i + 1]] = '*';
        }

        players[N] = nTail;

        m = "U";
        m+=N;
        m+=M;
        
        broadcastAll(m);
    }
}

void read_s(char N, string notif, struct sockaddr_in cliaddr)
{
  updatePlayerMatrix(N);

  string message;
  for (it = ClientTable.begin(); it != ClientTable.end(); it++)
  {
    sendto(sockfd, notif.c_str(), notif.size(),
           MSG_CONFIRM, (const struct sockaddr *)(&(it->second)),
           len);

    if (it->first != NicksTable[cliaddr.sin_addr.s_addr][cliaddr.sin_port])
    {
      message = "N";
      message += playerString(it->first);
      //cout << "mess:" << message << endl;
      sendto(sockfd, message.c_str(), message.size(),
             MSG_CONFIRM, (const struct sockaddr *)&cliaddr,
             len);
    }
  }

   Draw();
  while (!gameover)
  {
    n = recvfrom(sockfd, (char *)buffer, MAXLINE,
                 MSG_WAITALL, (struct sockaddr *)&servaddr,
                 &len);
    buffer[n] = '\0';
    //cout<<"buff: "<<buffer<<'\n';
    if (buffer[0] == 'M')
    {
      algorithm(buffer[2],buffer[1]);
    }
     Draw();
  }
}

int main(void)
{

  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
  {
    perror("socket creation failed");
    exit(EXIT_FAILURE);
  }

  memset(&servaddr, 0, sizeof(servaddr));

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

  setup();
  struct sockaddr_in cliaddr;
  memset(&cliaddr, 0, sizeof(cliaddr));
  len = sizeof(cliaddr);
  for (;;)
  {
    //if (n)
    //{
    n = recvfrom(sockfd, (char *)buffer, 2 + (14 * 3),
                 MSG_WAITALL, (struct sockaddr *)&cliaddr,
                 &len);
    buffer[n] = '\0';
    //cout<<buffer<<endl;

    if (buffer[0] == 'N')
    {
      if (ClientTable.find(buffer[1]) != ClientTable.end())
      {
        message_sent = "E400";
        sendto(sockfd, message_sent.c_str(), message_sent.size(),
               MSG_CONFIRM, (const struct sockaddr *)(&cliaddr),
               len);
      }
      else
      {
        string notif = buffer;
        cout << notif[1] << " connected" << endl;
        char N = notif[1];
        ClientTable[notif[1]] = cliaddr;
        NicksTable[cliaddr.sin_addr.s_addr][cliaddr.sin_port] = notif[1];
        cout << "nc" << NicksTable[cliaddr.sin_addr.s_addr][cliaddr.sin_port];
        message_sent = "O200";
        sendto(sockfd, message_sent.c_str(), message_sent.size(),
               MSG_CONFIRM, (const struct sockaddr *)(&cliaddr),
               len);
        ;

        vector<int> cords(14);

        int index = 0;
        for (int i = 0; i < 14 * 3; i += 3)
        {

          string cord = buffer;
          cord = cord.substr(2 + i, 3);
          //cout<<cord.data()<<":";
          sscanf(cord.data(), "%d", &cords[index]);
          //cout<<cords[index]<<'\n';
          index++;
        }

        players[N] = cords;
        players_points[N] = 100;

        std::thread first(read_s, N, notif, cliaddr);
        first.detach();
      }
    }

    else if (buffer[0] == 'M')
    {
      algorithm(buffer[2],buffer[1]);
       Draw();
    }

    //first.detach();
    //second.join();
    //}
  }

  //close(SocketFD);
  return 0;
}
