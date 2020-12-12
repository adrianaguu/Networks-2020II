#include <iostream>
#include <string>


using namespace std;

string word = "universidad";
string w(word.size(),'_');
int missing = word.size();
int lives=10;

bool insertInW(char c){
    bool correct = false;
  
    for (int i=0;i<word.size();i++){
        if(word[i]==c){
            w[i]=c;
            missing--;
            correct=true;
        }
    }
    return correct;
}

int main(){
    cout<<w;
    while(lives!=0 && w!=word){
        char c;
        cin >> c;
        if(!insertInW(c)){
            lives--;
            cout<<"No, try again"<<endl;
        }
        cout<<"("<<missing<<") "<<"["<<w<<"]"<<endl;           
    }

    if(w==word){
        cout<<"GANASTE";
        return 1;
    }

    cout << "Te quedaste sin vidas";
    return 0;
}
