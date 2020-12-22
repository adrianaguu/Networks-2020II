/* Client code in C */

#include <iostream>
#include <string>
#include "UCSPServer_RDT.h" 
#include "sha512.hh"
using namespace std;

void writteFile(string name, string _read){
  FILE *fp = NULL;

  fp = fopen(name.data() ,"a");

  fwrite (_read.data() , sizeof(unsigned char),_read.size() * sizeof(unsigned char), fp);
  fclose (fp);
}


int main(){

  init();


 
    string name="";
    string _read = getFile(&name);

    cout<<name;

    cout<<_read.size();

    writteFile(name,_read);
    cout<<"Documento guardado"<<endl;
  

  return 1;

}
