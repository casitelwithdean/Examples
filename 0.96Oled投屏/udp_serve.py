from socket import *
from time import ctime
import pyautogui
import cv2
import numpy as np
import binascii
import time
from tkinter import *
import tkinter.filedialog
from PIL import Image

import PIL.ImageTk

bw = 0
f = 0
sum = 0
bitmap = ""
def imagex():
        img = pyautogui.screenshot()
        img.save(r'D:\python11\pic_pyautogui.jpg')
        img = Image.open("D:\python11\pic_pyautogui.jpg")
        cropped = img.crop((800, 600, 1184, 792))  # (left, upper, right, lower)
        cropped.save("D:\python11\pic_pyautogui.jpg")
        image = PIL.Image.open("D:\python11\pic_pyautogui.jpg")
        pix = image.load()
        thresh =170
        bw=0
        f=0
        sum=0
        bitmap=""
        for y in range(64):
            for x in range(128):
                    f = 2 ** (x % 4)
                    if (pix[3*x, 3*y] > (thresh, thresh, thresh)):
                        bw=0
                    else:
                        bw=1
                    sum=sum+f*bw

                    if (f == 8):

                     sum_hex = hex(sum)[2:]
                     bitmap = bitmap + str(sum_hex)

                     sum = 0
        return  bitmap
host = '' #监听所有的ip
port = 1234 #接口必须一致
bufsize = 1024
addr = (host,port) 

udpServer = socket(AF_INET,SOCK_DGRAM)
udpServer.bind(addr) #开始监听
print('Waiting for connection...')

#bitmap = imagex()
data, addr = udpServer.recvfrom(bufsize)  # 接收数据和返回地址

imgx = cv2.imread("pi4.jpg")
img_encode = cv2.imencode('.jpeg' , imgx)[1]

np.set_printoptions(threshold=np.inf)
while True:


    bitmap=imagex()
    bitmap1 = ";" + bitmap[0:1023]
    bitmap2 = "/" + bitmap[1024:2047]
   # print(bitmap)

    #处理数据
    udpServer.sendto(bitmap1.encode(encoding='utf-8'), addr)
    udpServer.sendto(bitmap2.encode(encoding='utf-8'), addr)

    #发送数据
    #print('...recevied from and return to :',addr)
    time.sleep(0.1)

udpServer.close()