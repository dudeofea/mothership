#include <LiquidCrystal.h>

LiquidCrystal lcd(22, 23, 27, 26, 25, 24);
int r,g,b;
int val = 0;
int len = 0;
char buf[20];

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
	}
	return len;
}

String getHexColor(int red, int gre, int blu){
	String s[3];
	s[0] = String(red, HEX);
	s[1] = String(gre, HEX);
	s[2] = String(blu, HEX);
	for(int i = 0; i < 3; i++){
		if(s[i].length() == 1){
			s[i] = "0" + s[i];
		} 
	}
	return s[0] + s[1] + s[2];
}

//Send all values from analog 0-9 in a
//compressed 13byte (100bit) packet
void sendPinValues(){
  unsigned int pots[10];
  //get values
  int i, j;
  for(i = 0; i < 10; i++){
    pots[i] = analogRead(i);
  }
  //compress
  byte vals[13];
  memset(vals, 0, sizeof(vals));
  int offset = 8;    //bits we can fit
  int bitcount = 0;  //bits compressed
  int bytecount = 0; //bits we filled in the byte
  int nextval = false;   //triggered when switching value
  int cycle = false;
  i = 0; j = 0;
  while(i < 13){
      if(nextval){
         nextval = false;
         vals[i] |= ((1<<offset)-1) & (pots[j]);
      }else{
         vals[i] |= (((1<<offset)-1) & (pots[j]>>bitcount)) << (8 - offset);
      }
      bitcount += offset;
      bytecount+= offset;
      //we need to switch bytes
      if(bytecount >= 8){
         offset = 10 - offset;
         bytecount = 0;
         i++;
         cycle = true;
      }
      //we need to switch values
      if(bitcount >= 10){
         offset = 8 - offset;
         bitcount = 0;
         j++;
         nextval = true;
         //done a full cycle
         if(cycle){
           offset = 8;
         }
      }
      cycle = false;
  }
  
  /*vals[0] |=  (B11111111) & (pots[0]);                // 8, 8, 8
  vals[1] |= ((B11) & (pots[0] >> (10-2))) << (8-2);   // 2, 10,2
  vals[1] |=  (B111111) & (pots[1]);                   // 6, 6, 8
  vals[2] |= ((B1111) & (pots[1] >> (10-4))) << (8-4); // 4, 10,4
  vals[2] |=  (B1111) & (pots[2]);                     // 4, 4, 8
  vals[3] |= ((B111111) & (pots[2] >> (10-6))) << (8-6);//6, 10,6
  vals[3] |=  (B11) & (pots[3]);                       // 2, 2, 8
  vals[4] |=  (B11111111) & (pots[3] >> (10-8)) <<(8-8);//8, 10,8*/
  
  //char str[10];
  //Send VAL command
  Serial1.write(0x1);
  for(int i = 0; i < 13; i++){
     Serial1.write(vals[i]); 
  }
  Serial1.flush();
}

//Ask server for module details and display
//info accordingly
void initModule(int mod){
  //send request
  Serial1.write(2);  //module details req cmd
  Serial1.write(mod);
  int len = -1;
  while(len < 0){
    len = read_msg();
  }
  Serial.println(len, DEC);
  for(int i = 0; i < 3; i++){
      Serial.print(buf[i], DEC);
      Serial.print(' ');
  }
  for(int i = 3; i < len; i++){
    Serial.print(buf[i]);
  }
  Serial.print('\n');
}

void loop() {
 	// put your main code here, to run repeatedly:
 	/*if(read_msg() > 0){
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
	}*/
        //Color Testing
	/*b = analogRead(0) / 4;
	g = analogRead(1) / 4;
	r = analogRead(2) / 4;
	lcd.clear();
	lcd.setCursor(0,0);
	lcd.print("Color:");
	lcd.setCursor(0,1);
	lcd.print("#" + getHexColor(r,g,b));
	analogWrite(2,255 - b);
	analogWrite(3,255 - g);
	analogWrite(4,255 - r);
	//delay(100);*/
        //Pin testing
        //sendPinValues();
        //Read testing
        initModule(0);
        delay(50);
}


