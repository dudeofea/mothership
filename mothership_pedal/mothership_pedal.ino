#include <LiquidCrystal.h>

LiquidCrystal lcd(22, 23, 27, 26, 25, 24);
int val = 0;
int len = 0;
char buf[20];
int pots[10];

void setup() {
  // put your setup code here, to run once:
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  Serial.begin(9600);
  //BLE Mini Setup
  Serial1.begin(57600);
  lcd.begin(16,2);
  lcd.print("hello, world!");
}

int read_msg(){
  len = -1;
  if(Serial1.available()){
    len = Serial1.read();
    for(int i = 0; i < len; i++){
      val = -1;
      while(val < 0){
        val = Serial1.read();
      }
      buf[i] = val;
    }
    buf[len++] = '\0';
  }
  return len;
}

void loop() {
  // put your main code here, to run repeatedly:
  analogWrite(2,0);
  analogWrite(3,0);
  analogWrite(4,0);
  if(read_msg() > 0){
    //get msg type
    switch(buf[0]){
      case 1:    //module details
        Serial.print("id: ");
        Serial.println(buf[1]);
        Serial.print("name: ");
        Serial.println(buf+3);
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print(buf+3);
        lcd.setCursor(0,1);
        lcd.print(String(buf[1], DEC));
        lcd.print(" - ");
        break;
    }
    //Serial1.write(buf+3);
  }
}


