#include "mbed.h"

#include "TextLCD.h"


DigitalOut led(LED_RED);

TextLCD lcd(D2, D3, D4, D5, D6, D7);


int main()

{

      
      lcd.printf("106010006\n");

     
     int x=30;

      
      while(true)

      {


            
            for(x; x>=0;x--){

            lcd.locate(5,1);

            lcd.printf("%5i",x);    //conuter display

            wait(1);

            }

      }

      

}