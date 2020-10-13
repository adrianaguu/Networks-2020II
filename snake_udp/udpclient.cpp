#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <thread>
#include <string>
#include <iostream>
#include <windows.h>
#include <termios.h>
#include<bits/stdc++.h> 
#include <errno.h>
#include <atomic>
#include <mutex>
#include <map>
#include <vector>

std::mutex pantallamutex;
std::vector<std::pair<char,char> >  posicionesAnt;


/* reads from keypress, doesn't echo */
int getch(void)
{
    struct termios oldattr, newattr;
    int ch;
    tcgetattr( STDIN_FILENO, &oldattr );
    newattr = oldattr;
    newattr.c_lflag &= ~( ICANON | ECHO );
    tcsetattr( STDIN_FILENO, TCSANOW, &newattr );
    ch = getchar();
    tcsetattr( STDIN_FILENO, TCSANOW, &oldattr );
    return ch;
}

/* reads from keypress, echoes */
int getche(void)
{
    struct termios oldattr, newattr;
    int ch;
    tcgetattr( STDIN_FILENO, &oldattr );
    newattr = oldattr;
    newattr.c_lflag &= ~( ICANON );
    tcsetattr( STDIN_FILENO, TCSANOW, &newattr );
    ch = getchar();
    tcsetattr( STDIN_FILENO, TCSANOW, &oldattr );
    return ch;
}

#define BUFFER_SIZE 256



HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
COORD CursorPosition;

int ALTO=50;
int ANCHO=70;

char ficha;

char** mapa;

int SocketFD;
bool flag;
bool flagMensaje=1;

void intToChar(int x, char* c){
	for(int i = 3; i>=0;--i){
		c[i]=x%256;
		x/=256;
	}
}

int charToInt(char* c){
	int x=0;
	for(int i = 0; i<4;++i){
		x*=256;
		x+=c[i];
	}
	return x;
}
void gotoXY(int x, int y) 
{ 
CursorPosition.X = x; 
CursorPosition.Y = y; 
SetConsoleCursorPosition(console,CursorPosition); 
}

void sendMessage(){
	std::string message;
	char buffer[BUFFER_SIZE];
	
	do{
    if(flagMensaje){
      flagMensaje=0;
      getline(std::cin, message);
      char tamanhoMensaje[4];
      intToChar(message.length(), tamanhoMensaje);
      char tipo='m';
      if(write(SocketFD, &tipo, 1) < 0 || 
      write(SocketFD, tamanhoMensaje, 4) < 0  ||
      write(SocketFD, message.c_str(), message.length()) < 0){
        printf("Couldn't send the message");
        return;
      }
    }else{
      char c=getch();
      char tipo='p';
      if(write(SocketFD, &tipo, 1) < 0 || 
      write(SocketFD, &c, 1) < 0){
        printf("Couldn't send the message");
        return;
      }
    }
	}while(message.compare("bye") != 0);
	flag = 1;
	return;
}

