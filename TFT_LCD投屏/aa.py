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

img = pyautogui.screenshot()
img.save(r'D:\python11\pic_pyautogui.jpg')
# img = Image.open("D:\python11\pic_pyautogui.jpg")
# cropped = img.crop((0, 0, 128, 128))  # (left, upper, right, lower)
# cropped.save("D:\python11\pic_pyautogui.jpg")

img = cv2.imread("D:\python11\pic_pyautogui.jpg")
x, y = img.shape[0:2]
print(x)
print(y)
# import socket
#
# # AF_INET 表示使用IPv4, SOCK_DGRAM 则表明数据将是数据报(datagrams)
# udp_client = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
#
# client_msg = 'Hello udp server.'
#
# udp_client.sendto(client_msg.encode('utf8'), ('192.168.43.98', 1234))
#
# while True:
#     rec_msg, addr = udp_client.recvfrom(1024)
#     print('msg form server:', rec_msg.decode('utf8'))
