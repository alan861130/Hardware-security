#include <iostream>

using namespace std;

#define ROTL8(x,shift) ((uint8_t) ((x) << (shift)) | ((x) >> (8 - (shift))))

uint8_t sbox[256];



void initialize_aes_sbox(void) {
	/* loop invariant: p * q == 1 in the Galois field */
	uint8_t p = 1, q = 1;
	int round = 0;
	do {
		//cout<<round<<endl;
		
		/* multiply p by x+1 */
		p = p ^ (p << 1) ^ (p & 0x80 ? 0x1B : 0);
		/* divide q by x+1 */
		q ^= q << 1;
		q ^= q << 2;
		q ^= q << 4;
		q ^= q & 0x80 ? 0x09 : 0;
		/* compute the affine transformation */
		sbox[p] = 0x63 ^ q ^ ROTL8(q, 1) ^ ROTL8(q, 2) ^ ROTL8(q, 3) ^ ROTL8(q, 4);
		round ++;
		
		//cout<<"P = "<<hex<<(uint16_t)p<<endl;
		//cout<<"Q = "<<hex<<(uint16_t)q<<endl;
	} while (p != 1);

	/* 0 is a special case since it has no inverse */
	sbox[0] = 0x63;
}

int main(){
    initialize_aes_sbox();

    for(int i = 0 ; i < 16 ; i++){
		for(int j = 0 ; j < 16 ; j++){
			cout<<"0x";
			if(sbox[i*8 + j] < 16){
				cout<<0;
			}
			cout<<hex<<(uint16_t)sbox[i*8 + j]<<" ";
		}
		cout<<endl;
        
    }
    return 0;
}
