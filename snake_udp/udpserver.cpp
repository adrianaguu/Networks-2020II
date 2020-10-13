#include<bits/stdc++.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <signal.h>
#include <atomic>
#include <iostream>
#include <string>
#include <mutex>
#include <thread>
#include <map>
#include <vector>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 256
#define ALTO 50
#define ANCHO 100
int alguienGano=0;
int rateTime=60000;
bool iniciarJuego=0;
char** mapa;
std::vector<char> direcciones;
std::vector<int> arrSockets;
std::vector<std::vector<std::pair<char,char> > > posiciones;
std::pair<char,char> manzana;

char fichas[]={	'0','1','2','3','4','5','6','7','8','9',
				'a','b','c','d','e','f','g','h','i','j',
				'k','l','m','n','o','p','q','r','s','t',
				'u','v','w','x','y','z','A','B','C','D',
				'E','F','G','H','I','J','K','L','M','N'};

/*Global variables*/
static std::atomic_uint clientCount(0); /*Atomic types can't be modyfied concurrently by different threads*/

static std::atomic_bool flag(false);

std::map<std::string, int> clients;


std::mutex clients_mutex;


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

void sendIni(int i,int socket){
	clients_mutex.lock();
	char tipo='i';
	char num1[4];
	char num2[4];
	intToChar(ALTO,num1);
	intToChar(ANCHO,num2);
	if(write(socket, &tipo, 1) < 0 || 
		write(socket,num1,4) < 0|| 
		write(socket,num2,4) < 0|| 
		write(socket,&fichas[i],1) < 0){
		printf("No se puede enviar el mensaje");
	}
	clients_mutex.unlock();
}

void sendMessage(std::string message, int socket){
	clients_mutex.lock();
	char tamanhoMensaje[4];
	intToChar(message.length(), tamanhoMensaje);
	char tipo='m';
	if(write(socket, &tipo, 1) < 0 || 
	write(socket, tamanhoMensaje, 4) < 0  ||
	write(socket,message.c_str(),message.length()) < 0){
		printf("No se puede enviar el mensaje");
	}
	clients_mutex.unlock();
}


void perdio(int i){
	printf("%d Perdio\n",i);
	direcciones[i]=10;
	char tipo='l';
	if(write(arrSockets[i], &tipo, 1) < 0){
		printf("No se puede enviar el mensaje");
	}
	for(int i=0;i<clientCount;++i){
		if(direcciones[i]!=10){
			if(!alguienGano)
				alguienGano=i+1;
			else{
				alguienGano=0;
				return;
			}
		}
	}
}
void gano(int i){
	printf("%d Gano\n",i);
	char tipo='w';
	if(write(arrSockets[i], &tipo, 1) < 0){
		printf("No se puede enviar el mensaje");
	}
}
/*void sendMap(int socket){
	clients_mutex.lock();
	char tipo='g';
	if(write(socket, &tipo, 1) < 0 || 
		write(socket,mapa[0],ANCHO*ALTO) < 0){
		printf("No se puede enviar el mensaje");
	}
	clients_mutex.unlock();
}*/


void sendMap(int socket){
	clients_mutex.lock();
	char buffer[255];
	buffer[0]='g';
	buffer[1]=clientCount;
	buffer[2]=manzana.first;
	buffer[3]=manzana.second;
	if(write(socket, buffer, 4) < 0 ){
		printf("No se puede enviar el mensaje");
	}
	for(int i=0;i<clientCount;++i){
		buffer[0]=0;
		buffer[1]=fichas[i];
		int it=2;
		if(direcciones[i]!=10){
			buffer[0]=posiciones[i].size();
			for(int j=0;j<posiciones[i].size();++j){
				buffer[it]=posiciones[i][j].first;
				buffer[it+1]=posiciones[i][j].second;
				it+=2;
			}
			if(write(socket, buffer, 2) < 0 ||
			write(socket, &buffer[2], it-2) < 0 ){
				printf("No se puede enviar el mensaje");
			}
		}else{
			if(write(socket, buffer, 2) < 0){
				printf("No se puede enviar el mensaje");
			}
		}
	}
	clients_mutex.unlock();
}

std::string getName(std::string buffer, int &i){
	std::string name;
	while(buffer[i]!=' '){
		name += buffer[i];
		i++;
	}
	return name;
}

std::string getMessage(std::string buffer, int &i){
	if(buffer[i]=='\0') return " ";
	std::string message;
	while(buffer[i]!='\0'){
		message+=buffer[i];
		i++;
	}
}

int addClient(std::string name, int socket){
	clients_mutex.lock();
	if(clients[name]!=NULL){ 
		clients_mutex.unlock();
		return -1;
	}
	clients[name] = socket;	
	clients_mutex.unlock();
	return 0;
}

