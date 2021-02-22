//Send to testchip
const byte modePin = 2;         //Set PUF/TRNG mode
const byte requestPin = 4;      //User request for a random number

//Receive from testchip         
const byte getReadyPin = 3;     //Whether data is ready for user to request
const byte getInterruptPin = 5; //Whether user can catch a random number on Data pins

//8-bit data

const byte dataPin_0 = 6;  
const byte dataPin_1 = 7;  
const byte dataPin_2 = 8;  
const byte dataPin_3 = 9;  
const byte dataPin_4 = 10;  
const byte dataPin_5 = 11;  
const byte dataPin_6 = 12;  
const byte dataPin_7 = 13;  

//variables to store read value
int val_ready = 0;
int val_interrupt = 0;

//stored data
int PUF_data[4096];
int TRNG_data[8];

//flag for whether the chip has read PUF data
int PUF_flag = 0; // 0 for hasn't read , 1 for has read 

//=====================================================================================
//AES part
//Use PUF_data[0]~PUF_data[127] as plaintext
//Use PUF_data[128]~PUF_data[255] as key

#define xtime(x)   ((x << 1) ^ (((x >> 7) & 0x01) * 0x1b))
#define Nb 4

int KeySize = 128;
int binary_plaintext[128]; //plaintext for AES enc
int binary_key[128];  //key for AES enc

unsigned char in[16];          // plaintext block input array, 明文區塊輸入char陣列
unsigned char out[16];         // ciphertext block output array, 密文區塊輸出陣列
unsigned char state[4][4];     // temp state array in encrypt state, 加密運算過程中的的狀態陣列 4 * 4 
unsigned char Roundkey[240];   // round key array, stored Main Key and Expanded Key (Ex: AES-128(44words/176 bytes), AES-256(60w/260bytes)), 儲存主要鑰匙跟擴充鑰匙的陣列, w0(index 0 ~ 3) w1(index 4 ~ 7)....
unsigned char Key[16];         // Main key(input key Ex. AES-128(18 char), AES-256(32 char)), 輸入的金鑰

int Nr = 0; /* Number of round(Nr), 加密運算執行回合數, AES-128(10r), AES-192(12), AES-256(14)*/
int Nb_k = 0;  /* Number of block of key, 鑰匙(每block-32bits)的block數量 AES-128(4 block), AES-192(6), AES-256(8) */

/* S-box */
int S_Box[256] =   
{
    //0     1    2      3     4    5     6     7      8    9     A      B    C     D     E     F
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76, //0
    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0, //1
    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15, //2
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75, //3
    0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84, //4
    0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf, //5
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8, //6
    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2, //7
    0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73, //8
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb, //9
    0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79, //A
    0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08, //B
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a, //C
    0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e, //D
    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf, //E
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16  //F
};

int Rcon[11] = 
{
//   0     1     2     3      4    5     6     7     8    9     10
    0x87, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36
};

//====================================================================================


void setup() {
  // put your setup code here, to run once:
    Serial.begin(9600);
    
    //Serial.println ("Set up pins...");
    pinMode(modePin , OUTPUT);
    pinMode(requestPin , OUTPUT);
  
    pinMode(getReadyPin , INPUT_PULLUP);
    pinMode(getInterruptPin, INPUT);


    pinMode(dataPin_0 , INPUT);
    pinMode(dataPin_1 , INPUT);
    pinMode(dataPin_2 , INPUT);
    pinMode(dataPin_3 , INPUT);
    pinMode(dataPin_4 , INPUT);
    pinMode(dataPin_5 , INPUT);
    pinMode(dataPin_6 , INPUT);
    pinMode(dataPin_7 , INPUT);

}

void loop() {
  
  
  if( Serial.available()){

      Serial.println ("Select mode : ");

      String input = Serial.readString();
      Serial.println(input);

    if( input == "PUF"){
        
        Serial.println ("Start reading PUF data...");

        digitalWrite(modePin , LOW);  // set Mode Pin to low
        Serial.println ("1) Set Mode pin Low ");
        
        
        for(int i = 0 ; i < 128 ; i++){

            for(int j = 0 ; j < 4 ; j++){
                //Serial.println(i*4 + j);
                read_PUF( i*4 + j);  //reading PUF data from the testchip
            }
            Serial.println("");
            

        }

        Serial.println ("Finish reading PUF data...");
        Serial.println("");
        PUF_flag = 1; // set flag to 1
        
    }

    else if(input == "TRNG"){
        
        Serial.println ("Start reading TRNG data...");

        digitalWrite(modePin , HIGH);  // set Mode Pin to high
        Serial.println ("1) Set Mode pin high ");
        
        
        read_TRNG();  //reading TRNG data from the testchip

        for(int i = 0 ; i < 8 ; i++){
          Serial.print(TRNG_data[i]);
        }
        Serial.println("");
        
        Serial.println ("Finish reading TRNG data...");
        Serial.println("");
    }

    else if (input == "AES"){
        
        if(PUF_flag == 0){
          Serial.println ("You need to read PUF data first...");
        }
        else{
          AES();
        }

    }

  }
  
}

