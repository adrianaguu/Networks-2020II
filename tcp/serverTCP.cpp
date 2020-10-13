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
string message_sent;
struct sockaddr_in stSockAddr;
int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
int n;
int port = 50002;

map<string, int> ClientTable;


void read_s(int ConnectFD, string mynick){
  while(message_sent != "chau"){
  bzero(buffer,256);
     read(ConnectFD,buffer,1);
  
      if (buffer[0] == 'M'){
        read(ConnectFD,buffer,2);
        buffer[2]='\0';
        int size_nn = atoi(buffer);
        n = read(ConnectFD,buffer,size_nn);
        buffer[n]='\0';
        string nick = buffer;

        string n_size=std::to_string(mynick.size());
        n_size.insert(n_size.begin(),2-n_size.size(),'0');
      
        
        read(ConnectFD,buffer,3);
        buffer[3]='\0';
        int size_of_msg = atoi(buffer);
        n = read(ConnectFD,buffer,size_of_msg);
        buffer[n]='\0';
        message_sent = buffer;

        string m_size = std::to_string(message_sent.size());
        m_size.insert(m_size.begin(),3-m_size.size(),'0');

        message_sent = "m" + n_size + mynick + m_size + message_sent;
    
        if(ClientTable.find(nick)!=ClientTable.end()){
        write(ClientTable[nick],message_sent.c_str(),message_sent.size());
      }
      else{
    cout<<"that client is not connected"<<endl;
   }
      }
      else if(buffer[0] == 'Q'){
        shutdown(ConnectFD, SHUT_RDWR);
        close(ConnectFD);
        ClientTable.erase(mynick);
        return;
      } // quit
      else if(buffer[0] == 'F'){
        read(ConnectFD,buffer,2);
        buffer[2]='\0';
        int size_nn = atoi(buffer);
        n = read(ConnectFD,buffer,size_nn);
        buffer[n]='\0';
        string nick = buffer;

        string n_size=std::to_string(mynick.size());
        n_size.insert(n_size.begin(),2-n_size.size(),'0');
      
        
        read(ConnectFD,buffer,2);
        buffer[2]='\0';
        string s_size_of_fname  = buffer;

        int size_of_fname = atoi(buffer);
        n = read(ConnectFD,buffer,size_of_fname);
        buffer[n] = '\0';
        string fname = buffer;


        read(ConnectFD,buffer,4);
        buffer[4]='\0';
        string s_size_of_file  = buffer;

        int size_of_file = atoi(buffer);
        n = read(ConnectFD,buffer,size_of_file);
        buffer[n] = '\0';
        string file = buffer;

    

        message_sent = "f" + n_size + mynick + s_size_of_fname + fname + s_size_of_file + file;
    
        if(ClientTable.find(nick)!=ClientTable.end()){
        write(ClientTable[nick],message_sent.c_str(),message_sent.size());
      } // send a file
      else{
    cout<<"that client is not connected"<<endl;
   }
    }
      else  if(buffer[0] == 'L'){

        map<string, int>:: iterator it;
        int num=0;
        string list = "";
        for (it = ClientTable.begin(); it !=ClientTable.end();it++){
          num++;

          string n_size=std::to_string(it->first.size());
          n_size.insert(n_size.begin(),2-n_size.size(),'0');
          
          list+=n_size + it->first;
        }
        string s_num = to_string(num);
        s_num.insert(s_num.begin(),2-s_num.size(),'0');

        message_sent = "l" + s_num + list;
        write(ClientTable[mynick],message_sent.c_str(),message_sent.size());

      } // list of nick names
      else if(buffer[0] == 'A'){

        map<string, int>:: iterator it;
        
        read(ConnectFD,buffer,3);
        buffer[3]='\0';
        int size_of_msg = atoi(buffer);
        n = read(ConnectFD,buffer,size_of_msg);
        buffer[n]='\0';
        message_sent = buffer;

        string m_size = std::to_string(message_sent.size());
        m_size.insert(m_size.begin(),3-m_size.size(),'0');

        string n_size=std::to_string(mynick.size());
        n_size.insert(n_size.begin(),2-n_size.size(),'0');

        message_sent = "a" + n_size + mynick + m_size + message_sent;

        for (it = ClientTable.begin(); it !=ClientTable.end();it++){
          if(it->second!=ConnectFD)
        
            write(it->second,message_sent.c_str(),message_sent.size());
      
    }



      } // msg to all
     
    
     
   }

}

void write_s (int ConnectFD){
  while(message_sent != "chau"){

  cout<<"write your message: ";
     cin>> message_sent;
     n = write(ConnectFD,message_sent.c_str(),message_sent.size());
     if (n < 0) perror("ERROR writing to socket");
   }

}


 
  int main(void)
  {
    
 
    if(-1 == SocketFD)
    {
      perror("can not create socket");
      exit(EXIT_FAILURE);
    }
 
    memset(&stSockAddr, 0, sizeof(struct sockaddr_in));
 
    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(port);
    stSockAddr.sin_addr.s_addr = INADDR_ANY;
 
    if(-1 == bind(SocketFD,(const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in)))
    {
      perror("error bind failed");
      close(SocketFD);
      exit(EXIT_FAILURE);
    }
 
    if(-1 == listen(SocketFD, 10))
    {
      perror("error listen failed");
      close(SocketFD);
      exit(EXIT_FAILURE);
    }
 
    for(;;)
    {
      int ConnectFD = accept(SocketFD, NULL, NULL);
      bzero(buffer,256);
      read(ConnectFD,buffer,1);
      if (buffer[0] == 'N'){
        read(ConnectFD,buffer,2);
        buffer[2]='\0';
        int size_of_nn = atoi(buffer);
        read(ConnectFD,buffer,size_of_nn);
      }

      ClientTable[buffer] = ConnectFD;
 
      cout<<"nick connected: "<<buffer<<endl;
       message_sent = "";
    

      std::thread first (read_s,ClientTable[buffer],buffer);   
      //std::thread second (write_s,ConnectFD);

      first.detach();             
      //second.join(); 
 
     /* perform read write operations ... */
 
      //shutdown(ConnectFD, SHUT_RDWR);
 
      //close(ConnectFD);
    }
 
    close(SocketFD);
    return 0;
  }
