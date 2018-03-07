#include <boarddefs.h>
#include <ir_Lego_PF_BitStreamEncoder.h>
#include <IRremote.h>
#include <IRremoteInt.h>int RECIEVE_PIN = 2;
int TRANSMIT_PIN = 3;
int ButtonValue;
IRsend irsend;
IRrecv irrecv(RECIEVE_PIN);
decode_results results;

void setup()
{ 
  Serial.begin(9600);
  irrecv.enableIRIn(); // Start the receiver
  Serial.print("Let's Start Revieving \n");
}

void loop() {
  if (irrecv.decode(&results) )                                            //If we recieve a value
    {
    if (results.value != 0xFFFFFFFF){                                     // 0xFFFFFFFFF represents the kept pressing the button. I removed it for better printing. For normal use, comment this and uncomment the case in the translate function
      if (results.decode_type == NEC)          {Serial.print("NEC: ");}   
      else if (results.decode_type == SONY)    {Serial.print("SONY: ");} 
      else if (results.decode_type == RC5)     {Serial.print("RC5: ");}
      else if (results.decode_type == RC6)     {Serial.print("RC6: ");}
      else if (results.decode_type == UNKNOWN) {Serial.print("UNKNOWN: ");}
      Serial.print("\n Value = "); 
      Serial.println(results.value);                                                                                                                
    }
       irrecv.resume();                                                    // Receive the next value // note dont include in the if statement of results.value != FFFF...
   }

}


int translateIR() // returns value of 

{

 switch(results.value)

  {

  case 0x71AEE7B6:              //Insert the value you get here through trial of the remote
    Serial.println("Channel -");
    return 10;  // CH-
    break;

  case 0x903E6B07:  
    Serial.println("Channel +");
    return 12; // CH+
    break;

  case 0x45481702:  
    Serial.println("Left");
    return 13; // Left
    break;

  case 0xF0B4BB43:  
    Serial.println("Right");
    return 14; // Right
    break;

  case 0xB8E7B4FE: 
    Serial.println("Middle Button"); 
    return 15; //  Middle Button    
    break;

  case 0x776C6E7A:  
    Serial.println("Volume -");
    return 16; // VOL-
    break;

  case 0x22D912BB:  
    Serial.println("Volume +");
    return 17; // VOL+ 
    break;

  case 0xD4BE4E37:
    Serial.println("Return");
    return 18; // Return
    break;

  case 0x898FDF7A: 
    Serial.println("0");
    return 0; // ZERO
    break;

  case 0x6BC6597B:  
    Serial.println("1");
    return 1;  // 1 etc.. to 9
    break;

  case 0x735B797E:  
    Serial.println("2");
    return 2; 
    break;

  case 0x1EC81DBF:  
    Serial.println("3");
    return 3; 
    break;

  case 0x450753D6:
    Serial.println("4");  
    return 4;  
    break;

  case 0xBA0F4EDF:  
    Serial.println("5");
    return 5; 
    break;

  case 0x4AC4DA9A: 
    Serial.println("6"); 
    return 6; 
    break;

  case 0xF6317EDB:  
    Serial.println("7"); 
    return 7; 
    break;

  case 0xF9000E7E:  
    Serial.println("8");
    return 8;  
    break;

  case 0xC7291B77:  
    Serial.println("9");
    return 9; // 9 
    break;


  default: 
    Serial.println("Unknown Code");
    return -1; // Other Button
  } 
} 