void read_PUF(int round){

  
    val_ready = digitalRead( getReadyPin );
    if(val_ready == HIGH){
        //Serial.println ("2) Ready goes high ");
        
        digitalWrite(requestPin , HIGH);
        //Serial.println ("3) Set request high ");
        
        delay(5);
        
        //Serial.println ("4) Read PUF data ");
        PUF_data[round*8] = digitalRead(dataPin_0);
        PUF_data[round*8 + 1] = digitalRead(dataPin_1);
        PUF_data[round*8 + 2] = digitalRead(dataPin_2);
        PUF_data[round*8 + 3] = digitalRead(dataPin_3);
        PUF_data[round*8 + 4] = digitalRead(dataPin_4);
        PUF_data[round*8 + 5] = digitalRead(dataPin_5);
        PUF_data[round*8 + 6] = digitalRead(dataPin_6);
        PUF_data[round*8 + 7] = digitalRead(dataPin_7);
        
        
    }
    
     //for(int i = 7 ; i >=0 ; i--){
     //    Serial.print(PUF_data[round*8 + i]);
     //}
     //Serial.println("");

     binary_converter_PUF(round);
     
    //Serial.println ("");
    digitalWrite(requestPin , LOW);
    //Serial.println ("6) Reset request ");
    

}

void binary_converter_PUF(int round){

    int data_in_hex;
    
    data_in_hex = PUF_data[round*8] 
                + 2 * PUF_data[round*8 + 1] 
                + 4 * PUF_data[round*8 + 2] 
                + 8 * PUF_data[round*8 + 3]
                +16 * PUF_data[round*8 + 4]
                +32 * PUF_data[round*8 + 5]
                +64 * PUF_data[round*8 + 6]
                +128 * PUF_data[round*8 + 7];
    
    if(data_in_hex < 16){
        Serial.print(0);  
    }
    Serial.print(data_in_hex , HEX);
                
}

void read_TRNG(void){

  val_ready = digitalRead( getReadyPin );
    
    if(val_ready == 1){
        //Serial.println ("2) Ready goes high ");
        
        digitalWrite(requestPin , HIGH);
        //Serial.println ("3) Set request high ");
        delay(20);
        
    }
    
    //Serial.println ("5) Read TRNG data ");
     TRNG_data[0] = digitalRead(dataPin_0);
     TRNG_data[1] = digitalRead(dataPin_1);
     TRNG_data[2] = digitalRead(dataPin_2);
     TRNG_data[3] = digitalRead(dataPin_3);
     TRNG_data[4] = digitalRead(dataPin_4);
     TRNG_data[5] = digitalRead(dataPin_5);
     TRNG_data[6] = digitalRead(dataPin_6);
     TRNG_data[7] = digitalRead(dataPin_7);
     
     
    
     
    //Serial.println ("");
    digitalWrite(requestPin , LOW);
    //Serial.println ("6) Reset request ");
  
}

void AES(void){
  
  Serial.println("*** AES encryption System ***");

  Nb_k = KeySize / 32;     // Number of block of key, 計算key block數量 (Ex: AES-128 : 4) 
  Nr   = Nb_k + 6;         // Number of round(Nr),  計算AES 運算回合次數 (Ex:AES-128 : 10)

  
  gen_plaintext(); //get plaintext from random number generator

  for(int i = 0 ; i < 128 ; i++){
      //write data in plaintext and key
      binary_key[i] = PUF_data[i];
  }

  Serial.println("Plaintext in binary: ");
  for(int i = 0 ; i < 128 ; i++){
       Serial.print(binary_plaintext[i]);
  }
  Serial.println("");
  
  Serial.println("key in binary: ");
  for(int i = 0 ; i < 128 ; i++){
       Serial.print(binary_key[i]);
  }
  Serial.println("");

  binary_converter_AES(); // convert binary_plaintext and key into hex

  
  Serial.println("Plaintext in hex: ");
  for(int i = 0 ; i < 16 ; i++){
       if(in[i] < 16){
           Serial.print(0);   
       }
       Serial.print(in[i] , HEX);
  }
  Serial.println("");
  
  Serial.println("key in hex: ");
  for(int i = 0 ; i < 16 ; i++){
       if(Key[i] < 16){
           Serial.print(0);   
       }
       Serial.print(Key[i] , HEX);
  }
  Serial.println("");

  KeyExpansion();
  
  Cipher();

  Serial.println("out : ");
  for(int i = 0 ; i < 16 ; i++){
       if(out[i] < 16){
           Serial.print(0);   
       }
       Serial.print(out[i] , HEX);
  }
  Serial.println("");

  
}

