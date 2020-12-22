/* Client code in C */

#include <iostream>
#include <string>
#include "UCSPServer_RDT.h" 
#include "sha512.hh"
using namespace std;

void writteFile(string name, string _read){
  FILE *fp = NULL;

  fp = fopen(name.data() ,"w");

  fwrite (_read.data() , sizeof(unsigned char),_read.size() * sizeof(unsigned char), fp);
  fclose (fp);
}

void a_read(){
  while(1){

  

  string name="";
    string _read = getFile(&name);

    cout<<name;

    cout<<_read.size();

    writteFile(name,_read);
    cout<<"Documento guardado"<<endl;
  }

}

void a_write(){

  std::ifstream input( "image.jpg", std::ios::binary );
  std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(input), {});

  string _read(buffer.size(),'0');
  for(int i = 0;i<buffer.size();i++){
    _read[i]=buffer[i];
  }

  cout<<"input 1 to Notificate and 2 to Solicitate: ";
  int o;
  cin>>o;

  if (o==1){
  SendFileN("imageN.jpg",_read);
  }
  else{

    SendFileS("imageS.jpg",_read);

  }

}


int main(){

  init();

  
  thread t_read(a_read);
  thread t_write(a_write);

 
  t_write.detach();

   t_read.join();

 
  
  return 1;

}
