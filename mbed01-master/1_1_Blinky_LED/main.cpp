#include "mbed.h"


DigitalOut redLED(LED1);




int main(){

  while(1){

    redLED = 0;

    wait(0.2);

  }

}