void gen_plaintext(void){

    for(int i = 0 ; i < 128 ; i++){
        int k = int(random(dataPin_0)) % 2;
        //Serial.println(k);
        binary_plaintext[i] = k;
    }

}

void binary_converter_AES(void){

    for (int i=0; i<16; ++i) {
        int tmp = 0;
        for (int j=0; j<8; ++j) {
            int idx = i*8+j; //第idx位數
            tmp += (binary_plaintext[idx])<<(7-j);
        }
        in[i] = tmp;
    }

    for (int i=0; i<16; ++i) {
        int tmp = 0;
        for (int j=0; j<8; ++j) {
            int idx = i*8+j; //第idx位數
            tmp += (binary_key[idx])<<(7-j);
        }
        Key[i] = tmp;
    }

}

void KeyExpansion(){
    unsigned char tempByte[4]; // store 4 temp Byte(1 word) when generate subkey
    unsigned char a0;       // temp - store byte when execute RotWord function
    
    /**
     * First Round subKey = Main/Input Key, Divide to {Nb_k} block (each 32bits) [w0, w1, w2, w3]
     * each block divide to 4 subblock(8bit)
     * Ex: AES-128, Nb_k = 4, 4 block W0 ~ W3
     * Ex: AES-256, Nb_k = 8, 8 block W0 ~ W7
     * 
     * 第一回合子鑰匙 = 主鑰匙分成四個(Nb=4) 8 位元區塊 (W0=32 bits)
     * 第一回合需要 Nk 個鑰匙區塊, AES-128:Nk=4, W0 ~ W3 
     * 一個小block為8 bit = 1 character
     */
    for (int i = 0;i < Nb_k;i++){
        Roundkey[i * 4] = Key[i * 4];
        Roundkey[i *4 + 1] = Key[i * 4 + 1];
        Roundkey[i *4 + 2] = Key[i * 4 + 2];
        Roundkey[i *4 + 3] = Key[i * 4 + 3];
    }


    /**
     * Generate other subkey, 
     * 產生其他回合鑰匙: 
     * Ex: AES-128: i= 4 ~ 43, 共 11 個 4block(128bit), 需 44 個word (W0 ~ W43).
     * Ex: AES-256: i = 8 ~ 59, 共需要 15個 4block(128bit), 需60word(W0~ W59)
     * 每跑完一次產生一個block
     */
    for (int i = Nb_k;i < (Nb * (Nr + 1));i++)
    {
        for (int j = 0;j < 4;j++){ // 處理每個block(W)
            tempByte[j] = Roundkey[(i - 1) * 4 + j]; // 要新增一個block(Word)故取前一個的W值存入tempW
        }
        if (i % Nb_k == 0){
            /**
             * Ex: AES-128 when generate W4, will use W3 do SubWord(RotWord(tempW)) XOR Rcon[4/4]
             *     AES-128 i 是 4 的倍數的 Wi 用 Wi-1產生 Wi =  SubWord(RotWord(Wi-1)) XOR Rcon[i/4]
             */

            // RotWord function, [a0, a1, a2, a3](4byte) left circular shift in a word [a1, a2, a3, a0]
            a0 = tempByte[0];
            tempByte[0] = tempByte[1];
            tempByte[1] = tempByte[2];
            tempByte[2] = tempByte[3];
            tempByte[3] = a0;

            // SubWord function (S-Box substitution)
            tempByte[0] = S_Box[(int)tempByte[0]];
            tempByte[1] = S_Box[(int)tempByte[1]];
            tempByte[2] = S_Box[(int)tempByte[2]];
            tempByte[3] = S_Box[(int)tempByte[3]];
            
            // XOR Rcon[i/4], only leftmost byte are changed (只會XOR最左的byte)
            tempByte[0] = tempByte[0] ^ Rcon[i / Nb_k]; 
        }
        else if (Nb_k == 8 && i % Nb_k == 4){
            // Only AES-256 used, 僅 AES-256 使用此規則, 
            // 當 i mod 4 = 0 且 i mod 8 ≠ 0 時，Wn = SubWord (Wn−1) XOR Wn−8
            tempByte[0] = S_Box[(int)tempByte[0]];
            tempByte[1] = S_Box[(int)tempByte[1]];
            tempByte[2] = S_Box[(int)tempByte[2]];
            tempByte[3] = S_Box[(int)tempByte[3]];
        }
        /**
         * Wn = Wn-1 XOR Wk    k = current word - Nb_k
         * Ex: AES-128   Nb_k = 4  when W5 = Wn-1(W4) XOR Wk(W1)
         * Ex: AES-256   Nb_k = 8  when W10 = Wn-1(W9) XOR Wk(W2) 
         */
        Roundkey[i * 4 + 0] = Roundkey[(i - Nb_k) * 4 + 0] ^ tempByte[0];
        Roundkey[i * 4 + 1] = Roundkey[(i - Nb_k) * 4 + 1] ^ tempByte[1];
        Roundkey[i * 4 + 2] = Roundkey[(i - Nb_k) * 4 + 2] ^ tempByte[2];
        Roundkey[i * 4 + 3] = Roundkey[(i - Nb_k) * 4 + 3] ^ tempByte[3];   
    }

    
}

