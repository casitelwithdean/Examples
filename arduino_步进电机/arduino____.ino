void setup() {
  // put your setup code here, to run once:
pinMode(3,OUTPUT);
pinMode(4,OUTPUT);
pinMode(5,OUTPUT);
pinMode(6,OUTPUT);
for(int i=0;i<30;i++){
digitalWrite(3,1);
digitalWrite(4,0);
digitalWrite(5,0);
digitalWrite(6,0);
delayMicroseconds(800);
digitalWrite(3,1);
digitalWrite(4,1);
digitalWrite(5,0);
digitalWrite(6,0);
delayMicroseconds(800);
digitalWrite(3,0);
digitalWrite(4,1);
digitalWrite(5,0);
digitalWrite(6,0);
delayMicroseconds(800);
digitalWrite(3,0);
digitalWrite(4,1);
digitalWrite(5,1);
digitalWrite(6,0);
delayMicroseconds(800);
digitalWrite(3,0);
digitalWrite(4,0);
digitalWrite(5,1);
digitalWrite(6,0);
delayMicroseconds(800);
digitalWrite(3,0);
digitalWrite(4,0);
digitalWrite(5,1);
digitalWrite(6,1);
delayMicroseconds(800);
digitalWrite(3,0);
digitalWrite(4,0);
digitalWrite(5,0);
digitalWrite(6,1);
delayMicroseconds(800);
digitalWrite(3,1);
digitalWrite(4,0);
digitalWrite(5,0);
digitalWrite(6,1);
delayMicroseconds(800);
}
}

void loop() {
  // put your main code here, to run repeatedly:
digitalWrite(3,1);
digitalWrite(4,0);
digitalWrite(6,0);
delayMicroseconds(800);

digitalWrite(4,1);

delayMicroseconds(800);
digitalWrite(3,0);
digitalWrite(4,1);

delayMicroseconds(800);


digitalWrite(5,1);
digitalWrite(6,0);
delayMicroseconds(800);

digitalWrite(4,0);

digitalWrite(6,0);
delayMicroseconds(800);


digitalWrite(6,1);
delayMicroseconds(800);
digitalWrite(5,0);


delayMicroseconds(800);
digitalWrite(3,1);

delayMicroseconds(800);
}
