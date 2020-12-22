
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
#include <map>
#include <math.h>
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

map<string, string> files;

map<string, int> count_files;

bool complete_file = false;

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

        float rest = sum / hash_cheksum - floor(sum / hash_cheksum);

        return sum * rest;
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
        return sendRDT(s, m, times);
}

void SolicitateProcces(string message, string *response)
{
        string read = "";
        read = sendRDT(ConnectFD, message, 0);

        if (read.size() > 0)
        {

                string m_id = read.substr(1, 5);

                int m_size = stoi(read.substr(6, 3));
                string m_recieved(m_size, '0'); //read.substr(9, m_size);

                for (int i = 0; i < m_size; i++)
                {
                        m_recieved[i] = buffer[i + 9];
                }

                //printf("Client sent %d : %s\n",m_recieved.size(),m_recieved.data());

                string shash_recieved(3, '0');

                for (int i = 0; i < 3; i++)
                {
                        shash_recieved[i] = buffer[m_size + 9 + i];
                }

                int hash_recieved = stoi(shash_recieved);

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

string Solicitate(string message)
{
        message = buildMessage('3', message, id);

        //id++;

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

                string m_recieved(m_size, '0'); //read.substr(9, m_size);

                for (int i = 0; i < m_size; i++)
                {
                        m_recieved[i] = buffer[i + 9];
                }

                //printf("Client sent %d : %s\n",m_recieved.size(),m_recieved.data());

                string shash_recieved(3, '0');

                for (int i = 0; i < 3; i++)
                {
                        shash_recieved[i] = buffer[m_size + 9 + i];
                }

                int hash_recieved = stoi(shash_recieved);

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

bool Notificate(string message)
{
        message = buildMessage('6', message, id);

        //id++;

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
        printf("%s", "."); //received.data());
        return received;
}

void waitFile()
{
        while (!complete_file)
        {
        }
}

string getFile(string *name)
{

        thread wait_file(waitFile);
        wait_file.join();
        *name = files.begin()->first;
        string file = files.begin()->second;
        complete_file = false;
        files.erase(files.begin()->first);
        return file;
}

void cleanReceived()
{
        received = "";
}

bool SendFileN(string name, string file)
{
        bool global_success;
        int tam_m = MAXLINE - 20 - name.size();
        int num_m = ceil((float)file.size() / (float)tam_m);
        string snum_m = intToString(num_m, 6);

        for (int i = 0; i < num_m; i++)
        {
                string message = "";
                if (i == num_m - 1)
                {
                        message = name + ":" + snum_m + ":" + file.substr(i * tam_m, file.size() - i * tam_m);
                }
                else
                        message = name + ":" + snum_m + ":" + file.substr(i * tam_m, tam_m);

                message = buildMessage('F', message, id);

                //cout<<"mess size "<<message<<endl;

                //id++;

                string response;

                bool success;

                //cout<<message;

                std::thread process(NotificateProcces, message, &response, &success);

                process.join();

                global_success = global_success and success;

                //cout<<"Response: "<< response <<endl;

                cout << i << endl;
        }

        //cout<<"Response: "<< response <<endl;

        return global_success;
}

bool SendFileS(string name, string file)
{

        int tam_m = MAXLINE - 20 - name.size();
        int num_m = ceil((float)file.size() / (float)tam_m);
        string snum_m = intToString(num_m, 6);

        for (int i = 0; i < num_m; i++)
        {
                string message = "";
                if (i == num_m - 1)
                {
                        message = name + ":" + snum_m + ":" + file.substr(i * tam_m, file.size() - i * tam_m);
                }
                else
                        message = name + ":" + snum_m + ":" + file.substr(i * tam_m, tam_m);

                message = buildMessage('T', message, id);

                //cout<<"mess size "<<message<<endl;

                //id++;

                string response;

                bool success;

                //cout<<message;

                std::thread process(SolicitateProcces, message, &response);

                process.join();

                //cout<<"Response: "<< response <<endl;

                cout << i << endl;
        }
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

                char type_m = buffer[0];

                string m_id = read.substr(1, 5);

                id = stoi(m_id) + 1;

                printf("M id : %s\n", m_id.data());

                if (!recievedBefore(m_id))
                {

                        id_last_m.push_back(m_id);

                        int m_size = stoi(read.substr(6, 3));

                        //printf("%d\n",m_size);

                        string m_recieved(m_size, '0'); //read.substr(9, m_size);

                        for (int i = 0; i < m_size; i++)
                        {
                                m_recieved[i] = buffer[i + 9];
                        }

                        //printf("Client sent %d : %s\n",m_recieved.size(),m_recieved.data());

                        string shash_recieved(3, '0');

                        for (int i = 0; i < 3; i++)
                        {
                                shash_recieved[i] = buffer[m_size + 9 + i];
                        }

                        int hash_recieved = stoi(shash_recieved);

                        int calc_hash = calculateHash(m_recieved);

                        if (type_m == '1')
                        {
                                requested = true;
                                received = m_recieved;
                                received_id = m_id;
                                string m = buildMessage('2', "", stoi(m_id));
                                if (hash_recieved != calc_hash)
                                {
                                        string m = buildMessage('2', m_error, stoi(m_id));
                                }
                                sendto(sockfd, m.data(), m.size(),
                                       MSG_CONFIRM, (const struct sockaddr *)&cliaddr,
                                       len);
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

                        else if (type_m == 'F' || type_m == 'T' || type_m == 'Y')
                        {
                                received = m_recieved;
                                received_id = m_id;

                                int i = m_recieved.find(':');

                                string name = m_recieved.substr(0, i);

                                // printf("name : %s\n", name.data());
                                string temp_file = m_recieved.substr(i + 1, m_recieved.size() - 1);
                                // printf("temp : %s\n", temp_file.data());
                                i = temp_file.find(":");

                                string snum_files = temp_file.substr(0, i);
                                int num_files = stoi(snum_files);
                                string file = temp_file.substr(i + 1, temp_file.size() - 1);

                                // printf("file : %s\n", file.data());

                                if (files.find(name) != files.end())
                                {
                                        files[name] += file;
                                        count_files[name] += 1;
                                        if (num_files == count_files[name])
                                                complete_file = true;
                                }

                                else
                                {
                                        files[name] = file;
                                        count_files[name] = 1;
                                        complete_file = false;
                                }

                                string m;

                                if (type_m != 'Y')
                                {

                                        if (hash_recieved != calc_hash)
                                        {
                                                m = buildMessage('R', "0", stoi(m_id));
                                        }

                                        else if (type_m == 'F')
                                        {

                                                m = buildMessage('R', "1", stoi(m_id));
                                        }
                                        else
                                        {

                                                m_recieved[0] = 'R';
                                                //printf("%s\n",m_recieved.data());

                                                m = buildMessage('Y', m_recieved, stoi(m_id));

                                                //printf("%d\n",m.size());
                                        }

                                        sendto(sockfd, m.data(), m.size(),
                                       MSG_CONFIRM, (const struct sockaddr *)&cliaddr,
                                       len);
                                }

                                
                        }
                }
                else
                {
                        cout << "duplicate data" << endl;
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
