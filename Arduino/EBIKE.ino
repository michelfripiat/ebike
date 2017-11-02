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
int Pin_Hall_A = 4;
int Pin_Hall_B = 5;
int Pin_Hall_C = 6;

int HA = 9;
int HB = 10;
int HC = 11;
int LA = 8;
int LB = 12;
int LC = 7;

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
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED, OUTPUT);
  pinMode(HA, OUTPUT);
  pinMode(HB, OUTPUT);
  pinMode(HC, OUTPUT);
  pinMode(LA, OUTPUT);
  pinMode(LB, OUTPUT);
  pinMode(LC, OUTPUT);
  Open_All_Gates();
  
  //Timer1.initialize(250); 
  //Timer1.attachInterrupt(Update_Switch);
  
  TCCR0A = 0;     //setting timer0 for interrupts every 128us to update switch of MOSFET
  TIMSK0 = 2;
  OCR0A = 255;
  TCCR0B = 2;  
  
  TCCR1A = 241;   //setting timer1 for pwm's
  OCR1A = 255;
  OCR1B = 255;
  TCCR1B = 9;
  
  TCCR2A = 243;   //setting timer2 for pwm
  OCR2A = 255;
  TCCR2B = 1;
  
  
  sei();                                                 

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

ISR(TIMER0_COMPA_vect)
{
  Update_Switch();
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

  Control_Loop(Motorization_Current); //generate PWM_Motor 
  Inv_PWM_Motor=255-PWM_Motor;
  Actuation();
  
  Communication(Distance,Speed,Acc,Battery_Voltage,Motorization_Current);
  
  action_management();
  
  while (delay_1<390)               // wait for a 50 milli-second
  {digitalWrite(5,LOW);}
  
  delay_1=0;
  //delay(50);                      
}

void Update_Switch() {
  
  delay_1=delay_1+1;
  
  Position = Position_State();
  if (Position!=Last_Position)
  {Actuation();}
  Last_Position=Position;
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

void Actuation() {

  //Open_All_Gates();

  if (Position == 4)
  {
   if (Last_Position != 5)
   {Open_All_Gates();
    OCR1B=Inv_PWM_Motor; //HB ON
    TCNT1=0;
   }
   digitalWrite(LA,LOW);  //LA ON
   digitalWrite(LC,HIGH); //LC OFF
  }
  if (Position == 5)
  {
   if (Last_Position != 6)
   {Open_All_Gates();
    digitalWrite(LC,LOW);  //LC ON
   }
   
   OCR1B=Inv_PWM_Motor;  //HB ON
   TCNT1=0;
   OCR1A=255;  //HA OFF
   TCNT1=0;
  }
  if (Position == 6)
  {
   if (Last_Position != 1)
   {Open_All_Gates();
    OCR1A=Inv_PWM_Motor; //HA ON
    TCNT1=0;
   }
   
   digitalWrite(LC,LOW);  //LC ON
   digitalWrite(LB,HIGH); // LB OFF
  }
  if (Position == 1)
  {
   if (Last_Position != 2)
   {Open_All_Gates();
    digitalWrite(LB,LOW);  //LB ON
   }
   
    OCR1A=Inv_PWM_Motor; //HA ON
    TCNT1=0; 
    OCR2A=255; //HC OFF
    TCNT2=0;
  }
  if (Position == 2)
  {
   if (Last_Position != 3)
   {Open_All_Gates();
    OCR2A=Inv_PWM_Motor; //HC ON
    TCNT2=0;
   }
   
   digitalWrite(LB,LOW); // LB ON
   digitalWrite(LA,HIGH);// LA OFF
  }
  if (Position == 3)
  {
   if (Last_Position != 4)
   {Open_All_Gates();
    digitalWrite(LA,LOW);// LA ON
   }
   
   OCR2A=Inv_PWM_Motor; //HC ON
   TCNT2=0;
   OCR1B=255; //HB OFF
   TCNT1=0;
  }
  
   if (Position == 0)
  {
   Open_All_Gates();
  }
  
}

void  Open_All_Gates() {
  OCR1A=255; //HA OFF
  OCR1B=255; //HB OFF
  TCNT1=0;
  OCR2A=255; //HC OFF
  TCNT2=0;
  digitalWrite(LA, HIGH); //LA OFF
  digitalWrite(LB, HIGH); //LB OFF
  digitalWrite(LC, HIGH); //LC OFF
}

void Control_Loop(float Motorization_Current) {

if (mode_cmd==1)
{
  PWM_Motor=pwm_cmd;
}

if (mode_cmd==2)
{
float Kp=10;
float Ki=0;
float tg = float(Motorization_Current_cmd)/100;
Reg_Integrator=Reg_Integrator+Ki*(tg-Motorization_Current);
float PWM_ = Kp*(tg-Motorization_Current)+127;
PWM_Motor = int(PWM_);
if (PWM_>255)
{PWM_Motor=255; }
if (PWM_<0)
{PWM_Motor=0; }
}

if (mode_cmd==3)
{
  PWM_Motor= PWM_From_BT;
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

