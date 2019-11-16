#include "mbed.h"
#include <iostream>
#include <string>
#include <ctime>
#include <bitset>

//basic operater function
float power(float a, float b)
{
  int ans = a;
  for(int i = 1; i < b; i++)
    ans *= a;
  return ans;
}

string bin_to_ascii(string bin);
string ascii_to_bin(string ascii);

//global hardware variables of A
DigitalIn AstartButton(D9);
// DigitalIn Asensor(D10); //(sensor ต่อขนาน 4 ตัว)
// DigitalIn Aled(D11);
// DigitalIn Abuzzer(D12);
// DigitalIn Atrigger(D13);
// DigitalIn Alcd(D14);
// DigitalIn Ashoot(D15);
//global hardware variables of B
//send data 7 bit per time
//b state that sent from slave have only 0, 1 during start game 
//getc() -> recieved ascii -> convert to binary -> save to these variables
//(1)send_bytes
//(2)convert variables to binary -> convert to ascii -> change to char -> putc()
Serial slave(D8, D2);
string dataB_ascii;
string dataB_binary;

//global game variables
int state = 0;
clock_t start, stop;
float convert2sec = power(10,6);

class Player
{
public:
  //game
  int state = 1;
  int bullet = 0;
  int death = 0;
  clock_t immortal_start, immmortal_stop;
  
  //hardward
  bool startButton = 0;
  bool sensor = 0;
  bool led = 0;
  bool buzzer = 0;
  bool trigger = 0;
  bool lcd = 0;

  //player state
  //0 death
  //1 alive
  void updateState()
  {
    if (state==1 && sensor == 1)
    {
      state = 0;
      bullet = 0;
      death += 1;

      startButton = 0;
      sensor = 0;
      led = 0;
      buzzer = 1;
      trigger = 0;
      lcd = 1;
      wait_ms(100);
      immortal_start = std::clock()/convert2sec;
    }
    else if(state==0)
    {
      buzzer = 0;
      immmortal_stop = std::clock()/convert2sec;
      if(stop - start > 5)
      {
        state = 1;
        bullet = 300;

        led = 1;
        trigger = 1;
        lcd = 1;
      }
    }
    else
    {
      state = 1;
    }
    
  }
};

//game state
//0 wait to start
//1 press start button
//2 press start button for 5 sec
//3 play mode
//4 End game
int stateUpdate(int state, Player& a, Player& b)
{
  switch(state)
  {
    case 0:
      a.led = 0;
      a.sensor = 0;
      a.buzzer = 0;
      a.trigger = 0;
      a.lcd = 0;

      b.led = 0;
      b.sensor = 0;
      b.buzzer = 0;
      b.trigger = 0;
      b.lcd = 0;
      
      if (a.startButton == 1 && b.startButton == 1)
      {
        start = std::clock()/convert2sec;
        return 1;
      }
      else
      {
        return 0;
      }
      break;
    case 1:
      a.led = 1;
      b.led = 1;
      stop = std::clock()/convert2sec;
      if (a.startButton == 1 && b.startButton == 1)
      {
        if(stop - start > 4)
        {
          start = std::clock()/convert2sec;
          return 2;
        }
        else
        {
          return 1;
        }
      }
      else
      {
        return 0;
      }
      break;
    case 2:
      a.led = 0;
      a.trigger = 0;
      a.lcd = 1;

      b.led = 0;
      b.trigger = 0;
      b.lcd = 1;
      
      stop = std::clock()/convert2sec;
      if (stop - start > 5)
      {
        a.led = 1;
        a.trigger = 1;
        a.lcd = 1;
        
        b.led = 1;
        b.trigger = 1;
        b.lcd = 1;
        
        start = std::clock()/convert2sec;
        return 3;
      }
      else
      {
        return 2;
      }
      break;
    case 3:
      stop = std::clock()/convert2sec;
      if (stop - start > 600)
      {
        return 4;
      }
      else if (a.startButton == 1 && b.startButton == 1 && stop - start>30)
      {
        return 0;
      }
      else
      {
        a.updateState();
        b.updateState();
        return 3;
      }
      break;
    case 4:
      a.led = 0;
      a.sensor = 0;
      a.buzzer = 0;
      a.trigger = 0;
      a.lcd = 1;

      b.led = 0;
      b.sensor = 0;
      b.buzzer = 0;
      b.trigger = 0;
      b.lcd = 1;
      if (a.startButton == 1 && b.startButton == 1)
      {
        return 0;
      }
      else
      {
        return 4;
      } 
      break;      
  }
}

//show LCD
void updateLCD(Player&a)
{
  if(a.lcd==1)
  {
    printf("hi");
  }
}

int binaryAt(string str,int pos)
{
  char char_ans = str.at(pos);
  int ans = char_ans - 48;
  return ans;
}

void requestB(Player& b)
{
  if(slave.readable())
  {
    dataB_ascii = slave.getc();
    dataB_binary = ascii_to_bin(dataB_ascii);

    b.state = binaryAt(dataB_binary,0);
    b.startButton = binaryAt(dataB_binary,1);
    b.sensor = binaryAt(dataB_binary,2);
    b.led = binaryAt(dataB_binary,3);
    b.buzzer = binaryAt(dataB_binary,4);
    b.trigger = binaryAt(dataB_binary,5);
    b.lcd = binaryAt(dataB_binary, 6);
  }
}

void updateB(Player &b)
{
  int ascii = 0;
  ascii = 2*ascii + b.state;
  ascii = 2*ascii + b.startButton;
  ascii = 2*ascii + b.sensor;
  ascii = 2*ascii + b.led;
  ascii = 2*ascii + b.buzzer;
  ascii = 2*ascii + b.trigger;
  ascii = 2*ascii + b.lcd;
  char c = ascii;
  slave.putc(c);
}
int main() 
{
  Player a;
  Player b;
  slave.baud(38400);
  int shoot = 0;
  while(1) 
  {
    a.startButton = AstartButton.read();
    // a.sensor = Asensor.read(); 
    // a.led = Aled.read();
    // a.buzzer = Abuzzer.read();
    // a.trigger = Atrigger.read();
    // a.lcd = Alcd.read();
    // shoot = Ashoot.read();
    
    requestB(b);
    
    state = stateUpdate(state,a,b);

    updateB(b);

    //show LCD
    updateLCD(a);
  }
}