/*
  Blink
  Turns on an LED on for one second, then off for one second, repeatedly.

  Most Arduinos have an on-board LED you can control. On the UNO, MEGA and ZERO 
  it is attached to digital pin 13, on MKR1000 on pin 6. LED_BUILTIN is set to
  the correct LED pin independent of which board is used.
*/
#include <SoftwareSerial.h>
#include "TimerOne.h"

int Message_Length=6; //length in bytes of the received messages

int LED = 13;
int HallState1; //Variables for the three hall sensors (3,2,1)
int HallState2;
int HallState3;
int HallVal = 1; //binary value of all 3 hall sensors
int mSpeed = 0; //speed level of the motor
int bSpeed = 0; //braking level

int bluetoothTx = 2;  // TX-O pin of bluetooth mate, (greenwire)
int bluetoothRx = 3;  // RX-I pin of bluetooth mate, (bluewire)
SoftwareSerial bluetooth(bluetoothTx, bluetoothRx);
int PWM_From_BT = 0;

long Time_Tic = 0;
long Distance_Tic = 0;
int Last_Hall_A = 0;

float Raw_Distance = 0;
float Raw_Time = 0;
float Raw_Speed1 = 0;
float Raw_Speed2 = 0;
float Raw_Speed3 = 0;
float Raw_Speed4 = 0;
float Raw_Speed5 = 0;
float Raw_Speed6 = 0;
float Raw_Speed7 = 0;
float Raw_Speed8 = 0;
float Raw_Speed9 = 0;
float Raw_Speed10 = 0;

float Distance_Per_Tic=0.033/4.236;  //  m/distance_Tic
float Time_Per_Tic=0.000128;   //  second/time_Tic

float Distance = 0;
int Motorization_Current_Pin = A0;  
int Battery_Voltage_Pin = A1; 

int Motorization_Current_cmd = 0;
int mode_cmd=3;
int pwm_cmd=0;
int action_cmd=0;
int PWM_Motor;
int Inv_PWM_Motor;

int Position = 0; // Motor position state
int Last_Position = 0;
float Reg_Integrator=0;
long delay_1=0;


// the setup function runs once when you press reset or power the board
void setup() {

  pinMode(2,INPUT);    // Hall 1
  pinMode(3,INPUT);    // Hall 2
  pinMode(4,INPUT);    // Hall 3
 
// Outputs for the L6234 Motor Driver
  pinMode(5,OUTPUT);   // IN 1
  pinMode(6,OUTPUT);   // IN 2
  pinMode(7,OUTPUT);   // IN 3    
  pinMode(9,OUTPUT);   // EN 1
  pinMode(10,OUTPUT);  // EN 2
  pinMode(11,OUTPUT);  //  EN 3
  
  
  //Serial.begin(9600); //uncomment this line if you will use the serial connection
  // also uncomment Serial.flush command at end of program.

/* Set PWM frequency on pins 9,10, and 11
// this bit of code comes from
http://usethearduino.blogspot.com/2008/11/changing-pwm-frequency-on-arduino.html
*/ 
  // Set PWM for pins 9,10 to 32 kHz
  //First clear all three prescaler bits:
  int prescalerVal = 0x07; //create a variable called prescalerVal and set it equal to the binary number "00000111"                                                       number "00000111"                                                      number "00000111"
  TCCR1B &= ~prescalerVal; //AND the value in TCCR0B with binary number "11111000"
  //Now set the appropriate prescaler bits:
  int prescalerVal2 = 1; //set prescalerVal equal to binary number "00000001"
  TCCR1B |= prescalerVal2; //OR the value in TCCR0B with binary number "00000001"
  // Set PWM for pins 3,11 to 32 kHz (Only pin 11 is used in this program)
  //First clear all three prescaler bits:
  TCCR2B &= ~prescalerVal; //AND the value in TCCR0B with binary number "11111000"
  //Now set the appropriate prescaler bits:
  TCCR2B |= prescalerVal2; //OR the value in TCCR0B with binary number "00000001"//First clear all three prescaler bits:


  // ------------------------------
  
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED, OUTPUT);

  Serial.begin(38400);

  bluetooth.begin(115200);  // The Bluetooth Mate defaults to 115200bps
  bluetooth.print("$");  // Print three times individually
  bluetooth.print("$");
  bluetooth.print("$");  // Enter command mode
  delay(100);  // Short delay, wait for the Mate to send back CMD
  bluetooth.println("U,9600,N");  // Temporarily Change the baudrate to 9600, no parity
  // 115200 can be too fast at times for NewSoftSerial to relay the data reliably
  bluetooth.begin(9600);  // Start bluetooth serial at 38400
  
}

