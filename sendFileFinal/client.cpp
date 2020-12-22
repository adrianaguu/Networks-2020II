/* Client code in C */

#include <iostream>
#include <string>
#include "UCSPClient_RDT.h"

using namespace std;






int main(){


  init();
  std::ifstream input( "image.jpg", std::ios::binary );
  std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(input), {});

  cout<<buffer.size();
  string _read(buffer.size(),'0');
  for(int i = 0;i<buffer.size();i++){
    _read[i]=buffer[i];
  }


  SendFile("image2.jpg",_read);
  
  return 1;

}


