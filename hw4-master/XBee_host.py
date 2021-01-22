import serial
import time
import numpy as np
import matplotlib.pyplot as plt

# XBee setting
serdev = '/dev/ttyUSB0'
s = serial.Serial(serdev, 9600)
s.write("+++".encode())
char = s.read(2)
print("Enter AT mode.")
print(char.decode())

s.write("ATMY 0x140\r\n".encode())
char = s.read(3)
print("Set MY 0x140.")
print(char.decode())

s.write("ATDL 0x240\r\n".encode())
char = s.read(3)
print("Set DL 0x240.")
print(char.decode())

s.write("ATID 0x1\r\n".encode())
char = s.read(3)
print("Set PAN ID 0x1.")
print(char.decode())

s.write("ATWR\r\n".encode())
char = s.read(3)
print("Write config.")
print(char.decode())

s.write("ATMY\r\n".encode())
char = s.read(4)
print("MY :")
print(char.decode())

s.write("ATDL\r\n".encode())
char = s.read(4)
print("DL : ")
print(char.decode())

s.write("ATCN\r\n".encode())
char = s.read(3)
print("Exit AT mode.")
print(char.decode())

s = serial.Serial(serdev, 9600, timeout = 2)
print("start sending RPC")
count = 0
A = np.zeros(20)
while count < 21:
    s.write("/cc/run\r".encode())
    line = s.readline()
    if(line == b''):
        continue
    num = line.split(b"\r\n")
    if count>0:
        zz=int(num[0])
        A[count - 1] = zz
        print('%d: number: %d' %(count - 1, A[count - 1]))
    count = count + 1
    time.sleep(1)
s.close()


x = np.linspace(0, 20, 20)
plt.figure()
plt.plot(x, A, 'g')
plt.title("collected data plot")
plt.xlabel("timestamp")
plt.ylabel("number")
plt.show()