/*
  Blink
  Turns on an LED on for one second, then off for one second, repeatedly.

  Most Arduinos have an on-board LED you can control. On the UNO, MEGA and ZERO 
  it is attached to digital pin 13, on MKR1000 on pin 6. LED_BUILTIN is set to
  the correct LED pin independent of which board is used.
*/
#include <SoftwareSerial.h>
#include "TimerOne.h"
int LED = 13;
int State= 0;
 
int bluetoothTx = 2;  // TX-O pin of bluetooth mate, Arduino D2  (greenwire)
int bluetoothRx = 3;  // RX-I pin of bluetooth mate, Arduino D3  (bluewire)

SoftwareSerial bluetooth(bluetoothTx, bluetoothRx);




// the setup function runs once when you press reset or power the board
void setup() {
  Serial.begin(9600);  // Begin the serial monitor at 9600bps
 
  bluetooth.begin(115200);  // The Bluetooth Mate defaults to 115200bps
  bluetooth.print("$");  // Print three times individually
  bluetooth.print("$");
  bluetooth.print("$");  // Enter command mode
  delay(100);  // Short delay, wait for the Mate to send back CMD
  bluetooth.println("U,9600,N");  // Temporarily Change the baudrate to 9600, no parity
  // 115200 can be too fast at times for NewSoftSerial to relay the data reliably
  bluetooth.begin(9600);  // Start bluetooth serial at 9600
  pinMode(LED, OUTPUT);
}

// the loop function runs over and over again forever
void loop() {

 
 if (bluetooth.available()) // If the bluetooth sent any characters
  {
    // Send any characters the bluetooth prints to the serial monitor
    int value = bluetooth.read();
    
    Serial.println((char)value);
    //Serial.println(' ');
    //Serial.write(value);
    //int value = bluetooth.read();
    //Serial.write(value);
    //Serial.print(value);
    //Serial.write(bluetooth.read());
    char dataFromBt = (char)value;

    if (dataFromBt == '1') {
      Serial.println("led on");
      digitalWrite(LED, HIGH);
      bluetooth.print("1");
    }
    if (dataFromBt == '0') {
      Serial.println("led off");
      digitalWrite(LED, LOW);
      bluetooth.print("0");
    }
  }

}
