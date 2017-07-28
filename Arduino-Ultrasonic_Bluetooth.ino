
#include <SoftwareSerial.h>
#include <SPI.h>                             // Serial Peripheral Interface Library
#include <String.h>                          // contains function strtok: split string into tokens

// defines pins numbers
const int trigPin = 8;
const int echoPin = 9;
const int Rx = 10;
const int Tx = 11;

// defines variables
byte size;
long duration;
int distance;
int INPUT_SIZE = 30;

// Others
SoftwareSerial Bluetooth (Rx,Tx); 

//Main Functions

void setup() {
  pinMode(Rx, INPUT);
  pinMode(Tx, OUTPUT);
  Bluetooth.begin(9600);
  Bluetooth.setTimeout(1000);                  // 1000 ms time out
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  Serial.begin(9600); // Starts the serial communication
  Bluetooth.flush();
  Bluetooth.println ("Hello , please send \"Distance\" to veiw the distance! Thank you ");
}

void loop() {
  bluetooth_func();
  Serial.println(find_distance());
}

//Minor functions
void bluetooth_func()
{
  if (Bluetooth.available()>0)
  {
   char input[INPUT_SIZE+1];
   size = Bluetooth.readBytesUntil('\n', input, INPUT_SIZE );
   input[size] = '\0';
   Bluetooth.print("You have inserted [");
   for (int i=0;i<size;i++)
   {Bluetooth.print(input[i]);}
    Bluetooth.println("]");
    compare(input);}
    delayMicroseconds(500);
  }

  
void compare(char* A)
//if you want to implement a char array comparison .Here i did a string comparison.
{/*           
delayMicroseconds(2);
//String comparator= String (A);
if (strcasecmp(A,"Distance")==0)
{
  Bluetooth.print("The distance between the sensor and the object is ");
  Bluetooth.println(find_distance());}
  else if (strcasecmp(A,"Marwan")==0)
  Bluetooth.println("He's the best person on earth");
  else 
  Bluetooth.println("Invalid text . Please try again!");
  */
String comparator= String (A);
if (comparator.equalsIgnoreCase("Distance"))
{Bluetooth.print("The distance between the sensor and the object is ");
 Bluetooth.println(find_distance());}
 else if (comparator.equalsIgnoreCase("name"))
Bluetooth.println("He's the best person on earth");
else 
Bluetooth.println("Invalid text . Please try again!");
  }
  
int find_distance()
{
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  // Calculating the distance
  distance= duration*0.034/2;
  return distance;
  }
