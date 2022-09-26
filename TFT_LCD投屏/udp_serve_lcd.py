from socket import *
from time import ctime
import pyautogui
import binascii
import time
from tkinter import *
import tkinter.filedialog
from PIL import Image
import PIL.ImageTk
import binascii
import cv2
import numpy as np
def screen():

    img = pyautogui.screenshot()
    img.save(r'D:\python11\pic_pyautogui.jpg')
    # img = Image.open("D:\python11\pic_pyautogui.jpg")
    #cropped = img.crop((0, 0, 128, 128))  # (left, upper, right, lower)
    #cropped.save("D:\python11\pic_pyautogui.jpg")

    img = cv2.imread("D:\python11\pic_pyautogui.jpg")
    # x, y = img.shape[0:2]//获取高和宽

    img_test1 = cv2.resize(img, (240,240))
    params = [cv2.IMWRITE_JPEG_QUALITY,30]  # ratio:0~100
    img_encode = cv2.imencode('.jpeg' , img_test1,params)[1]

    #jpeg_bytes = np.array(img_encode).tobytes()
    str=""
    np.set_printoptions(threshold=np.inf)
    print(len(img_encode))
    for i in range(len(img_encode)):
        dick = hex(img_encode[i])[2:]
        baga = dick.zfill(2)
        str=str+baga
    return str,img_encode
# start = time.time()  # 1657267196.3012242
# print('spend： %s second' % start)
# time.sleep(3)
esp32 = '192.168.43.82' #监听所有的ip
port = 1234 #接口必须一致
bufsize = 1024
addr = (esp32,port)

udpServer = socket(AF_INET,SOCK_DGRAM)
# udpServer.bind(addr) #开始监听
print('Waiting for connection...')
print('Waiting for connection...')
ABC="ABCDEFGHIKLMKOP"
while True:
    str,img_encode=screen()
    udpServer.sendto((";" + str[0:1024]).encode(encoding='utf-8'), addr)
    for i in range(1, int(len(img_encode)*2 / 1024)):
        udpServer.sendto((ABC[i-1:i]+str[i * 1024:i * 1024 + 1024]).encode(encoding='utf-8'), addr)
    udpServer.sendto(("/" + str[int(len(img_encode)*2 / 1024) * 1024:len(img_encode)*2]).encode(encoding='utf-8'), addr)
    time.sleep(0.01)

udpServer.close()