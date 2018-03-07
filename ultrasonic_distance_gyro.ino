#include <helper_3dmath.h>
#include <MPU6050_6Axis_MotionApps20.h>


#include "I2Cdev.h"
#include <SoftwareSerial.h>
#include <SPI.h>                             // Serial Peripheral Interface Library
#include <String.h>                          // contains function strtok: split string into tokens
#include<Wire.h>

// defines pins numbers
const int MPU_addr=0x68; //Default adress for MPU6050
#define INTERRUPT_PIN 2
const int trigPin = 8;
const int echoPin = 9;
const int Rx = 10; // Recieve
const int Tx = 11; // Transmit

// defines variables
byte size;
long duration;
int distance;
int INPUT_SIZE = 30;
static enum GYRO_OUTPUT {Tempst,EULER_A,QUATERNION,YAWPITCHROLL,REALACCEL,WORLDACCEL,OUTPUTTEAPOT} stat_gyro;
int temp_raw,tempreal;

//MPU Status/Control Vars
bool dmpReady = false;  // set true if DMP init was successful
bool ActionState = false;
byte mpuIntStatus, devStatus; //holds actual interrupt status byte from MPU|return status after each device operation (0 = success, !0 = error)
byte fifoBuffer[64]; // FIFO storage buffe
int packetSize,fifoCount;    // expected DMP packet size (default is 42 bytes)|count of all bytes currently in FIFO

//MPU Others
MPU6050 gyro;
byte teapotPacket[14] = { '$', 0x02, 0,0, 0,0, 0,0, 0,0, 0x00, 0x00, '\r', '\n' };

// orientation/motion vars
Quaternion q;           // [w, x, y, z]         quaternion container
VectorInt16 aa;         // [x, y, z]            accel sensor measurements
VectorInt16 aaReal;     // [x, y, z]            gravity-free accel sensor measurements
VectorInt16 aaWorld;    // [x, y, z]            world-frame accel sensor measurements
VectorFloat gravity;    // [x, y, z]            gravity vector
float euler[3];         // [psi, theta, phi]    Euler angle container
float ypr[3];           // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector

// Others
SoftwareSerial Bluetooth (Rx,Tx); 
void compare(char* A);



// ================================================================
// ===               INTERRUPT DETECTION ROUTINE                ===
// ================================================================

volatile bool mpuInterrupt = false;     // indicates whether MPU interrupt pin has gone high
void dmpDataReady() {
    mpuInterrupt = true;
}

// ================================================================
// ===               Setup Functions                             ===
// ================================================================

void setup() {

Serial.begin(9600); // Starts the serial communication
setup_bluetooth();
setup_g521();
}

void setup_g521()
{ pinMode(INTERRUPT_PIN, INPUT);
  Wire.begin();
  Wire.setClock(400000);
  gyro.initialize();
  Bluetooth.println(gyro.testConnection() ? F("MPU6050 connection successful") : F("MPU6050 connection failed"));
  devStatus = gyro.dmpInitialize();
  gyro.setXGyroOffset(220);
  gyro.setYGyroOffset(76);
  gyro.setZGyroOffset(-85);
  gyro.setZAccelOffset(1788);
    if (devStatus == 0) {  
        gyro.setDMPEnabled(true);
        attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), dmpDataReady, RISING);
        mpuIntStatus = gyro.getIntStatus();
        dmpReady = true;
        packetSize = gyro.dmpGetFIFOPacketSize();
    }
    else
    { 
       Bluetooth.print(F("DMP Initialization failed (code "));
       Bluetooth.print(devStatus);
       Bluetooth.println(F(")"));
      }

  }
void setup_bluetooth()
{
pinMode(Rx, INPUT);
pinMode(Tx, OUTPUT);
Bluetooth.begin(9600);
Bluetooth.setTimeout(1000);                  // 1000 ms time out
Bluetooth.flush();

}
// ================================================================
// ===               Loop Functions                             ===
// ================================================================
  
void loop() 
{
bluetooth_func();
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
   {
    Bluetooth.print(input[i]);}
   Bluetooth.println("]");
   compare(input);
  }
    delayMicroseconds(2000);
  }



//WORLDACCEL,OUTPUTTEAPOT
  
void compare(char* A)
{
String comparator= String (A);

else if (comparator.equalsIgnoreCase("YPR")){
stat_gyro = YAWPITCHROLL ;
gyro_repeated();
}
else if (comparator.equalsIgnoreCase("EULER")){
stat_gyro = EULER_A ;
gyro_repeated();
}
else if (comparator.equalsIgnoreCase("WXYZ")){   // display quaternion values in easy matrix form: w x y z
stat_gyro = QUATERNION ;
gyro_repeated();
}
else if (comparator.equalsIgnoreCase("Acceleration")){
stat_gyro = REALACCEL ;
gyro_repeated();
}
else if (comparator.equalsIgnoreCase("World Acceleration")){              // display initial world-frame acceleration, adjusted to remove gravity
                                                                      // and rotated based on known orientation from quaternion
stat_gyro = WORLDACCEL ;
gyro_repeated();
}
else if (comparator.equalsIgnoreCase("temp")){    
stat_gyro = Tempst ;
gyro_repeated() ;
}
else if (comparator.equalsIgnoreCase("Marwan"))
Bluetooth.println("He's the best person on earth");
else 
{Bluetooth.println("Invalid text . Please try again!");
defaulttext();}
}





void gyro_repeated() /// to be put inside the big function later
{
 for (int i=0;i<100;i++)
{delayMicroseconds(100);
gyro_accelero_temp();
delayMicroseconds(100);}
defaulttext();
  }


