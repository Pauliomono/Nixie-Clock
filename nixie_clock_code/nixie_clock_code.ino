#include <TimeLib.h>
#include <DS1307RTC.h>
#include <Bounce2.h> 

//modified 1/30/21 3:16pm est 75 sec, 137 days

//declare vars
Button switch_reset = Button();
Button switch_mode = Button();
Button switch_light = Button();
int s;
int s10;
int s1;
int m;
int m10;
int m1;
int h;
int h10;
int h1;
int time_mode;
int submode = 0;
bool flash_state;
bool t24h;
int digit_states[10][4] = { //stores the pin states corresponding to each digit
  {0, 0, 1, 1}, //0   --
  {0, 1, 0, 0}, //1   4
  {0, 1, 0, 1}, //2   3
  {1, 1, 0, 1}, //3   
  {1, 1, 0, 0}, //4   
  {1, 0, 0, 0}, //5
  {1, 0, 0, 1}, //6
  {0, 0, 0, 1}, //7
  {0, 0, 0, 0}, //8  7
  {0, 0, 1, 0}, //9  6
};

#define s1_pins 0,1,2,3 
#define s10_pins 4,5,6,7
#define m1_pins 8,9,10,11
#define m10_pins 28,29,30,31
#define h1_pins 36,35,34,33
#define h10_pins 40,39,38,37
#define RESET_PIN 23
#define MODE_PIN 22
#define LIGHT_PIN 21

void setup() { // initialization code
//get time from RTC
  setSyncProvider(getRTCTime);
  setSyncInterval(86400);
  time_mode = 0;

//pin setup  
  pinMode(13, OUTPUT);

  pinMode(0, OUTPUT);
  pinMode(1, OUTPUT);
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(28, OUTPUT);
  pinMode(29, OUTPUT);
  pinMode(30, OUTPUT);
  pinMode(31, OUTPUT);
  pinMode(33, OUTPUT);
  pinMode(34, OUTPUT);
  pinMode(35, OUTPUT);
  pinMode(36, OUTPUT);
  pinMode(37, OUTPUT);
  pinMode(38, OUTPUT);
  pinMode(39, OUTPUT);
  pinMode(40, OUTPUT);  

  switch_reset.attach(RESET_PIN,INPUT_PULLUP);
  switch_reset.interval(10); 
  switch_mode.attach(MODE_PIN,INPUT_PULLUP);
  switch_mode.interval(10);
  switch_light.attach(LIGHT_PIN,INPUT_PULLUP);
  switch_light.interval(10);
  
//start serial terminal
  Serial.begin(9600);
  Serial.println("starting COM port...");
//get time from RTC module
  if(timeStatus()!= timeSet) {
     Serial.println("Unable to sync with the RTC");}
  else{
     Serial.println("RTC and system time synced");}
}

time_t getRTCTime()
{
  return Teensy3Clock.get();
}

int digit10(int input){ //function to get the tens place
  if (input < 10){
    return 0;
  }
  int d1 = int(input/10);
  return d1;
}

int digit1(int input){ //function to get the ones place
  int d1 = int(input/10);
  int d2 = input - 10*d1;
  return d2;
}

void write_tubes(int input_digit,int p1,int p2,int p3,int p4){ //function to write 4 pin combo from a single digit
  digitalWrite(p1, digit_states[input_digit][0]);
  digitalWrite(p2, digit_states[input_digit][1]);
  digitalWrite(p3, digit_states[input_digit][2]);
  digitalWrite(p4, digit_states[input_digit][3]);  
}



void adjust_time(){
  int s0 = now();
  flash();
  switch_reset.update();
  switch_mode.update();
  switch_light.update();

  if(switch_reset.pressed()){
    time_mode = 0;
    submode = 0;
    Teensy3Clock.set(s0);
  }
  if(switch_mode.pressed()){
      submode++;
      if (submode >= 3){
        submode = 0;
    }
    
  }
  if(switch_light.pressed()){
    switch (submode){
      case 0:
        h = hour();
        if(h == 23){
          adjustTime(-82800);
          }
        else{
          adjustTime(3600);
        }
        break;
      case 1:
        m = minute();
        if(m == 59){
          adjustTime(-3540);
        }
        else{
          adjustTime(60);
        }
        break;
      case 2:
        s0 = s0/60;
        setTime(s0*60);
        break;
    }
  }
}

