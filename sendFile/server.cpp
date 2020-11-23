/* Client code in C */

#include <iostream>
#include <string>
#include "UCSPServer_RDT.h" 
using namespace std;

vector<unsigned char> writteFile(string name, string _read){
  FILE *fp = NULL;
  fp = fopen(name.data() ,"a");
  fwrite (_read.data() , sizeof(unsigned char),_read.size() * sizeof(unsigned char), fp);
  fclose (fp);
}


int main(){

  init();

  while(1){
    string name;
    string _read = getFile(&name);
    writteFile(name,_read);
    cout<<"Documento guardado"<<endl;
  }

  return 1;

}
