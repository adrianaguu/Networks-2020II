/* Client code in C */

#include <iostream>
#include <string>
#include "UCSPClient_RDT.h"

using namespace std;


string readFile(string name){
  FILE *in = fopen(name.data(), "r");
  string _read;
  while(1) {
    unsigned char c = fgetc(in);
    _read.push_back(c);
    if (feof(in)) break;
    //printf("%d", c);
  }
  fclose(in);
  return _read;
}



int main(){

  init();
  string _read = readFile("image.jpg");

  SendFile("i.jpg",_read);
  //Request("hola");
  return 1;

}