void returnLista(int socket){
	std::map<std::string, int>::iterator it = clients.begin();
	while(it!=clients.end()){
		sendMessage(it->first, socket);
		it++;
	}
}
void colocarManzana(){
	do{
		manzana.first=5+rand()%(ALTO-10) ;
	 	manzana.second=5+rand()%(ANCHO-10);
	}while(mapa[manzana.first][manzana.second]!=' ');
	mapa[manzana.first][manzana.second]='*';
}

void handleClient(int clientSocket){
	char buffer[BUFFER_SIZE];	
	sendMessage("You have joined\n Please type your name:\t", clientSocket);
	int identificador=clientCount;
	clientCount++;
	bzero(buffer, BUFFER_SIZE);
	std::string name, message;
	
	int receive = read(clientSocket, buffer, 1);
	receive = read(clientSocket, buffer, 4);
	receive = read(clientSocket, buffer, charToInt(buffer));
	
	name = buffer;
	
	addClient(name, clientSocket);
	sendIni(identificador,clientSocket);
	posiciones.push_back(std::vector<std::pair<char,char> >());
	for(int i=0;i<10;i++){
		posiciones[identificador].push_back(std::make_pair(10*identificador,15) );
	}
	direcciones.push_back(0);
	arrSockets.push_back(clientSocket);
	sendMessage("I hear you buddy\n", clientSocket);
	std::string help;
	//iniciarJuego=1;
	do{
		sendMap(clientSocket);
		bzero(buffer, BUFFER_SIZE);
		name = "", message="";		
		receive = read(clientSocket, buffer, 1);		
		if(receive>0){
			if(buffer[0]=='m'){
				receive = read(clientSocket, buffer, 4);
				receive = read(clientSocket, buffer, charToInt(buffer));
				if(receive>0){
					std::string raw(buffer);
					if(raw == "lista"){ 
						returnLista(clientSocket);
						continue;
					}
					//write(clientSocket,buffer,BUFFER_SIZE);
					//sendMessage(raw+"\n", clientSocket);
					int find = raw.find(" ");
					std::string aux(raw,0,find);
					name = aux;
					std::string aux2(raw, find+1, raw.length()-1);
					message = aux2;
					sendMessage(message, clients[name]);
				}else if(receive == 0){
					printf(" Abandono el chat\n");
					break;
				}else{
					break;
				}
			}
			else if(buffer[0]=='p'){
				receive = read(clientSocket, buffer, 1);
				if(receive>0){
					if(direcciones[identificador]!=10){
						if(buffer[0]=='a'&& posiciones[identificador][0].second != posiciones[identificador][1].second+1 ){
							direcciones[identificador]=0;
						}else if(buffer[0]=='w' && posiciones[identificador][0].first != posiciones[identificador][1].first+1){
							direcciones[identificador]=1;
						}else if(buffer[0]=='d' && posiciones[identificador][0].second != posiciones[identificador][1].second-1){
							direcciones[identificador]=2;
						}else if(buffer[0]=='s' && posiciones[identificador][0].first != posiciones[identificador][1].first-1){
							direcciones[identificador]=3;
						}else if(buffer[0]=='p' && identificador==0){
							iniciarJuego=1;
						}	
						printf(" Jugador Presionando %c\n",buffer[0]);
					}
				}else if(receive == 0){
					printf(" Abandono el chat\n");
					break;
				}else{
					break;
				}
			}
		}else if(receive == 0){
			printf("Somebody left\n");
			break;
		}else{
			break;
		}
		
	}while(help != "bye");
	close(clientSocket);
	//clients.erase(name);
	
}

void listenClients(int listenfd){
	//printf("====CONGRATULATIONS WE STARTED THE CHATROOM====\n");
	int connfd = 0; /*Connects with the clients*/
	while(1){
		/*Accept the connection with the connfs socket. This socket allows clients to
		connect*/
		connfd = accept(listenfd, NULL, NULL); 
		
		/*Check server capacity*/
		if((clientCount+1) == MAX_CLIENTS || iniciarJuego){
			printf("Max number of clients connected. Connection not allowed.\n");
			close(connfd);
			continue;
		}
		std::thread(handleClient, connfd).detach();
		
		sleep(1);
		
	}
	
}

