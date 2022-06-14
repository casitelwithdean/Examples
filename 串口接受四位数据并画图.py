import matplotlib.pyplot as plt
import serial
from time import sleep
import os
#  *_* coding:utf8 *_*
from pywinauto.keyboard import send_keys
from pywinauto import Application
import time
from tkinter import *







fig, ax = plt.subplots()
y1 = []

def say_hi():
    global E1
    global E2
    DD = E2.get()
    print(DD)
    global serial
    ad = E1.get()
    d=115200
    serial = serial.Serial("COM"+ad,DD, timeout=0.5)
    if serial.isOpen():
        print("serial open success")
    else:
        print("serial open failed")
    while True:
        data = recv(serial)
        print(data)  # str
        y1.append(int(data))#注意接受的是字符串 最好转成整形数据

       # plt.rcParams['figure.autolayout'] = True
        ax.cla()

        #ax.set_xlim(0, 10)  # 有时候x轴不会从0显示，使得折线图和y轴有间隙
        ax.set_ylim(0, 3300)
        ax.set_title(data)
        if(int(data)>1000):
            print("dadadada")


            ax.plot(y1, color="red")
        else:
            ax.plot(y1, color="blue")
        plt.pause(0.1)
def recv(serial):
    while True:
        data = serial.read_all().decode()  # str
        if data == '':
            continue
        else:
            break
        sleep(0.02)
    return data



root = Tk()
L0 = Label(root, text="微处理器作业")
L0.pack(padx=5, pady=5)
L1 = Label(root, text="端口")
L1.pack(padx=10, pady=10)
E1 = Entry(root, bd=5)
E1.pack(padx=25, pady=25)
L2 = Label(root, text="波特率")
L2.pack(padx=30, pady=30)
E2 = Entry(root, bd=5)
E2.pack(padx=50, pady=50)
frame1 = Frame(root)
frame2 = Frame(root)
root.title("tkinter frame")

label = Label(frame1, text="输入串口号，例如COM3，只需输入3即可", justify=LEFT)
label.pack(side=LEFT)

hi_there = Button(frame2, text="开始画图", command=say_hi)
hi_there.pack()

frame1.pack(padx=1, pady=1)
frame2.pack(padx=10, pady=10)

root.mainloop()