void Cipher(void){
    int round = 0;
    
    /**
     *  將in[](plaintext) 轉換成 column 排列方式
     *  圖示:
     *  [b0 b1 ... b15] -> [b0 b4 b8  b12
     *                      b1 b5 b9  b13
     *                      b2 b6 b10 b14
     *                      b3 b7 b11 b15]
     */
    for (int i = 0;i < 4;i++)
        for (int j = 0;j < 4;j++)
            state[j][i] = in[i * 4 + j]; // transform input(plaintext), 將plaintext 轉成 column形式(w0, w1, w2, w3)
    

    // round 0 : add round key, 第0回合: 僅執行-key XOR block - key使用[w0 ~ w3]
    AddRoundKey(0);

    Serial.println(round);
    for(int j = 0 ; j < 4 ; j++){

      if(Roundkey[round*4 + j] < 16){
        Serial.print(0);
      }
      Serial.print(Roundkey[round * 4 + j] , HEX);
    }
    Serial.println("");
    

    // Round 1 ~ Nr-1, 反覆執行 1 ~ Nr-1回合
    for (round = 1;round < Nr;round++){
        SubBytes();
        ShiftRows();
        MixColumns();

        Serial.println(round);
        for(int j = 0 ; j < 4 ; j++){

          if(Roundkey[round*4 + j] < 16){
            Serial.print(0);
          }
          Serial.print(Roundkey[round * 4 + j],HEX);
        }
        Serial.println("");

        AddRoundKey(round);
    }

    // Round Nr, no MixColumns(), 第 Nr 回合 沒有混合行運算
    SubBytes();
    ShiftRows();
    AddRoundKey(Nr);

    /**
     *  將state[] transform 到 out[]上
     *  圖示:
     *   [c0 c4 c8  c12
     *    c1 c5 c9  c13    --> [c0 c1 c2 ... c15]
     *    c2 c6 c10 c14
     *    c3 c7 c11 c15]
     */
    for(int i = 0;i < 4;i++) 
        for(int j = 0;j < 4;j++)
            out[i * 4 + j]=state[j][i];
}

void AddRoundKey(int round){
    /**
     * 根據round來使用key(每次用1個block = 16byte)
     * first key index = round * 16 bytes = round * Nb * 4;
     * Nb = 4
     */
    for (int i = 0;i < 4;i++)
        for (int j = 0;j < 4;j++)
            state[j][i] ^= Roundkey[(i * Nb + j) + (round * Nb * 4)]; 
}

void SubBytes(void){
    for (int i = 0;i < 4;i++)
        for (int j = 0;j < 4;j++)
            state[i][j] = S_Box[state[i][j]];
}

void ShiftRows(void){
    unsigned char tempByte;
    
    // 2nd row left Circular Shift 1 byte
    tempByte    = state[1][0];
    state[1][0] = state[1][1];
    state[1][1] = state[1][2];
    state[1][2] = state[1][3];
    state[1][3] = tempByte;

    // 3th row left Circular Shift 2 byte
    tempByte    = state[2][0];
    state[2][0] = state[2][2];
    state[2][2] = tempByte;

    tempByte    = state[2][1];
    state[2][1] = state[2][3];
    state[2][3] = tempByte;

    // 4th row left Circular Shift 3 byte
    tempByte    = state[3][0];
    state[3][0] = state[3][3];
    state[3][3] = state[3][2];
    state[3][2] = state[3][1];
    state[3][1] = tempByte;
}

void MixColumns(void){
    unsigned char Tmp,Tm,t;
    for(int i = 0;i < 4;i++)
    {    
        t   = state[0][i];
        Tmp = state[0][i] ^ state[1][i] ^ state[2][i] ^ state[3][i];
        Tm  = state[0][i] ^ state[1][i]; Tm = xtime(Tm); state[0][i] ^= Tm ^ Tmp ;
        Tm  = state[1][i] ^ state[2][i]; Tm = xtime(Tm); state[1][i] ^= Tm ^ Tmp ;
        Tm  = state[2][i] ^ state[3][i]; Tm = xtime(Tm); state[2][i] ^= Tm ^ Tmp ;
        Tm  = state[3][i] ^ t;           Tm = xtime(Tm); state[3][i] ^= Tm ^ Tmp ;
    }
}