void time() {
  s = second();
  m = minute();
  h = hour();
  if((t24h == 0)&&(h > 12)){
    h = h - 12;
  }
  if((t24h == 0)&&(h == 0)){
    h = 12;
  }
  
  //split into single digits
  s10 = digit10(s);
  s1 = digit1(s);
  m10 = digit10(m);
  m1 = digit1(m);
  h10 = digit10(h);
  h1 = digit1(h);

  //write to tubes
  write_tubes(s1, s1_pins);
  write_tubes(s10, s10_pins);
  write_tubes(m1, m1_pins);
  write_tubes(m10, m10_pins);
  write_tubes(h1, h1_pins);
  write_tubes(h10, h10_pins);
  
  //debug
  Serial.print(h10);
  Serial.print(h1);
  Serial.print(":");
  Serial.print(m10);
  Serial.print(m1);
  Serial.print(":");
  Serial.print(s10);
  Serial.print(s1);
  Serial.print("     ");
  Serial.println(time_mode);
}

void date(){
  s = year()%100;
  m = day();
  h = month();
   
  //split into single digits
  s10 = digit10(s);
  s1 = digit1(s);
  m10 = digit10(m);
  m1 = digit1(m);
  h10 = digit10(h);
  h1 = digit1(h);

  //write to tubes
  write_tubes(s1, s1_pins);
  write_tubes(s10, s10_pins);
  write_tubes(m1, m1_pins);
  write_tubes(m10, m10_pins);
  write_tubes(h1, h1_pins);
  write_tubes(h10, h10_pins);
  
  //debug
  Serial.print(h10);
  Serial.print(h1);
  Serial.print(":");
  Serial.print(m10);
  Serial.print(m1);
  Serial.print(":");
  Serial.print(s10);
  Serial.print(s1);
  Serial.print("     ");
  Serial.println(time_mode);
}

void update_buttons(){
  //check buttons
  switch_reset.update();
  switch_mode.update();
  switch_light.update();
  
//adjust time
  if(switch_reset.pressed()){
    time_mode = 3;
    }
    
    
  //toggle mode
  if(switch_mode.pressed()){
    time_mode++;
    if(time_mode > 2){
      time_mode = 0;
    }
  }
  return;
}

void flash(){
  int t = millis()%1000;
  
  if((t >= 750)||(t < 250)){
    
    Serial.print("on  ");
    time();
  }
  if((t < 750)&&(t >= 250)){
    switch (submode){
      case 0:
        write_tubes(0, h1_pins);
        write_tubes(0, h10_pins);
        Serial.print("00:");
        Serial.print(m10);
        Serial.print(m1);
        Serial.print(":");
        Serial.print(s10);
        Serial.print(s1);
        Serial.print("     ");
        Serial.println(time_mode);
        break;
      case 1:
        write_tubes(0, m1_pins);
        write_tubes(0, m10_pins);
        Serial.print(h10);
        Serial.print(h1);
        Serial.print(":");
        Serial.print("00:");
        Serial.print(s10);
        Serial.print(s1);
        Serial.print("     ");
        Serial.println(time_mode);
        break;
      case 2:
        write_tubes(0, s1_pins);
        write_tubes(0, s10_pins);
        Serial.print(h10);
        Serial.print(h1);
        Serial.print(":");
        Serial.print(m10);
        Serial.print(m1);
        Serial.print(":");
        Serial.print("00   ");
        Serial.println(time_mode);
        break;
    }
    
    Serial.print("off ");
  }
}

void correct(){
  int t = now();
  int rem = t % 157824;
  if(rem == 0){
  t++;
  Teensy3Clock.set(t);
  }
}

void loop() { // main loop
  switch (time_mode){
    case 0:
      time();
      update_buttons();
      t24h = 0;
      break;
    case 1:
      time();
      update_buttons();
      t24h = 1;
      break;
    case 2:
      date();
      update_buttons();
      break;
    case 3:
      adjust_time();
      break;
  }


    

  
  
  
}
