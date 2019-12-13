/mbed
#include "mbed.h"
#include "USBHostSerial.h"
#include "LiquidCrystal_I2C.h"
//c++ standard
#include <string>

//adress, 20 chars, 4 lines 
LiquidCrystal_I2C lcd(0x4E, 20, 4);

//hardware A
DigitalIn sw_time(D11);
DigitalIn Laser(D10);
DigitalOut led1(D3), led2(D4), led3(D5), led4(D6);
AnalogIn sensor1(A1), sensor2(A2), sensor3(A3), sensor4(A4)
PwmOut buzzer(D9), servo(7);
Serial slave(D8,D2);
int connect=0;
//global variable A
Timer time_pressing_button;
Timer time_game;
Timer time_respawn;
const char*kill_s =  "Total kill   : ";
const char*death_s = "Total death  : ";
const char*bullet_s ="Total bullet : ";
int kill;
int death;
int bullet;

//global variable B
int  B_time_pressing_button;
int Bdeath;

void led_on(){
  led1 = 1;
  led2 = 1;
  led3 = 1;
  led4 = 1;
}

void led_off(){
  led1 = 0;
  led2 = 0;
  led3 = 0;
  led4 = 0;
}

void buzzer_death(){
  buzzer.period(0.5f);
  buzzer.write(0.5f);
  wait(3);
  buzzer.write(0);
}

void buzzer_timeup(){
  buzzer.period(1.0f);
  buzzer.write(0.7f);
  wait(3);
  buzzer.write(0);
}

void servo_lock(){
  servo.pulsewidth(0.00225);
}

void servo_unlock(){
  servo.pulsewidth(0.001);
}

void show_displayLCD(int totalkill,int totaldeath,int totalbullet,int reset){

  if (totalkill >= 9){
    totalkill = totalkill + 1;
    string str = std::to_string(totalkill);
    const char* ttkill = str.c_str(); 
    lcd.setCursor(15,1);
    lcd.print(ttkill);
  }

  if(totalkill <= 9){
    totalkill = totalkill + 1;
    string str = std::to_string(totalkill);
    const char* ttkill = str.c_str(); 
    lcd.setCursor(16,1);
    lcd.print(ttkill);
  }


  if (totaldeath >= 9){
    totaldeath = totaldeath + 1;
    string str = std::to_string(totaldeath);
    const char* ttdeath = str.c_str(); 
    lcd.setCursor(15,2);
    lcd.print(ttdeath);
  }

  if(totaldeath <= 9){
    totaldeath = totaldeath + 1;
    string str = std::to_string(totaldeath);
    const char* ttdeath = str.c_str();  
    lcd.setCursor(16,2);
    lcd.print(ttdeath);
  }


  if (totalbullet <= 100 and totalbullet >=11){
    totalbullet = totalbullet - 1;
    string str = std::to_string(totalbullet);
    const char* ttbl = str.c_str(); 
    lcd.setCursor(16,3);
    lcd.print(ttbl);
    lcd.setCursor(15,3);
    lcd.print("0");
  }
  else if (totalbullet <= 10 and totalbullet >=1 ){
    totalbullet = totalbullet - 1;
    string str = std::to_string(totalbullet);
    const char* ttbl = str.c_str(); 
    lcd.setCursor(17,3);
    lcd.print(ttbl);
    lcd.setCursor(15,3);
    lcd.print("0");
    lcd.setCursor(16,3);
    lcd.print("0");
  }
  else if (totalbullet <= 1){
    totalbullet = 0;
    lcd.setCursor(15,3);
    lcd.print("000");
  }
  else{
    totalbullet = totalbullet - 1;
    string str = std::to_string(totalbullet);
    const char* ttbl = str.c_str(); 
    lcd.setCursor(15,3);
    lcd.print(ttbl);
  }


  if(reset == 1){
    totalkill = 0;
    totaldeath = 0;
    totalbullet = 300;
    lcd.setCursor(15,1);
    lcd.print("00");
    lcd.setCursor(15,2);
    lcd.print("00");
    lcd.setCursor(15,3);
    lcd.print("300");
  }
}

//state
//0 wait
//1 pressing button
//2 before game start
//3 start
//4 death
//5 end game
void state0(){
  kill = 0;
  death = 0;
  bullet = 300;
  lcd.noDisplay();
  led_off();
  servo_lock();
}

void state1(){
  lcd.noDisplay();
  led_on();
  servo_lock();
}

void state2(){
  led_off();
  servo_lock();

  //set up
  lcd.begin();
  lcd.backlight();
  
  //print defualt message
  lcd.setCursor(0,1);
  lcd.print(kill_s);      
  lcd.setCursor(15,1);
  lcd.print("00");
  lcd.setCursor(0,2);
  lcd.print(death_s);
  lcd.setCursor(15,2);
  lcd.print("00");
  lcd.setCursor(0,3);
  lcd.print(bullet_s);
  lcd.setCursor(15,3);
  lcd.print("300");
}

void state3(){
  requestBdeath();
  kill = Bdeath;
  led_on();
  servo_unlock();
  show_displayLCD(kill,death,bullet,0);
}

void state4(){
  buzzer_death();
  led_off();
  server_lock();
  death += 1;
  show_displayLCD(kill,death,bullet,0);
  updateAdeath();
  time_respawn.start();
  while(time_respawn.read() < 10){};
  time_respawn.stop();
  state = 3;
}

void state5(){
  led_off();
  servo_lock();
  show_displayLCD(kill, death, bullrt,0);
  while(sw_time.read()==1){}
  show_displayLCD(kill,death,bullet,1)
  state = 0;
}

void requestBtime(){
  if(slave.readable()){
    wait_us(100.0);
    B_time_pressing_button = slave.getc();
  }
}

void requestBdeath(){
  if(slave.readable()){
    wait_us(100.0);
    Bdeath = slave.getc();
  }
}

void updateAdeath(){
  slave.putc(death);
  wait_us(100.0);
}

void auto_connect(void const*){
  USBHostSerial serial;

  while(1){
    while(!serial.connect())
      Thread::wait(500);
  }

  while(1){
    if (!serial.connected())
      connect = 0;
      break;
    while(serial.available()){
      connect = 1;
    }
    Thread::wait(50);
  }
}

void stateUpdate(int state){
  switch (state)
  {
  case 0;
    state0();
    if(sw_time.read() == 1){
      state = 1
    }
    break;
  case 1;
    state1();
    time_pressing_button.start();
    while(time.pressing_button.read() < 5 && sw_time.read() == 1){};
    requestBtime();
    if (requestBtime == 4){
      state = 2;
    }else{
      state = 0;
    }    
    break;
  case 2;
    state2();
    time_respawn.start();
    while(time_respawn.read() < 5){};
    time_game.start();
    state = 3;
    break;
  case 3;
    state3();

    if (sensor1.read() > 0.3 || sensor2.read() > 0.3 || sensor3.read() > 0.3 || sensor4.read() > 0.3){
      state = 4;
    }

    if (Laser.read()==1 && bullet!=0){
      bullet -= 1;
    }
    break;
  case 4;
    state4();
    break;
  case 5;
    state5();
    break;
  default;
    state = 0;
    break;
  }
}

int main() {
  state = 0;
  Thread serialTask(auto_connect, NULL, osPriorityNormal, 8)
  while(1) {
    stateUpdate(state);
    Thread::wait(500);
  }
}
