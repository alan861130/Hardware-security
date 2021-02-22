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

int num_of_bits;

//flag for whether the chip has read PUF data
int PUF_flag = 0; // 0 for hasn't read , 1 for has read 

//=====================================================================================


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

      Serial.println ("The size of TRNG ");

      String input = Serial.readString();
      //Serial.println(input);

    if(input == "8"){
        
        Serial.println ("Start reading TRNG data...");

        digitalWrite(modePin , HIGH);  // set Mode Pin to high
        Serial.println ("1) Set Mode pin high ");
        
        for(int i = 0 ; i < 1 ; i ++){
            read_TRNG();  //reading TRNG data from the testchip
        }
        
        Serial.println("");
        Serial.println ("Finish reading TRNG data...");
        Serial.println("");
    }

    else if(input == "1000000"){
        
        for(int i = 0 ; i < 5 ; i++){
            Serial.println ("Start reading TRNG data...");

            digitalWrite(modePin , HIGH);  // set Mode Pin to high
            Serial.println ("1) Set Mode pin high ");
            
            for(int j = 0 ; j < 8000/8 + 1 ; j ++){
                read_TRNG();  //reading TRNG data from the testchip
            }
            
            Serial.println("");
            Serial.println ("Finish reading TRNG data...");
            Serial.println("");
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
     //TRNG_data[0] = digitalRead(dataPin_0);
     //TRNG_data[1] = digitalRead(dataPin_1);
     //TRNG_data[2] = digitalRead(dataPin_2);
     //TRNG_data[3] = digitalRead(dataPin_3);
     //TRNG_data[4] = digitalRead(dataPin_4);
     //TRNG_data[5] = digitalRead(dataPin_5);
     //TRNG_data[6] = digitalRead(dataPin_6);
     //TRNG_data[7] = digitalRead(dataPin_7);

     Serial.print(digitalRead(dataPin_0));
     Serial.print(digitalRead(dataPin_1));
     Serial.print(digitalRead(dataPin_2));
     Serial.print(digitalRead(dataPin_3));
     Serial.print(digitalRead(dataPin_4));
     Serial.print(digitalRead(dataPin_5));
     Serial.print(digitalRead(dataPin_6));
     Serial.print(digitalRead(dataPin_7));
     
     
    
     
    //Serial.println ("");
    digitalWrite(requestPin , LOW);
    //Serial.println ("6) Reset request ");
  
}