void gyro_accelero_temp()
{
  // if programming failed, don't try to do anything
    if (!dmpReady) return;
    
    // reset interrupt flag and get INT_STATUS byte
    mpuInterrupt = false;
    mpuIntStatus = gyro.getIntStatus();

    // get current FIFO count
    fifoCount = gyro.getFIFOCount();

    // check for overflow (this should never happen unless our code is too inefficient)
   if ((mpuIntStatus & 0x10) || fifoCount == 1024) {
        // reset so we can continue cleanly
        gyro.resetFIFO();
   //     Bluetooth.println(F("FIFO overflow!"));

    // otherwise, check for DMP data ready interrupt (this should happen frequently)
    } else if (mpuIntStatus & 0x02) {
        // wait for correct available data length, should be a VERY short wait
        while (fifoCount < packetSize) fifoCount = gyro.getFIFOCount();

        // read a packet from FIFO
        gyro.getFIFOBytes(fifoBuffer, packetSize);
        
        // track FIFO count here in case there is > 1 packet available
        // (this lets us immediately read more without waiting for an interrupt)
        fifoCount -= packetSize;
switch (stat_gyro){
        case QUATERNION:
            // display quaternion values in easy matrix form: w x y z
            {
            gyro.dmpGetQuaternion(&q, fifoBuffer);
            Bluetooth.print("quat\t");
            Bluetooth.print(q.w);
            Bluetooth.print("\t");
            Bluetooth.print(q.x);
            Bluetooth.print("\t");
            Bluetooth.print(q.y);
            Bluetooth.print("\t");
            Bluetooth.println(q.z);}
            break;

        case EULER_A:
        
            // display Euler angles in degrees
            {gyro.dmpGetQuaternion(&q, fifoBuffer);
            gyro.dmpGetEuler(euler, &q);
            Bluetooth.print("euler\t");
            Bluetooth.print(euler[0] * 180/M_PI);
            Bluetooth.print("\t");
            Bluetooth.print(euler[1] * 180/M_PI);
            Bluetooth.print("\t");
            Bluetooth.println(euler[2] * 180/M_PI);}
        break;
      

        case YAWPITCHROLL :
            // display Euler angles in degrees
            {gyro.dmpGetQuaternion(&q, fifoBuffer);
            gyro.dmpGetGravity(&gravity, &q);
            gyro.dmpGetYawPitchRoll(ypr, &q, &gravity);
            Bluetooth.print("ypr\t");
            Bluetooth.print(ypr[0] * 180/M_PI);
            Bluetooth.print("\t");
            Bluetooth.print(ypr[1] * 180/M_PI);
            Bluetooth.print("\t");
            Bluetooth.println(ypr[2] * 180/M_PI);
             break;
             }
          
        case REALACCEL:
        
            // display real acceleration, adjusted to remove gravity
            {gyro.dmpGetQuaternion(&q, fifoBuffer);
            gyro.dmpGetAccel(&aa, fifoBuffer);
            gyro.dmpGetGravity(&gravity, &q);
            gyro.dmpGetLinearAccel(&aaReal, &aa, &gravity);
            Bluetooth.print("areal\t");
            Bluetooth.print(aaReal.x);
            Bluetooth.print("\t");
            Bluetooth.print(aaReal.y);
            Bluetooth.print("\t");
            Bluetooth.println(aaReal.z);}
        break;

        case WORLDACCEL:
        
            // display initial world-frame acceleration, adjusted to remove gravity
            // and rotated based on known orientation from quaternion
            {gyro.dmpGetQuaternion(&q, fifoBuffer);
            gyro.dmpGetAccel(&aa, fifoBuffer);
            gyro.dmpGetGravity(&gravity, &q);
            gyro.dmpGetLinearAccel(&aaReal, &aa, &gravity);
            gyro.dmpGetLinearAccelInWorld(&aaWorld, &aaReal, &q);
            Bluetooth.print("aworld\t");
            Bluetooth.print(aaWorld.x);
            Bluetooth.print("\t");
            Bluetooth.print(aaWorld.y);
            Bluetooth.print("\t");
            Bluetooth.println(aaWorld.z);}
        break;
    
        case OUTPUTTEAPOT:
        
            // display quaternion values in InvenSense Teapot demo format:
            {teapotPacket[2] = fifoBuffer[0];
            teapotPacket[3] = fifoBuffer[1];
            teapotPacket[4] = fifoBuffer[4];
            teapotPacket[5] = fifoBuffer[5];
            teapotPacket[6] = fifoBuffer[8];
            teapotPacket[7] = fifoBuffer[9];
            teapotPacket[8] = fifoBuffer[12];
            teapotPacket[9] = fifoBuffer[13];
            Bluetooth.write(teapotPacket, 14);
            teapotPacket[11]++;} // packetCount, loops at 0xFF on purpose
              break;
              
         case Tempst:
           temp_raw=gyro.getTemperature ();
           tempreal= temp_raw/340 + 36.53;
           Bluetooth.print("tempreal \t");
           Bluetooth.println(tempreal);
           break;
         default :
         break;
    }
  }
}



// ================================================================
// ===               Unused Functions                           ===
// ================================================================

   /*
void compare_char_array()
{
  delayMicroseconds(2);
if (strcasecmp(A,"Distance")==0)
{
  Bluetooth.print("The distance between the sensor and the object is ");
  Bluetooth.println(find_distance());}
  else if (strcasecmp(A,"Marwan")==0)
  Bluetooth.println("He's the best person on earth");
  else 
  Bluetooth.println("Invalid text . Please try again!");
  }
  */
