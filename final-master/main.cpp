#include "mbed.h"
#include "mbed_rpc.h"
#include "fsl_port.h"
#include "fsl_gpio.h"
#include "bbcar.h"
#include "bbcar_rpc.h"

Serial uart(D1, D0); //tx,rx
Ticker servo_ticker;
PwmOut pin9(D9), pin8(D8);
BBCar car(pin8, pin9, servo_ticker);
DigitalOut redLED(LED1);
DigitalInOut pin10(D10);
parallax_ping ping(pin10);
RawSerial xbee(D12, D11);
Thread t;
EventQueue queue(16 * EVENTS_EVENT_SIZE);
int en = 0;
int thistime = 0;
char act = 'S';

void sendXbee(void);
void getshape(void);
void getnumber(void);

int main()
{
  xbee.baud(9600);
  uart.baud(9600);
  redLED = 1;
  t.start(callback(&queue, &EventQueue::dispatch_forever));
  queue.call(&sendXbee);
  wait(3);
  
  act = 'S';
  car.goStraight(-100);
  wait(9);
  car.stop();
  wait(1);
  act = 'L';
  car.turn(-80,-0.3); //turn left 90
  wait(0.8);
  car.stop();
  wait(0.8);
  act = 'S';
  car.goStraight(-100);
  wait(5.5);
  car.stop();
  wait(1);
  act = 'R';
  car.turn(80,-0.3); //turn right 90
  wait(1);
  car.stop();
  wait(0.8);
  act = 'S';
  //park
  car.goStraight(100); //back
  wait(2.5);
  car.stop();
  wait(3);
  act = 'S';
  car.goStraight(-100);
  wait(1.8);
  car.stop();
  wait(0.8);
  act = 'a';
  getnumber();
  wait(1);
  act = 'R';
  car.turn(80,-0.3); //turn right 90
  wait(0.8);
  car.stop();
  wait(0.8);
  act = 'S';
  car.goStraight(-100);
  wait(4.5);
  car.stop();
  wait(1);
  act = 'R';
  car.turn(80,-0.3); //turn right 90
  wait(1);
  car.stop();
  wait(0.8);
  act = 'S';
  car.goStraight(-100);
  wait(8.8);
  car.stop();
  wait(1);
  act = 'R';
  car.turn(80,-0.3); //turn right 90
  wait(1);
  car.stop();
  wait(0.8);
  act = 'S';
  car.goStraight(-100);
  wait(4.5);
  car.stop();
  wait(1);
  act = 'R';
  car.turn(80,-0.3); //turn right 90
  wait(1);
  car.stop();
  wait(0.8);
  act = 'S';
  
  car.goStraight(-100);
  wait(1);
  car.stop();
  wait(0.8);
  
  act = 'a';
  getshape();
  wait(0.5);
  
  act = 'S';
  car.goStraight(100); //back
  wait(1);
  car.stop();
  wait(1);
  act = 'L';
  car.turn(-80,-0.3); //turn left 90
  wait(1);
  car.stop();
  wait(0.8);
  act = 'S';
  car.goStraight(-100);
  wait(2);
  car.stop();
  wait(1);
  act = 'R';
  car.turn(80,-0.3); //turn right 90
  wait(0.9);
  car.stop();
  wait(0.8);
  act = 'S';
  car.goStraight(-100);
  wait(20);

}
void sendXbee(void)
{
  while (1)
  {
    if (en == 1)
    {
      wait(1);
    }
    if (en == 0)
    {
      xbee.printf("%d %c\r\n", thistime, act);
      thistime++;
      wait(1);
    }
  }
}
void getshape(void)
{
  float A[3];
  int shape = 1;
  float d1;
  float d2;
  redLED = 0;
  en = 1;
  xbee.printf("222\r\n");
  // data grabbing
  A[0] = (float)ping;
  car.turn(30, 0.01);
  wait(0.4);
  car.stop();
  wait(2);
  A[1] = (float)ping;
  car.turn(-30, 0.01);
  wait(0.8);
  car.stop();
  wait(2);
  A[2] = (float)ping;
  car.turn(30, 0.01);
  wait(0.4);
  car.stop();
  d1 = A[1] - A[0];
  d2 = A[2] - A[0];
  if ((d1-d2 < 0.5)||((d2-d1 < 0.5)){
    if(A[0]-A[1]<0)
    shape=1;
    else if(A[0]-A[1]<1)
    shape=3;
    else
    shape=4;
  }else{
    shape=2;
  }



  if (shape == 1)
    xbee.printf("square\r\n");
  if (shape == 2)
    xbee.printf("slope\r\n");
  if (shape == 3)
    xbee.printf("tip\r\n");
  if (shape == 4)
    xbee.printf("twotips\r\n");
  en = 0;
  redLED = 1;
}

// UART thistimemmand-identify
void getnumber(void)
{
  en = 1;
  redLED = 0;
  char s[10];
  sprintf(s, "getnumber");
  uart.puts(s);
  wait(5);
  if (uart.readable())
  {
    char recv = uart.getc();
    xbee.printf("111\r\n");
    xbee.putc(recv);
    xbee.printf("\r\n");
  }
  en = 0;
  redLED = 1;
}