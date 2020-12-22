/* Client code in C */

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <iterator>
#include <algorithm>

using namespace std;


vector<unsigned char> readFile(string name){
  FILE *in = fopen(name.data(), "r");
  vector<unsigned char> _read;
  while(1) {
    unsigned char c = fgetc(in);
    _read.push_back(c);
    if (feof(in)) break;
    //printf("%d", c);
  }
  fclose(in);
  cout<<_read.size();
  return _read;
}

void writeFile(string name, vector<unsigned char> _read){
  FILE *fp = NULL;

  fp = fopen(name.data() ,"a");
  cout<<endl<<_read.size();

  fwrite (_read.data() , sizeof(unsigned char),_read.size() * sizeof(unsigned char), fp);
  fclose (fp);
}



int main(){


 
  std::ifstream input( "image.jpg", std::ios::binary );
  std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(input), {});
  writeFile("u1.jpg",buffer);
  
  return 1;

}


