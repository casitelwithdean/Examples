import serial
from time import sleep
import os
#  *_* coding:utf8 *_*
from pywinauto.keyboard import send_keys
from pywinauto import Application
import time

def recv(serial):
    while True:
        data = serial.read_all().decode()  # str
        if data == '':
            continue
        else:
            break
        sleep(0.02)
    return data

if __name__ == '__main__':
    serial = serial.Serial('COM3', 115200, timeout=0.5)
    if serial.isOpen():
        print("serial open success")
    else:
        print("serial open failed")
    while True:
        data = recv(serial)
        print(data)  # str
        if(data=="4"):
            send_keys('^%{LEFT}')
        if (data == "6"):
                send_keys('^%{RIGHT}')
        if (data == "5"):
                send_keys('^%{F5}')
        if (data == "8"):
            send_keys('^%{DOWN}')
            send_keys('^%{DOWN}')
            send_keys('^%{DOWN}')
            send_keys('^%{DOWN}')
            send_keys('^%{DOWN}')
            #os.system('"C:\Program Files (x86)\Microsoft\Edge\Application\msedge.exe" https://www.baidu.com')