// the loop function runs over and over again forever
void loop() {
  
  Raw_Distance=Distance_Tic;
  Distance_Tic=0;
  Raw_Time=Time_Tic;
  Time_Tic=0;
  Raw_Distance=Raw_Distance * Distance_Per_Tic;
  Raw_Time=Raw_Time * Time_Per_Tic;

  Raw_Speed1=Raw_Speed2;
  Raw_Speed2=Raw_Speed3;
  Raw_Speed3=Raw_Speed4;
  Raw_Speed4=Raw_Speed5;
  Raw_Speed5=Raw_Speed6;
  Raw_Speed6=Raw_Speed7;
  Raw_Speed7=Raw_Speed8;
  Raw_Speed8=Raw_Speed9;
  Raw_Speed9=Raw_Speed10;
  Raw_Speed10=Raw_Distance/Raw_Time;    //     [m/s]

  Distance = Distance + Raw_Distance;
  float Speed = ( Raw_Speed6 + Raw_Speed7 +  Raw_Speed8 +  Raw_Speed9 +  Raw_Speed10)/(5);
  float Acc= ((Raw_Speed10 + Raw_Speed9 + Raw_Speed8 + Raw_Speed7 + Raw_Speed6 )/5 - (Raw_Speed5 + Raw_Speed4 + Raw_Speed3 + Raw_Speed2 + Raw_Speed1 )/5)/(5*Raw_Time);

  float Motorization_Current=float(analogRead(Motorization_Current_Pin))*0.7*5/1024;
  float Battery_Voltage=float(analogRead(Battery_Voltage_Pin))*10*5/1024;
  
//--------------------
  HallState1 = digitalRead(2);  // read input value from Hall 1
  HallState2  = digitalRead(3);  // read input value from Hall 2
  HallState3  = digitalRead(4);  // read input value from Hall 3
  HallVal = (HallState1) + (2*HallState2) + (4*HallState3); //Computes the binary value of the 3 Hall sensors
//--------------------

  Control_Loop(Motorization_Current); //generate PWM_Motor 
  Inv_PWM_Motor=255-PWM_Motor;
  
  mSpeed = PWM_Motor; //map(PWM_Motor, 512, 1023, 0, 255); //motoring is mapped to the top half of potentiometer
  bSpeed = map(PWM_Motor, 0, 511, 255, 0);    // regenerative braking on bottom half of pot
  
  Actuation();


  
  
  Communication(Distance,Speed,Acc,Battery_Voltage,Motorization_Current);
  
  action_management();
  

  
  //delay_1=0;                     
}

void Actuation() {
   switch (HallVal)
       {
        case 3:
          //PORTD = B011xxx00;  // Desired Output for pins 0-7 xxx refers to the Hall inputs, which should not be changed
          PORTD  &= B00011111;
          PORTD  |= B01100000;  //

          analogWrite(9,mSpeed); // PWM on Phase A (High side transistor)
          analogWrite(10,0);  // Phase B off (duty = 0)
          analogWrite(11,255); // Phase C on - duty = 100% (Low side transistor)
          break;
        case 1:
          //PORTD = B001xxx00;  // Desired Output for pins 0-7
          PORTD  &= B00011111;  //
          PORTD  |= B00100000;  //

          analogWrite(9,mSpeed); // PWM on Phase A (High side transistor)
          analogWrite(10,255); //Phase B on (Low side transistor)
          analogWrite(11,0); //Phase B off (duty = 0)
          break;
        case 5:
          //PORTD = B101xxx00;  // Desired Output for pins 0-7
          PORTD  &= B00011111;  //
          PORTD  |= B10100000;

          analogWrite(9,0);
          analogWrite(10,255);
          analogWrite(11,mSpeed);
          break;
        case 4: 
          //PORTD = B100xxx00;  // Desired Output for pins 0-7
          PORTD  &= B00011111;
          PORTD  |= B10000000;  //

          analogWrite(9,255);
          analogWrite(10,0);
          analogWrite(11,mSpeed);
          break;
        case 6:
        //PORTD = B110xxx00;  // Desired Output for pins 0-7
          PORTD  &= B00011111;
          PORTD = B11000000;  //

          analogWrite(9,255);
          analogWrite(10,mSpeed);
          analogWrite(11,0);
          break;
        case 2:
          //PORTD = B010xxx00;  // Desired Output for pins 0-7
          PORTD  &= B00011111;
          PORTD  |= B01000000;  //

          analogWrite(9,0);
          analogWrite(10,mSpeed);
          analogWrite(11,255);
          break;
       } 
}

