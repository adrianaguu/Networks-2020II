
// Server side implementation of UDP client-server model
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <thread>
#include <vector>
#include <iostream>
#include <algorithm>
#include <sys/time.h>
#include<math.h>
using namespace std;

#define PORT 60001
#define MAXLINE 1000

struct sockaddr_in servaddr, cliaddr;

unsigned int id = 0;
int hash_cheksum = 666;
int ConnectFD;

unsigned int len = sizeof(cliaddr);

string m_error = "msg recibido con errores en datos, segun el hash";

string ack(10, 'X');

vector<string> id_last_m;

string received = "";
string received_id = "";

bool requested = false;

char buffer[MAXLINE];

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

bool recievedBefore(string val)
{
        std::vector<string>::iterator it;
        it = find(id_last_m.begin(), id_last_m.end(), val);
        if (it != id_last_m.end())
                return true;
        else
                return false;
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
               MSG_CONFIRM, (const struct sockaddr *)&cliaddr,
               len);

        string m_id = m.substr(1, 5);

        while (mtime <= 300)
        {

                //cout<<"mid "<<m_id<<endl;
                //cout<<"ack "<<ack.substr(1, 5)<<endl;
                if (ack.substr(1, 5) == m_id)
                {

                        return ack;
                }

                gettimeofday(&end, NULL);

                seconds = end.tv_sec - start.tv_sec;
                useconds = end.tv_usec - start.tv_usec;

                mtime = ((seconds)*1000 + useconds / 1000.0) + 0.5;

                //printf("Elapsed time: %ld milliseconds\n", mtime);
        }

        times++;
        sendRDT(s, m, times);
}

void SolicitateProcces(string message, string *response)
{
        string read = "";
        read = sendRDT(ConnectFD, message, 0);

        if (read.size() > 0)
        {

                string m_id = read.substr(1, 5);

                int m_size = stoi(read.substr(6, 3));

                string m_recieved = read.substr(9, m_size);

                int hash_recieved = stoi(read.substr(9 + m_size, 3));

                if (m_recieved == m_error)
                {
                        *response = m_error;
                }

                else if (hash_recieved == calculateHash(m_recieved))
                {
                        *response = "msg recibido sin problemas: " + m_recieved;
                }

                else
                {
                        *response = "Acuse con errorres en datos, segun hash";
                }
        }
        else
        {
                *response = "Time Out reach";
        }
}

string Solicitate(string message, int _id)
{
        message = buildMessage('3', message, _id);

        string response;

        std::thread process(SolicitateProcces, message, &response);

        process.join();

        return response;
}

void NotificateProcces(string message, string *response, bool *succes)
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

bool Notificate(string message, int _id)
{
        message = buildMessage('6', message, _id);

        string response;

        bool success;

        std::thread process(NotificateProcces, message, &response, &success);

        process.join();

        //cout<<"Response: "<< response <<endl;

        return success;
}

bool ResponseData(string _data)
{
        if (requested)
        {
                requested = false;
                string m = buildMessage('7', _data, stoi(received_id));
                sendto(ConnectFD, m.data(), m.size(),
                       MSG_CONFIRM, (const struct sockaddr *)&cliaddr,
                       len);
                return true;
        }
        else
        {
                return false;
        }
}

bool Response()
{
        if (requested)
        {
                requested = false;
                string m = buildMessage('7', "", stoi(received_id));
                sendto(ConnectFD, m.data(), m.size(),
                       MSG_CONFIRM, (const struct sockaddr *)&cliaddr,
                       len);
        }
        else
        {
                return false;
        }
}

string getReceived()
{
        printf("%s", "waiting...\n");//received.data());
        return received;
}

void cleanReceived()
{
        received="";
}

void read_s(int sockfd)
{
        int n;

        while (1)
        {

                n = recvfrom(sockfd, (char *)buffer, MAXLINE,
                             MSG_WAITALL, (struct sockaddr *)&cliaddr,
                             &len);
                buffer[n] = '\0';

                string read = buffer;
                ack = read;

                //printf("ack : %s\n", ack.data());

                char type_m = buffer[0];

                string m_id = read.substr(1, 5);

                printf("M id : %s\n", m_id.data());

                if (!recievedBefore(m_id))
                {

                        id_last_m.push_back(m_id);

                        int m_size = stoi(read.substr(6, 3));

                        string m_recieved = read.substr(9, m_size);

                        printf("Client : %s\n", m_recieved.data());

                        int hash_recieved = stoi(read.substr(9 + m_size, 3));

                        int calc_hash = calculateHash(m_recieved);

                        if (type_m == '1')
                        {
                                requested = true;
                                received = m_recieved;
                                received_id = m_id;
                                if (hash_recieved != calc_hash)
                                {
                                        string m = buildMessage('2', m_error, stoi(m_id));
                                        sendto(sockfd, m.data(), m.size(),
                                               MSG_CONFIRM, (const struct sockaddr *)&cliaddr,
                                               len);
                                }
                        }

                        else if (type_m == 'B')
                        {
                                received = m_recieved;
                                
                                received_id = m_id;
                                requested = true;
                                if (hash_recieved != calc_hash)
                                {
                                        string m = buildMessage('7', m_error, stoi(m_id));
                                        sendto(sockfd, m.data(), m.size(),
                                               MSG_CONFIRM, (const struct sockaddr *)&cliaddr,
                                               len);
                                }

                                /*string m = buildMessage('7', m_recieved, stoi(m_id));
                                        sendto(sockfd, m.data(), m.size(),
                                               MSG_CONFIRM, (const struct sockaddr *)&cliaddr,
                                               len);*/
                        }

                        else if (type_m == '5')
                        {
                                received = m_recieved;
                                received_id = m_id;
                                string m = buildMessage('9', "1", stoi(m_id));

                                if (hash_recieved != calc_hash)
                                {
                                        m = buildMessage('9', "0", stoi(m_id));
                                }

                                sendto(sockfd, m.data(), m.size(),
                                       MSG_CONFIRM, (const struct sockaddr *)&cliaddr,
                                       len);
                        }
                }

        }
}
// Driver code
void init()
{

        // Creating socket file descriptor
        if ((ConnectFD = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
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

        // Bind the socket with the server address
        if (bind(ConnectFD, (const struct sockaddr *)&servaddr,
                 sizeof(servaddr)) < 0)
        {
                perror("bind failed");
                exit(EXIT_FAILURE);
        }

        //len is value/resuslt

        std::thread first(read_s, ConnectFD);
        //std::thread second(write_s, sockfd);

        first.detach(); // pauses until first finishes
        //second.join();
}
