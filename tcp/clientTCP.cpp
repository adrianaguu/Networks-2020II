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
 
#include<fcntl.h> 


using namespace std;

char buffer[256];
string message_sent = "";
struct sockaddr_in stSockAddr;
int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
int n;
int port = 50002;

void read_s(int ConnectFD){
  while(message_sent != "chau"){
  bzero(buffer,256);
     read(ConnectFD,buffer,1);
  
      if (buffer[0] == 'm'){
        cout<<"\n";
        read(ConnectFD,buffer,2);
        buffer[2]='\0';
        int size_nn = atoi(buffer);
      
        n = read(ConnectFD,buffer,size_nn);
        buffer[n]='\0';
        string nick = buffer;
     
        
        read(ConnectFD,buffer,3);
        buffer[3]='\0';
        int size_of_msg = atoi(buffer);

        
   
        n = read(ConnectFD,buffer,size_of_msg);
        buffer[n] = '\0';
        message_sent = buffer;
    
        
        message_sent =  "[recieved]: " + nick + ": " + message_sent + "\n";

        cout<<message_sent;
      }

      else if (buffer[0] == 'a'){
        cout<<"\n";
        read(ConnectFD,buffer,2);
        buffer[2]='\0';
        int size_nn = atoi(buffer);
      
        n = read(ConnectFD,buffer,size_nn);
        buffer[n]='\0';
        string nick = buffer;
     
        
        read(ConnectFD,buffer,3);
        buffer[3]='\0';
        int size_of_msg = atoi(buffer);
   
        n = read(ConnectFD,buffer,size_of_msg);
        buffer[n] = '\0';
        message_sent = buffer;
    
        
        message_sent =  "[recieved]: " + nick + ": " + message_sent + "\n";

        cout<<message_sent;
      }

      else if (buffer[0] == 'l'){
        cout<<"\n";
        read(ConnectFD,buffer,2);
        buffer[2]='\0';
        int num = atoi(buffer);
        cout<<"USERS:"<<endl;
        for (int i=0;i<num;i++)
        {
          read(ConnectFD,buffer,2);
          buffer[2]='\0';
          int size_nn = atoi(buffer);
          n = read(ConnectFD,buffer,size_nn);
          buffer[n] = '\0';
          cout<<buffer<<endl;

        }

      }

      else if (buffer[0] == 'f'){
        cout<<"\n";
        read(ConnectFD,buffer,2);
        buffer[2]='\0';

        int size_nn = atoi(buffer);      
        n = read(ConnectFD,buffer,size_nn);
        buffer[n] = '\0';
        string nick = buffer;

        read(ConnectFD,buffer,2);
        buffer[2]='\0';

        int size_fn = atoi(buffer);      
        n = read(ConnectFD,buffer,size_fn);
        buffer[n] = '\0';
        string file_name = buffer;

        read(ConnectFD,buffer,4);
        buffer[4]='\0';

        int size_file = atoi(buffer);      
        n = read(ConnectFD,buffer,size_file);
        buffer[n] = '\0';
        string file= buffer;


        int fd = open(file_name.data(), O_WRONLY | O_CREAT | O_TRUNC, 0644); 

        write(fd, file.data(), size_file); 
  

      }

}

}

void write_s (int ConnectFD){
  cout<<"write your nickname: ";
      string nick="N";
      string n_size;
     cin>> message_sent;
     n_size=std::to_string(message_sent.size());
     n_size.insert(n_size.begin(),2-n_size.size(),'0');

     nick += n_size;
     nick += message_sent;

     //cout<<nick<<endl;

     n = write(ConnectFD,nick.c_str(),nick.size());
     if (n < 0) perror("ERROR writing to socket");

     char action = 'N';
  while(action != 'Q'){

      cout<<"Enter M to write a message\nQ to quit\nF to send a file\nL to list the users\nA to message all users: ";
     cin>> action;
     if (action=='M'){
      cout<<"to: ";
      string dest, dest_size,m_size,message;
      cin>>dest;
      cout<<"message: ";
     cin>> message;

      dest_size = std::to_string(dest.size());
     dest_size.insert(dest_size.begin(),2-dest_size.size(),'0');

     m_size = std::to_string(message.size());
     m_size.insert(m_size.begin(),3-m_size.size(),'0');

     message_sent =  "M" + dest_size + dest +  m_size + message;
     //cout<<message_sent;
     n = write(ConnectFD,message_sent.c_str(),message_sent.size());
     if (n < 0) perror("ERROR writing to socket");
     }

     else if (action=='A'){

      string message, m_size;

      cout<<"message: ";
     cin>> message;

     m_size = std::to_string(message.size());
     m_size.insert(m_size.begin(),3-m_size.size(),'0');

     message_sent =  "A" + m_size + message;
     //cout<<message_sent;
     n = write(ConnectFD,message_sent.c_str(),message_sent.size());
     if (n < 0) perror("ERROR writing to socket");
     }

     else if (action=='L'){

     message_sent =  "L";
     n = write(ConnectFD,message_sent.c_str(),message_sent.size());
     if (n < 0) perror("ERROR writing to socket");
     }

     else if (action=='Q'){

     message_sent =  "Q";
     //cout<<message_sent;
     n = write(ConnectFD,message_sent.c_str(),message_sent.size());
 
     exit(1);
     }

     else if (action=='F'){
      int fs = 1000000;
      char buffer_file[fs];
      string fn;
      cout<<"file: ";
      cin>>fn;
      int fd = open(fn.data(), O_RDONLY | O_CREAT); 
      n = read(fd,buffer_file,fs);
      buffer_file[n]='\0';
      cout<<"to: ";
      string dest, dest_size,f_size,f_n, file;
      cin>>dest;

      dest_size = std::to_string(dest.size());
     dest_size.insert(dest_size.begin(),2-dest_size.size(),'0');

     f_size = std::to_string(n);
     f_size.insert(f_size.begin(),4-f_size.size(),'0');

     f_n = std::to_string(fn.size());
     f_n.insert(f_n.begin(),2-f_n.size(),'0');

     file = buffer_file;

     message_sent = "F" + dest_size + dest + f_n + fn + f_size + file;

     //cout<<"Mess"<<message_sent<<endl;

     write(ConnectFD,message_sent.c_str(),message_sent.size());

     }
   }

}
 
  int main(void)
  {
    
 
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

    std::thread first (read_s,SocketFD);   
      std::thread second (write_s,SocketFD);

      first.join();                // pauses until first finishes
      second.join(); 
 
    shutdown(SocketFD, SHUT_RDWR);
 
    close(SocketFD);
    return 0;
  }
