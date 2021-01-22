#include "mbed.h"
#include "math.h"
Serial pc( USBTX, USBRX );
AnalogOut Aout(DAC0_OUT);
AnalogIn Ain(A0);
DigitalIn  Switch(SW3);
DigitalOut redLED(LED1);
DigitalOut greenLED(LED2);
DigitalOut dot(D8);
BusOut display(D6, D7, D9, D10, D11, D5, D4, D8);
char table[10] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};
int sample = 700;
float ADCdata[700];
float T,f;


int main(){

    for (int i = 0; i < sample; i++){
    Aout = Ain;
    ADCdata[i] = Ain;
    wait(1./sample);
    }
    int g[200];
    int y=0;
    int n=0;
    for(int j=0; j< sample; j++){
        if (ADCdata[j]<0.28){
            //pc.printf("j:%d A:%1.5f\r\n",j,ADCdata[j]);
            if(ADCdata[j+1]>0.28){
                  g[n]=j;
                  //pc.printf("n:%d g:%d\r\n",n,g[n]);
                  n=n+1;
                }
                
            }   
          
         
    }

    y=g[100]-g[99];
    T=y*(1./sample);
    //pc.printf("T:%1.5f",T);
    f=1/T;
    
   for (int d = 0; d < sample; d++){
   pc.printf("%1.3f\r\n", ADCdata[d]);
    wait(0.001);
    }



    //pc.printf("freqeuncy:");
    //pc.printf("%1.5f\r\n",f);
    int ff=round(f);
    int A[3]={0};

    A[0]=ff/100;
    A[1]=ff%100/10;
    A[2]=ff%10;  
    
  
  


    while(1){
    if( Switch == 0 ){
        redLED = 0;
        greenLED = 1;
    }
    else{
        greenLED = 0;
        redLED = 1;
        }
    if(Switch == 0){
       for (int k = 0; k<3; k = k+1){
           display = table[A[k]];
           if(k==2){
                dot=1;
                }
        wait(1);
        }
    }
    else{
        display =0;
    }
    float i;
    for( i=0; i<0.7; i+=0.0001 ){
      Aout = 0.5 + 0.5*sin(2*ff*i*3.14159);
      wait(0.0001);
    }
    }
    
    

}
