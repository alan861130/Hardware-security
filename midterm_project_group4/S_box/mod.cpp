#include <iostream>

using namespace std;

int main(){

    cout<<"Calculate (x+1)^255 mod x^8 ..."<<endl;

    uint8_t p = 1;
    for(int i = 0 ; i < 256 ; i++){

        p = p ^ (p << 1) ^ (p & 0x80 ? 0x1B : 0);
        
        cout<<"(x+1)^"<<i+1<<" mod x^8 = ";
        cout<<hex<<(uint16_t)p<<endl;
        
    }
    //cout<< k << endl;
}
