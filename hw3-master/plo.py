
import matplotlib.pyplot as plt

import numpy as np

import serial

import time


F = 100;  # sampling rate



X = np.arange(0,10,0.1) # signal vector; create Fs samples
Y = np.arange(0,10,0.1)
Z = np.arange(0,10,0.1)
TT = np.arange(0,10,0.1)
t = np.arange(0,10,0.1)



serdev = '/dev/ttyACM0'

s = serial.Serial(serdev,baudrate=115200)


for x in range(0, int(F)):
    
    line=s.readline() # Read an echo string from K66F terminated with '\n'

    X[x] = float(line)
    
   


for y in range(0, int(F)):
    
    line=s.readline() # Read an echo string from K66F terminated with '\n'

    Y[y] = float(line)
    
  
    

for z in range(0, int(F)):
    
    line=s.readline() # Read an echo string from K66F terminated with '\n'

    Z[z] = float(line)
    
   
for tt in range(0, int(F)):
    line=s.readline() # Read an echo string from K66F terminated with '\n'

    TT[tt] = int(line)





fig, ax = plt.subplots(2, 1)

ax[0].plot(t,X,label='X')
ax[0].plot(t,Y,label='Y')
ax[0].plot(t,Z,label='Z')
leg = ax[0].legend(loc='upper right')
ax[0].set_xlabel('Time')

ax[0].set_ylabel('Acc Vector')


ax[1].plot(t,TT) # plotting the spectrum

ax[1].set_xlabel('Time')

ax[1].set_ylabel('Tilt')

plt.show()

s.close()
