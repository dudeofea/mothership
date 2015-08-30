#include <LiquidCrystal.h>

#define BUTTON_LEFT    51
#define BUTTON_CENTER  52
#define BUTTON_RIGHT   53

LiquidCrystal lcd(22, 23, 27, 26, 25, 24);
int r,g,b;
int val = 0;
int len = 0;
char buf[40];
int effects_len = 0;

//pots
int pots[10];      //current values of pots
int last_pots[10]; //last values of pots

//state variables
int mod_init = 0;     //1 if module initialized, 0 otherwise
int page = 0;         //0 is main page, 1 is settings
int action_timeout = 0;//timeout so buttons don't loop so much

//module variables
int mod = 0;
int inp_ports = 0;
int out_ports = 0;
int arg_ports = 0;
int mod_r = 0, mod_g = 0, mod_b = 0;
char mod_name[17];
int mod_name_len = 0;

void setup() {
	// put your setup code here, to run once:
	pinMode(2, OUTPUT);
	pinMode(3, OUTPUT);
	pinMode(4, OUTPUT);
        pinMode(BUTTON_LEFT, INPUT_PULLUP);
        pinMode(BUTTON_CENTER, INPUT_PULLUP);
        pinMode(BUTTON_RIGHT, INPUT_PULLUP);
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
		for(int i = 0; i < len-1; i++){
			val = -1;
			while(val < 0){
				val = Serial1.read();
			}
			buf[i] = val;
                        //Serial.print(buf[i], DEC);
                        //Serial.print(' ');
		}
                //Serial.print('\n');
	}
	return len;
}

void clear_buf(){
   for(int i = 0; i < sizeof(buf); i++){
       buf[i] = 0;
   } 
}

void refresh_screen(){
    //set color
    analogWrite(2,255 - mod_b);
    analogWrite(3,255 - mod_g);
    analogWrite(4,255 - mod_r);
    //print info
    lcd.clear();
    //center name with dashes
    int before_count = (16 - mod_name_len)/2;
    //show arrow if not first
    if(mod > 0){
      lcd.print((char)127); //left arrow
    }else{
       lcd.print(' ');
    }
    for(int i = 0; i < before_count-1; i++){
       lcd.print(' '); 
    }
    for(int i = 0; i < mod_name_len; i++){
       lcd.print(mod_name[i]); 
    }
    for(int i = 0; i < 15 - (mod_name_len + before_count); i++){
       lcd.print(' ');
    }
    //show arrow if not last
    if(mod < effects_len - 1){
       lcd.print((char)126); //right arrow
    }else{
       lcd.print(' ');
    }
    //print port numbers
    lcd.setCursor(0, 1);
    lcd.print("I:");
    lcd.print(inp_ports);
    lcd.setCursor(8, 1);
    lcd.print("O:");
    lcd.print(out_ports);
    //print connections
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
  //get values
  int i, j;
  int old = 0;
  int diff = 0;
  for(i = 0; i < 10; i++){
    last_pots[i] = pots[i];
    pots[i] = analogRead(i);
    if(abs(last_pots[i]-pots[i]) > 3){
       old = 1;
    }
  }
  //don't send if not new
  if(old == 0){
    return;
  }
  Serial.print("---");
  Serial.println(abs(last_pots[0]-pots[0]), DEC);
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
  Serial1.write(mod);
  for(int i = 0; i < 13; i++){
     Serial1.write(vals[i]);
  }
  Serial1.flush();
}

//Ask server for module details and display
//info accordingly
void init_mod(){
  //send request
  Serial1.write(2);  //module details req cmd
  Serial1.write(mod);
  int len = -1;
  while(len <= 0){
    len = read_msg();
  }
  //set info
  inp_ports = buf[0];
  out_ports = buf[1];
  arg_ports = buf[2];
  mod_r = buf[3];
  mod_g = buf[4];
  mod_b = buf[5];
  effects_len = buf[6];
  
  //get name
  len = -1;
  while(len <= 0){
    len = read_msg();
  }
  mod_name_len = len - 1;
  strncpy(mod_name, buf, 17);
  //refresh screen
  refresh_screen();
  clear_buf();
  //set flag
  mod_init = 1;
}

void loop() {
        //init the module if not initialized
        Serial.print(mod, DEC);
        Serial.println(effects_len, DEC);
        if(mod_init == 0){
           init_mod();
        }else if(page == 0){
            //send values
            sendPinValues();
            //check for buttons
            if(action_timeout <= 0 && digitalRead(BUTTON_LEFT) == LOW){
              //goto prev module
              if(mod > 0){
                 Serial.print("left\n");
                 action_timeout = 300;
                 mod--;
                 mod_init = 0; 
              }
            }else if(action_timeout <= 0 && digitalRead(BUTTON_RIGHT) == LOW){
              //goto next module
              if(mod < effects_len - 1){
                 Serial.print("right\n");
                 action_timeout = 300;
                 mod++;
                 mod_init = 0;
              }
            }
        }
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
        //buttons testing
        /*val = digitalRead(BUTTON_LEFT);
        if(val == LOW){
           Serial.print("left\n");
        }
        val = digitalRead(BUTTON_CENTER);
        if(val == LOW){
           Serial.print("center\n");
        }
        val = digitalRead(BUTTON_RIGHT);
        if(val == LOW){
           Serial.print("right\n");
        }*/
        delay(10);
        if(action_timeout >= 0){
           action_timeout -= 100;
        }
}


