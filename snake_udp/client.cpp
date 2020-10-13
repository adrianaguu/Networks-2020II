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
#include <map>
#include <vector>
#include <ctype.h>

#define PORT 50005
#define MAXLINE 1000

using namespace std;

char buffer[MAXLINE];
string message_sent = "";
int n;
unsigned len;
int SocketFD;

bool gameover = false;

char N;

struct hostent *host;
struct sockaddr_in servaddr;

const int width = 30;

const int height = 30;

char matrix[width][height];
int points;

enum eDirecton
{
  STOP = 0,
  LEFT,
  RIGHT,
  UP,
  DOWN
}; // Controls

eDirecton dir;

map<char, vector<int>> players;
map<char, vector<int>>::iterator it_players;

void setup()
{
  points=100;
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

void updatePlayerMatrix(char N)
{
  vector<int> nTail = players[N];

  matrix[nTail[0]][nTail[1]] = N;

  //cout<<nTail[0]<<","<<nTail[1]<<endl;

  for (int i = 2; i < 14; i += 2)
  {
    //     cout<<nTail[i]<<","<<nTail[i+1]<<endl;
    matrix[nTail[i]][nTail[i + 1]] = '*';
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

  cout<<"POINTS: " <<endl;
}


void algorithm(char N, char M)
{

  vector<int> nTail = players[N];
  int prev0 = nTail[0];
  int prev1 = nTail[1];
  int last0 = nTail[12];
  int last1 = nTail[13];

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
}


void read_s(char N)
{
  string message;
  while (!gameover)
  {
    n = recvfrom(SocketFD, (char *)buffer, MAXLINE,
                 MSG_WAITALL, (struct sockaddr *)&servaddr,
                 &len);
    buffer[n] = '\0';

    if (buffer[0] == 'N')
    {

      char N = buffer[1];
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
      updatePlayerMatrix(N);
    }

    else if (buffer[0] == 'U')
    {
      algorithm(buffer[1],buffer[2]);
    }

    else if (buffer[0] == 'W')
    {
      string spoints = buffer;
      spoints = spoints.substr(4,3);
      points = atoi(spoints.data());
    }
    Draw();
  }
}

void write_s(char N)
{
  while (!gameover)
  {
    char c;
    cin >> c;
    switch (c)
    {

    case 'a':

      c = 'L';

      break;

    case 'd':

      c = 'R';

      break;

    case 'w':

      c = 'U';

      break;

    case 's':

      c = 'D';

      break;
    }
    string message = "M";
    message += c;
    message += N;

    sendto(SocketFD, message.data(), message.size(),
           MSG_CONFIRM, (const struct sockaddr *)&servaddr,
           sizeof(servaddr));
  }
}

string intToString(int num, int size)
{
  string res = to_string(num);
  res.insert(res.begin(), size - res.size(), '0');
  return res;
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
  setup();
  memset(&servaddr, 0, sizeof(servaddr));

  // Filling server information
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(PORT);
  servaddr.sin_addr = *((struct in_addr *)host->h_addr);

  cout << "Enter a letter for your nickname:";

  cin >> N;
  message_sent = "N";
  message_sent += N;

  players[N] = vector<int>(14);
  srand((unsigned)time(NULL));
  players[N][0] = rand() % (width - 8) + 7;
  players[N][1] = rand() % (width - 8) + 7;

  message_sent += intToString(players[N][0], 3);
  message_sent += intToString(players[N][1], 3);

  for (int i = 2; i < 14; i += 2)
  {
    players[N][i] = players[N][i - 2] - 1;
    message_sent += intToString(players[N][i], 3);
    players[N][i + 1] = players[N][1];
    message_sent += intToString(players[N][i + 1], 3);
  }

  sendto(SocketFD, message_sent.data(), message_sent.size(),
         MSG_CONFIRM, (const struct sockaddr *)&servaddr,
         sizeof(servaddr));

  n = recvfrom(SocketFD, (char *)buffer, 4,
               MSG_WAITALL, (struct sockaddr *)&servaddr,
               &len);
  buffer[n] = '\0';

  if (buffer[0] == 'E')
  {
    cout << "letter is already used" << endl;
  }

  else if (buffer[0] == 'O')
  {

    std::thread first(read_s, N);
    std::thread second(write_s, N);

    first.join(); // pauses until first finishes
    second.join();
  }

  //close(SocketFD);
  return 0;
}