void Control_Loop(float Motorization_Current) {          
  float Kp=10;
  float Ki=0;
  float tg = float(Motorization_Current_cmd)/100;
  float PWM_ = Kp*(tg-Motorization_Current)+127;
   switch (mode_cmd)
       {
        case 1:
          PWM_Motor=pwm_cmd;
          break;
        case 2:

          Reg_Integrator=Reg_Integrator+Ki*(tg-Motorization_Current);
          PWM_Motor = int(PWM_);
          if (PWM_>255)
          {PWM_Motor=255; }
          if (PWM_<0)
          {PWM_Motor=0; }
          break;
        case 3:
          PWM_Motor= PWM_From_BT;
          break;
        }
}

void Communication(float Distance,float Speed,float Acc,float Battery_Voltage,float Motorization_Current) {

 while (Serial.available() > 4*Message_Length) {
   
   int trash=0;
   for (int u1=0; u1 <Message_Length; u1++)
   { trash = Serial.read(); }
   
 }
 
 if (Serial.available() > 2*Message_Length)
 {
  int Message_Tempo_Buffer[6] = {0,0,0,0,0,0};
  for (int i=0; i< Message_Length-1; i++)
  {Message_Tempo_Buffer[i] = Serial.read();}
  
   for (int y = 0; y < Message_Length; y++)
   {
    Message_Tempo_Buffer[y+ Message_Length-1] = Serial.read();

    if (Message_Tempo_Buffer[y] == 95 && Message_Tempo_Buffer[y + Message_Length - 1] == 222)
    {
                        
     Analyse_Message(Message_Tempo_Buffer,y);
                        
     break;
     }
    }
 
 }

 while (bluetooth.available()) // Read while data in BT buffer
  {
    PWM_From_BT = bluetooth.read();
  }
 
 //Motorization_Current_cmd = Serial.read();
 
 
Serial.write(95);   //Startbyte '_'

Serial.write(PWM_Motor);
Serial.write(Position);

long Distance_L = long(Distance*1000);
byte temp[4];
memcpy(temp, (unsigned char*) (&Distance_L), 4);
Serial.write(temp[3]);
Serial.write(temp[2]);
Serial.write(temp[1]);
Serial.write(temp[0]);

long Speed_L = long(Speed*1000);
memcpy(temp, (unsigned char*) (&Speed_L), 4);
Serial.write(temp[3]);
Serial.write(temp[2]);
Serial.write(temp[1]);
Serial.write(temp[0]);

long Acc_L = long(Acc*1000);
memcpy(temp, (unsigned char*) (&Acc_L), 4);
Serial.write(temp[3]);
Serial.write(temp[2]);
Serial.write(temp[1]);
Serial.write(temp[0]); 

long Battery_Voltage_L = long(Battery_Voltage*1000);
memcpy(temp, (unsigned char*) (&Battery_Voltage_L), 4);
Serial.write(temp[3]);
Serial.write(temp[2]);
Serial.write(temp[1]);
Serial.write(temp[0]); 

long Motorization_Current_L = long(Motorization_Current*1000);
memcpy(temp, (unsigned char*) (&Motorization_Current_L), 4);
Serial.write(temp[3]);
Serial.write(temp[2]);
Serial.write(temp[1]);
Serial.write(temp[0]);

Serial.write(Motorization_Current_cmd);
Serial.write(mode_cmd);
   

Serial.write(222);   //Stopbyte 
}

void Analyse_Message(int Message_Tempo_Buffer[],int y)
{
  mode_cmd=Message_Tempo_Buffer[y+1];
  Motorization_Current_cmd=Message_Tempo_Buffer[y+2];
  pwm_cmd=Message_Tempo_Buffer[y+3];
  action_cmd=Message_Tempo_Buffer[y+4];

}

void action_management()
{
  if (action_cmd==147)
  {
    Distance=0;
    action_cmd=0;
  }
  
  if (action_cmd==12)
  {
    digitalWrite(LED, HIGH);
    action_cmd=0;
  }
  
  if (action_cmd==33)
  {
    digitalWrite(LED, LOW);
    action_cmd=0;
  }


}

