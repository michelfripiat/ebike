
#include "TimerOne.h"


int LED = 13;
long delay_1=0;




// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED, OUTPUT);
  digitalWrite(13,LOW);
  
  
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
  TCCR2B = 9;
  
  
  sei();                                                 

  Serial.begin(115200);
}

ISR(TIMER0_COMPA_vect)
{
  Update_Switch();
}

// the loop function runs over and over again forever
void loop() {
  
  OCR1A = 150;
  OCR1B = 128;
  OCR2A = 50;
  int a;
  while (delay_1<39000)               // wait for a 50 milli-second
  {
  digitalWrite(5,LOW);
  }
  digitalWrite(13,HIGH);
  //delay(1);                       // wait for a 50 milli-second
  //digitalWrite(13,LOW);
}

void Update_Switch() {
  
 
  delay_1=delay_1+1;
//  if (counter>39600)
//  {digitalWrite(13,HIGH);}
//  else
//  { digitalWrite(13,LOW);}
  
}


