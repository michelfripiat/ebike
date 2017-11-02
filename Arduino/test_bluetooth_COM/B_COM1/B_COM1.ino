/*
  Blink
  Turns on an LED on for one second, then off for one second, repeatedly.

  Most Arduinos have an on-board LED you can control. On the UNO, MEGA and ZERO 
  it is attached to digital pin 13, on MKR1000 on pin 6. LED_BUILTIN is set to
  the correct LED pin independent of which board is used.
*/
#include "TimerOne.h"
int LED = 13;
int State= 0;



// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);
  Serial.begin(38400);
}

// the loop function runs over and over again forever
void loop() {
  
 if (Serial.available()>0)
 {State = Serial.read();}

 if (State == '0')
 {
  digitalWrite(LED,LOW);
  Serial.write(120);
  //Serial.println("LED: OFF");
  delay(500);
  State = 0;
 }

 if (State == '1')
 {
  digitalWrite(LED,HIGH);
  Serial.write(124);
  //Serial.println("LED: ON");
  delay(500);
  State = 0;
 }

 

}