void recvMessage(){
	char buffer[BUFFER_SIZE];
	while(1){
    if(read(SocketFD, buffer, 1) > 0){
      if(buffer[0]=='m'){
				read(SocketFD, buffer, 4);
				read(SocketFD, buffer, charToInt(buffer));
				printf("%s\n", buffer);
				bzero(buffer, BUFFER_SIZE);
      }
      else if(buffer[0]=='g'){
        pantallamutex.lock();
        for(int i=0;i<posicionesAnt.size();i++){
          mapa[posicionesAnt[i].first][posicionesAnt[i].second]=' ';
        }
        posicionesAnt.resize(0);
        read(SocketFD, buffer, 3);
        
          mapa[buffer[1]][buffer[2]]='*';
        int n=buffer[0];
        for(int i=0;i<n;++i){
          read(SocketFD, buffer, 2);
          int m=buffer[0];
          char c=buffer[1];
          read(SocketFD, buffer, m*2);
          for(int j=0;j<m*2;j+=2){
            mapa[buffer[j]][buffer[j+1]]=c;
            posicionesAnt.push_back(std::make_pair(buffer[j],buffer[j+1]) );
          }
        }
        gotoXY(0,0);
        for(int i=0;i<ANCHO+2;i++){
          printf("#");
        }
        printf("\n");
        for(int i=0;i<ALTO;i++){
          printf("#%s#\n", mapa[i]);
        }
        for(int i=0;i<ANCHO+2;i++){
          printf("#");
        }
        printf("\n");
        pantallamutex.unlock();
      }
      else if(buffer[0]=='i'){
				read(SocketFD, buffer, 4);
        ALTO=charToInt(buffer);
        read(SocketFD, buffer, 4);
        ANCHO=charToInt(buffer);
        read(SocketFD, buffer, 1);
        ficha=buffer[0];
        bzero(buffer, BUFFER_SIZE);
        mapa=new char*[ALTO];
        for(int i=0;i<ALTO;i++){
          mapa[i]=new char[ANCHO+1];
          for(int j=0;j<ANCHO;j++){
            mapa[i][j]=' ';
          }	
          mapa[i][ANCHO]=0;
        }
        printf("\nJuegas con la ficha %c\n",ficha);
        flagMensaje=0;
        
      }else if(buffer[0]=='l'){
        gotoXY(0,(ALTO-7)/2);
        printf("       ######   #######  #####    ######   ######   ####   ######  ####### \n");
        printf("       ##    #  ##       ##   #   ##    #    ##    ##   #    ##    ##      \n");
        printf("       ##    #  ##       ##   #   ##     #   ##    ##        ##    ##      \n");
        printf("       #####    #####    #####    ##     #   ##     ####     ##    #####   \n");
        printf("       ##       ##       ##   #   ##     #   ##        ##    ##    ##      \n");
        printf("       ##       ##       ##    #  ##    #    ##    #   ##    ##    ##      \n");
        printf("       ##       #######  ##    #  ######   ######   ####     ##    ####### \n");
    }else if(buffer[0]=='w'){
        gotoXY(0,(ALTO-7)/2);
        printf("         ####     ###    ##   ##     ###     ####   ######  ####### \n");
        printf("       ##    #  ##   ##  ###  ##   ##   ##  ##   #    ##    ##      \n");
        printf("       ##       ##   ##  ## # ##   ##   ##  ##        ##    ##      \n");
        printf("       ##  ###  #######  ## # ##   #######   ####     ##    #####   \n");
        printf("       ##   ##  ##   ##  ##  ###   ##   ##      ##    ##    ##      \n");
        printf("       ##   ##  ##   ##  ##  ###   ##   ##  #   ##    ##    ##      \n");
        printf("         ###    ##   ##  ##   ##   ##   ##   ####     ##    ####### \n");
      }
    }else{
			break;
		}
	}
	return;
}
 
int main(void){
  struct sockaddr_in stSockAddr;
  int Res;
  SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  int n;

  if (-1 == SocketFD){
    perror("cannot create socket");
    exit(EXIT_FAILURE);
  }

  memset(&stSockAddr, 0, sizeof(struct sockaddr_in));

  stSockAddr.sin_family = AF_INET;
  stSockAddr.sin_port = htons(20021);
  //Res = inet_pton(AF_INET, "51.15.220.108", &stSockAddr.sin_addr);
  Res = inet_pton(AF_INET, "127.0.0.1", &stSockAddr.sin_addr);

  if (0 > Res){
    perror("error: first parameter is not a valid address family");
    close(SocketFD);
    exit(EXIT_FAILURE);
  }
  else if (0 == Res){
    perror("char string (second parameter does not contain valid ipaddress");
    close(SocketFD);
    exit(EXIT_FAILURE);
  }
  if (-1 == connect(SocketFD, (const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in))){
    perror("connect failed");
    close(SocketFD);
    exit(EXIT_FAILURE);
  }
  printf("=======Welcome to Chatroom=======\n");
	
	
	std::thread(recvMessage).detach();
	std::thread(sendMessage).detach();
  while(!flag);
	
  shutdown(SocketFD, SHUT_RDWR);

  close(SocketFD);
  delete[] mapa[0];
	delete[] mapa;
  return EXIT_SUCCESS;
}