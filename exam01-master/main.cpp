#include "mbed.h"

#include "uLCD_4DGL.h"


uLCD_4DGL uLCD(D1, D0, D2);
PwmOut PWM1(D6);
AnalogIn Ain(A0);


int main()

{

      uLCD.printf("\n106010006\n"); //Default Green on black text
      uLCD.line(4, 23 , 10, 23, 0xFFFFFF);
      uLCD.line(4, 24 , 10, 24, 0xFFFFFF);
      uLCD.line(4, 25 , 10, 25, 0xFFFFFF);
      uLCD.line(4, 26 , 10, 26, 0xFFFFFF);
      uLCD.line(4, 27 , 10, 27, 0xFFFFFF);
      uLCD.line(4, 28 , 10, 28, 0xFFFFFF);

      wait(30);





  while(1){
    for(float i=0;i<1;i+=0.1 ){

    PWM1.period(0.001);
    PWM1 = Ain;
    PWM1 = i;
    wait(0.1);
    }
    for(float i=0.9;i>0;i-=0.1 ){

    PWM1.period(0.001);
    PWM1 = Ain;
    PWM1 = i;
    wait(0.1);
    }

for (int k = 0; k < sample; k++){

    Aout = Ain;

    ADCdata[i] = Ain;

    pc.printf("%1.3f\r\n", ADCdata[i]);

    wait(0.1);

  }





  }

}
