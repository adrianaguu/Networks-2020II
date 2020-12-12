/* Client code in C */

#include <iostream>
#include <string>
#include "UCSPClient_RDT.h"

using namespace std;

string w;
int missing;
int lives;

void updateVars(string response)
{

  int i = response.find(":");
  lives = stoi(response.substr(0, i));
  string temp = response.substr(i + 1, response.size() - 1);

  i = temp.find(":");

  missing = stoi(temp.substr(0, i));
  w = temp.substr(i + 1, temp.size() - 1);
}

int main()
{
  init();

  string response = Request("start");

  //cout << response;

  updateVars(response);

  cout << endl
       << "lives: " << lives << ", (" << missing << ") [" << w << "]" << endl;

  while (lives != 0)
  {
    string c;
    cin >> c;
    response = RequestData(c);
    
 
    updateVars(response);
    cout << endl
         << "lives: " << lives << ", (" << missing << ") [" << w << "]" << endl;
  }

  cout << "Te quedaste sin vidas";
  return 0;

  return 0;
}
