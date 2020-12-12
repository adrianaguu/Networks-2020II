/* Client code in C */

#include <iostream>
#include <string>
#include "UCSPServer_RDT.h"
using namespace std;

string word = "universidad";
string w(word.size(), '_');
int missing = word.size();
int lives = 10;

bool insertInW(char c)
{
  bool correct = false;

   cout<<"c"<<c<<endl; 

  for (int i = 0; i < word.size(); i++)
  {
    if (word[i] == c)
    {
      w[i] = c;
      missing--;
      correct = true;
    }
  }
  return correct;
}

int main()
{
  init();

  string c;

    string slives = to_string(lives);
    string smissing = to_string(missing);
    while(!ResponseData(slives + ":" + smissing + ":" + w));
    cleanReceived();

  while (lives != 0 && w != word)
  {
  

    c = getReceived();

   

    if (c.size()>0 && !insertInW(c[0]))
    {
      lives--;
      cout << "No, try again" << endl;
    }

    string slives = to_string(lives);
    string smissing = to_string(missing);

    ResponseData(slives + ":" + smissing + ":" + w);
    cleanReceived();
    //cout << "(" << missing << ") "<< "[" << w << "]" << endl;
  }

  if (w == word)
  {
    ResponseData("GANASTE");
    return 1;
  }

  cout << "Te quedaste sin vidas";
  return 0;
  
}
