
// IEEE-754 floating-point standard


void setup() {
  // put your setup code here, to run once:
   Serial.begin(115200);

}

int AB = 55;
float BB =-5;


void loop() {
  // put your main code here, to run repeatedly:
  while (Serial.available() > 0) {

   int PWM = Serial.read();
   Serial.print('\n');
   Serial.print(PWM);
   Serial.print('\n');
    
   }
   Serial.print(BB);
   Serial.write(95);
   long BBL = long(BB*1000);

   byte temp[4];
   //temp[0]=(byte)BB;

   
  memcpy(temp, (unsigned char*) (&BBL), 4);
  

   
   

   //temp[3]=5;
   Serial.print(temp[3],HEX);
    Serial.print(temp[2],HEX);
     Serial.print(temp[1],HEX);
      Serial.print(temp[0],HEX);

   BB=BB+1;
  Serial.print('\n');
   //byte Q = (byte)BB;
   //Serial.print(Q,HEX);

   
  // byte *data = (byte*) & BB;
   //Serial.print(data.length);
   //Serial.print(data[6],BIN);
   //Serial.write(data[1]);
   //Serial.write(data[2]);
   //Serial.write(data[3]);
   //Serial.write(data[4]);
   //Serial.write(95);
    //Serial.print(BB,BIN);
   
   delay(1000);
}
