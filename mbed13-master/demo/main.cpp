#include "mbed.h"

#include "bbcar.h"

#include "bbcar_rpc.h"


Ticker servo_ticker;

PwmOut pin9(D9), pin8(D8);

Serial xbee(D12, D11);

BBCar car(pin8, pin9, servo_ticker);

void demoup(Arguments *in, Reply *out);
RPCFunction rpcAcc(&demoup, "demoup");

void demoup(Arguments *in, Reply *out) {
        car.goStraightCalib(5);

        wait(5);

        car.stop();

        wait(0.1);

        car.goStraightCalib(-5);

        wait(5);

        car.stop();

        wait(5);

    
}

int main() {

    double pwm_table0[] = {-150, -120, -90, -60, -30, 0, 30, 60, 90, 120, 150};

    double speed_table0[] = {-10.445, -9.812, -9.647, -9.408, -5.900, 0.000, 5.900, 10.843, 11.880, 11.401, 12.199};

    double pwm_table1[] = {-150, -120, -90, -60, -30, 0, 30, 60, 90, 120, 150};

    double speed_table1[] = {-10.445, -9.812, -9.647, -9.408, -5.900, 0.000, 5.900, 10.843, 11.880, 11.401, 12.199};

    car.setCalibTable(11, pwm_table0, speed_table0, 11, pwm_table1, speed_table1);
    char buf[256], outbuf[256];

    while (1) {

        for( int i=0; ;i++ ) {

            buf[i] = xbee.getc();

            if(buf[i] == '\n') break;

        }


        RPC::call(buf, outbuf);

        xbee.printf("%s\r\n", outbuf);

    }

}