void juego(){
	colocarManzana();
	while(!iniciarJuego){
	}
	while(!alguienGano){
		for(int i=0; i<clientCount;++i){
			if(direcciones[i]==0){
				//izquierda
				if(posiciones[i].front().second>0){
					std::pair<char,char>& aux=posiciones[i].front();
					if(mapa[aux.first][aux.second-1] ==' '){
						mapa[posiciones[i].back().first][posiciones[i].back().second]=' ';
						posiciones[i].pop_back();
						mapa[aux.first][aux.second-1] =fichas[i];
						posiciones[i].insert(posiciones[i].begin(),std::make_pair(aux.first,aux.second-1) );
					}else if(mapa[aux.first][aux.second-1] =='*'){
						mapa[aux.first][aux.second-1] =fichas[i];
						posiciones[i].insert(posiciones[i].begin(),std::make_pair(aux.first,aux.second-1) );
						colocarManzana();
					}else{
						perdio(i);
					}
				}
			}else if(direcciones[i]==1){
				//arriba
				if(posiciones[i].front().first>0){
					std::pair<char,char>& aux=posiciones[i].front();
					if(mapa[aux.first-1][aux.second] ==' '){
						mapa[posiciones[i].back().first][posiciones[i].back().second]=' ';
						posiciones[i].pop_back();
						mapa[aux.first-1][aux.second] =fichas[i];
						posiciones[i].insert(posiciones[i].begin(),std::make_pair(aux.first-1,aux.second) );
					}else if(mapa[aux.first-1][aux.second] =='*'){
						mapa[aux.first-1][aux.second] =fichas[i];
						posiciones[i].insert(posiciones[i].begin(),std::make_pair(aux.first-1,aux.second) );
						colocarManzana();
					}else{
						perdio(i);
					}
				}
			}else if(direcciones[i]==2){
				//derecha
				if(posiciones[i].front().second<ANCHO-1){
					std::pair<char,char>& aux=posiciones[i].front();
					if(mapa[aux.first][aux.second+1] ==' '){
						mapa[posiciones[i].back().first][posiciones[i].back().second]=' ';
						posiciones[i].pop_back();
						mapa[aux.first][aux.second+1] =fichas[i];
						posiciones[i].insert(posiciones[i].begin(),std::make_pair(aux.first,aux.second+1) );
					}else if(mapa[aux.first][aux.second+1] =='*'){
						mapa[aux.first][aux.second+1] =fichas[i];
						posiciones[i].insert(posiciones[i].begin(),std::make_pair(aux.first,aux.second+1) );
						colocarManzana();
					}else{
						perdio(i);
					}
				}
			}else if(direcciones[i]==3){
				//abajo
				if(posiciones[i].front().first<ALTO-1){
					std::pair<char,char>& aux=posiciones[i].front();
					if(mapa[aux.first+1][aux.second] ==' '){
						mapa[posiciones[i].back().first][posiciones[i].back().second]=' ';
						posiciones[i].pop_back();
						mapa[aux.first+1][aux.second] =fichas[i];
						posiciones[i].insert(posiciones[i].begin(),std::make_pair(aux.first+1,aux.second) );
					}else if(mapa[aux.first+1][aux.second] =='*'){
						mapa[aux.first+1][aux.second] =fichas[i];
						posiciones[i].insert(posiciones[i].begin(),std::make_pair(aux.first+1,aux.second) );
						colocarManzana();
					}else{
						perdio(i);
					}
				}
			}
			usleep(rateTime);
		}
		if(rateTime>20000){
				rateTime-=50;
			}
		for(int i=0; i<clientCount;++i){
			if(direcciones[i]!=10)
				sendMap(arrSockets[i]);
		}
	}
	gano(alguienGano-1);
}


int main(void){
	struct sockaddr_in stSockAddr;
	int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	char buffer[256];
	int n;
	mapa=new char*[ALTO];
	mapa[0]=new char[ALTO*ANCHO];
	for(int j=0;j<ANCHO;j++){
		mapa[0][j]=' ';
	}
	for(int i=1;i<ALTO;i++){
		mapa[i]=mapa[i-1]+ANCHO;
		for(int j=0;j<ANCHO;j++){
			mapa[i][j]=' ';
		}	
	}
	if(-1 == SocketFD){
		perror("can not create socket");
		exit(EXIT_FAILURE);
	}

	memset(&stSockAddr, 0, sizeof(struct sockaddr_in));

	stSockAddr.sin_family = AF_INET;
	stSockAddr.sin_port = htons(20021);
	stSockAddr.sin_addr.s_addr = INADDR_ANY;

	signal(SIGPIPE, SIG_IGN);
	
	if(-1 == bind(SocketFD,(const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in))){
		perror("error bind failed");
		close(SocketFD);
		exit(EXIT_FAILURE);
	}

	if(-1 == listen(SocketFD, 10)){
		perror("error listen failed");
		close(SocketFD);
		exit(EXIT_FAILURE);
	}
	printf("====CONGRATULATIONS WE STARTED THE CHATROOM====\n");
	std::thread(listenClients, SocketFD).detach();
	
	std::thread(juego).detach();
	
	while(!flag){

	}

	close(SocketFD);
	delete[] mapa[0];
	delete[] mapa;
	return 0;
}