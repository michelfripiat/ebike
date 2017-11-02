/*
  Blink
  Turns on an LED on for one second, then off for one second, repeatedly.

  Most Arduinos have an on-board LED you can control. On the UNO, MEGA and ZERO 
  it is attached to digital pin 13, on MKR1000 on pin 6. LED_BUILTIN is set to
  the correct LED pin independent of which board is used.
*/
#include "TimerOne.h"

int LED = 13;
int Pin_Hall_A = 2;
int Pin_Hall_B = 3;
int Pin_Hall_C = 4;

int HA = 8;
int HB = 9;
int HC = 10;
int LA = 11;
int LB = 12;
int LC = 13;

int Time_Tic = 0;
int Distance_Tic = 0;
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
float Time_Per_Tic=0.000250;   //  second/time_Tic

float Distance = 0;
int Motorization_Current_Pin = A0;  
int Battery_Voltage_Pin = A1; 
int PWM_Pin = 6;  

int Motorization_Current_cmd = 0;
int Position = 0; // Motor position state
float Reg_Integrator=0;


// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED, OUTPUT);
  pinMode(HA, OUTPUT);
  pinMode(HB, OUTPUT);
  pinMode(HC, OUTPUT);
  pinMode(LA, OUTPUT);
  pinMode(LB, OUTPUT);
  pinMode(LC, OUTPUT);
  Open_All_Gates();
  
  Timer1.initialize(250); 
  Timer1.attachInterrupt(Update_Switch);

  Serial.begin(115200);
}

// the loop function runs over and over again forever
void loop() {
  digitalWrite(LED, LOW);    // turn the LED off by making the voltage LOW
  
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

  int PWM_Motor = Control_Loop(Motorization_Current,Motorization_Current_cmd);
  analogWrite(PWM_Pin,PWM_Motor);

  Communication(Distance,Speed,Acc,Battery_Voltage,Motorization_Current,PWM_Motor);
  
  delay(50);                       // wait for a 50 milli-second
}

void Update_Switch() {
    
  Position = Position_State();
  Actuation(Position);
}

int Position_State(){

  int New_Hall_A=digitalRead(Pin_Hall_A);

  if (Last_Hall_A != New_Hall_A)          //update of distance and time Tic
  {Distance_Tic = Distance_Tic+1;
   Last_Hall_A = New_Hall_A;
  }
  Time_Tic=Time_Tic+1;
  
  
  int D = 4*New_Hall_A+2*digitalRead(Pin_Hall_B)+digitalRead(Pin_Hall_C);
  
  if (D==1)
   {return 2;}
  if (D==2)
   {return 4;}
  if (D==3)
   {return 3;}
  if (D==4)
   {return 6;}
  if (D==5)
   {return 1;}
  if (D==6)
   {return 5;}
  if (D==0 || D==7)
   {return 0;}
  
}

void Actuation(int Position) {

  //Open_All_Gates();

  if (Position == 4)
  {
   digitalWrite(HA,LOW);  //safety
   digitalWrite(LA,LOW);
   digitalWrite(LC,HIGH); 
  }
   if (Position == 5)
  {
   digitalWrite(LB,HIGH);  //safety
   digitalWrite(HB,HIGH);
   digitalWrite(HA,LOW);
  }
   if (Position == 6)
  {
   digitalWrite(HC,LOW);  //safety
   digitalWrite(LC,LOW);
   digitalWrite(LB,HIGH);
  }
   if (Position == 1)
  {
   digitalWrite(LA,HIGH);  //safety
   digitalWrite(HA,HIGH);
   digitalWrite(HC,LOW);
  }
   if (Position == 2)
  {
   digitalWrite(HB,LOW);  //safety
   digitalWrite(LB,LOW);
   digitalWrite(LA,HIGH);
  }
   if (Position == 3)
  {
   digitalWrite(LC,HIGH);  //safety
   digitalWrite(HC,HIGH);
   digitalWrite(HB,LOW);
  }
  
   if (Position == 0)
  {
   Open_All_Gates();
  }
  
}

void  Open_All_Gates() {
  digitalWrite(HA, LOW);
  digitalWrite(HB, LOW);
  digitalWrite(HC, LOW);
  digitalWrite(LA, HIGH);
  digitalWrite(LB, HIGH);
  digitalWrite(LC, HIGH);
}

int Control_Loop(float Motorization_Current,int Motorization_Current_cmd) {

float Kp=10;
float Ki=0;
float tg = float(Motorization_Current_cmd)/100;
Reg_Integrator=Reg_Integrator+Ki*(tg-Motorization_Current);
float PWM_ = Kp*(tg-Motorization_Current)+127;
int PWM_Motor = int(PWM_);
if (PWM_>255)
{int PWM_Motor=255; }
if (PWM_<0)
{int PWM_Motor=0; }

 
return PWM_Motor;
}

void Communication(float Distance,float Speed,float Acc,float Battery_Voltage,float Motorization_Current, int PWM_Motor) {

 while (Serial.available() > 0) {

   Motorization_Current_cmd = Serial.read();

 }

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
   

Serial.write(222);   //Stopbyte 


}

