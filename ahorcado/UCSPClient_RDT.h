// Client side implementation of UDP client-server model
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <thread>
#include <sys/time.h>
#include <vector>
#include <algorithm>
#include<math.h>

using namespace std;

#define PORT 60001
#define MAXLINE 1000

unsigned int id = 0;
int hash_cheksum = 666;

string ack(10, 'X');

vector<string> id_last_m;

string m_error = "msg recibido con errores en datos, segun el hash";

char buffer[MAXLINE];

struct hostent *host;

struct sockaddr_in servaddr;

int ConnectFD;

string intToString(int num, int size)
{
        string res = to_string(num);
        res.insert(res.begin(), size - res.size(), '0');
        return res;
}

string padding(string m, int size)
{
        string s0(size - m.size(), '0');
        m += s0;
        return m;
}

int calculateHash(string m)
{
        int sum = 0;
        for (unsigned int i = 0; i < m.size(); i++)
        {
                sum += m[i];
        }

        float rest = sum/hash_cheksum - floor(sum/hash_cheksum);

        return sum*rest;
}

bool recievedBefore(string val)
{
        std::vector<string>::iterator it;
        it = find(id_last_m.begin(), id_last_m.end(), val);
        if (it != id_last_m.end())
                return true;
        else
                return false;
}

string buildMessage(char c, string message, int _id)
{
        string size_m = intToString(message.size(), 3);

        string m_id = intToString(_id, 5);

        string hash = intToString(calculateHash(message), 3);

        message = c + m_id + size_m + message + hash;

        message = padding(message, MAXLINE);

        return message;
}

string sendRDT(int s, string m, int times)
{
       
        if (times == 3)
        {
                return "";
        }

        struct timeval start, end;

        long mtime, seconds, useconds;

        mtime = 0;
        gettimeofday(&start, NULL);

        sendto(s, m.data(), m.size(),
               MSG_CONFIRM, (const struct sockaddr *)&servaddr,
               sizeof(servaddr));

        string m_id = m.substr(1, 5);

        while (mtime <= 1300)
        {

                //cout<<"mid "<<m_id<<endl;
                
                if (ack.substr(1, 5) == m_id)
                {
                        //cout<<"ack "<<ack<<endl;
                        return ack;
                }

                gettimeofday(&end, NULL);

                seconds = end.tv_sec - start.tv_sec;
                useconds = end.tv_usec - start.tv_usec;

                mtime = ((seconds)*1000 + useconds / 1000.0) + 0.5;

                //printf("Elapsed time: %ld milliseconds\n", mtime);
        }

        times++;
        return sendRDT(s, m, times);
}

void requestProcces( string message, string *response)
{
        string _read = "";
        _read = sendRDT(ConnectFD, message, 0);
     
        if (_read.size() > 0)
        {
                //cout<<"read2 "<<_read<<endl;

                string m_id = _read.substr(1, 5);

                int m_size = stoi(_read.substr(6, 3));

                string m_recieved = _read.substr(9, m_size);

                int hash_recieved = stoi(_read.substr(9 + m_size, 3));

                if (m_recieved == m_error)
                {
                        *response = m_error;
                }

                else if (hash_recieved == calculateHash(m_recieved))
                {
                        printf("%s\n","msg recibido sin problemas");
                        *response = m_recieved;
                }

                else
                {
                        *response = "Acuse con errorres en datos, segun hash";
                }
        }
        else
        {
                *response = "Time Out reach";
                //cout<<"yime";
        }
}

string Request(string message)
{
        message = buildMessage('1', message, id);

        id++;

        string response;

        std::thread process(requestProcces, message, &response);

        process.join();

         //cout<<"Response: "<< response <<endl;

        return response;
}

string RequestData(string message)
{
        message = buildMessage('B', message, id);

        id++;

        //cout<<message;

        string response="";

        std::thread process(requestProcces, message, &response);

        process.join();

        

        while(response.size()<0){}


        return response;
}

void sendandforgetProcces( string message, string *response, bool *succes)
{
        string read = "";
        read = sendRDT(ConnectFD, message, 0);

        if (read.size() > 0)
        {

                string m_id = read.substr(1, 5);

                int m_size = stoi(read.substr(6, 3));

                string m_recieved = read.substr(9, m_size);

                int hash_recieved = stoi(read.substr(9 + m_size, 3));

                if (m_recieved == "0")
                {
                        *succes = false;
                        *response = m_error;
                }

                else if (hash_recieved == calculateHash(m_recieved))
                {
                        *succes = true;
                        *response = "msg recibido sin problemas: " + m_recieved;
                }

                else
                {
                        *succes = false;
                        *response = "Acuse con errorres en datos, segun hash";
                }
        }
        else
        {
                *succes = false;
                *response = "Time Out reach";
        }
}

bool SendAndForget( string message)
{
        message = buildMessage('5', message,  id);

        id++;

        string response;

        bool success;

        std::thread process(sendandforgetProcces, message, &response, &success);

        process.join();

       

        return success;
}



void read_s(int sockfd)
{
        int n;
        unsigned int len;
        len = sizeof(servaddr);

        while (1)
        {

                n = recvfrom(sockfd, (char *)buffer, MAXLINE,
                             MSG_WAITALL, (struct sockaddr *)&servaddr,
                             &len);
                buffer[n] = '\0';

                string read = buffer;
                ack = read;

                char type_m = buffer[0];

                string m_id = read.substr(1, 5);

                if (!recievedBefore(m_id))
                {

                        id_last_m.push_back(m_id);

                        int m_size = stoi(read.substr(6, 3));

                        string m_recieved = read.substr(9, m_size);

                        printf("Server : %s\n", m_recieved.data());

                        int hash_recieved = stoi(read.substr(9 + m_size, 3));

                        int calc_hash = calculateHash(m_recieved);

                        //printf("type: %c\n", type_m);

                        if (type_m == '3')
                        {
                                //cout<<read<<endl;

                                string m = buildMessage('4', "", stoi(m_id));

                                if (hash_recieved != calc_hash)
                                {
                                        m = "msg recibido con errores en datos, segun el hash";
                                }

                                //cout <<"sending: " << m << endl;

                                sendto(sockfd, m.data(), m.size(),
                                       MSG_CONFIRM, (const struct sockaddr *)&servaddr,
                                       len);
                        }

                        else if (type_m == '6')
                        {
                                string m = buildMessage('A', "1", stoi(m_id));

                                if (hash_recieved != calc_hash)
                                {
                                        m = buildMessage('A', "0", stoi(m_id));
                                }

                                sendto(sockfd, m.data(), m.size(),
                                       MSG_CONFIRM, (const struct sockaddr *)&servaddr,
                                       len);
                        }
                }
        }
}

// Driver code
void init()
{
        

        host = (struct hostent *)gethostbyname((char *)"192.99.8.130");

        // Creating socket file descriptor
        if ((ConnectFD = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        {
                perror("socket creation failed");
                exit(EXIT_FAILURE);
        }

        memset(&servaddr, 0, sizeof(servaddr));

        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(PORT);
        servaddr.sin_addr = *((struct in_addr *)host->h_addr);

        int n, len;

        std::thread first(read_s, ConnectFD);

        //std::thread second(write_s, sockfd);
        first.detach();
        //second.join();

}
