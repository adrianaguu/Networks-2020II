/* Client code in C */
#include <iostream>
#include <string>
#include "UCSPClient_RDT.h"
#include <fstream>
#include <iterator>

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

    writteFile(name,_read);
    cout<<"Documento guardado"<<endl;
  }

}

int main(){


  init();

   thread t_read(a_read);

  while(1){

    cout<<"Input the path: ";
    string path;
    cin >> path;
  std::ifstream input( path, std::ios::binary );
  std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(input), {});

  string _read(buffer.size(),'0');
  for(int i = 0;i<buffer.size();i++){
    _read[i]=buffer[i];
  }


cout<<"input 1 to SendForget and 2 to Request: ";
  int o;
  cin>>o;

  if (o==1){

  SendFileSF("imageSF.jpg",_read);
  } else
  {

    SendFileR("imageREQ.jpg",_read);
    
  }
  }
 

  t_read.join();



  
  
  return 1;

}


