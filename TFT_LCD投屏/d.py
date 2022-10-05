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


# img = Image.open("D:\python11\pic_pyautogui.jpg")
#cropped = img.crop((0, 0, 128, 128))  # (left, upper, right, lower)
#cropped.save("D:\python11\pic_pyautogui.jpg")

img = cv2.imread("D:/desktop/1163557.jpg")
# x, y = img.shape[0:2]//获取高和宽


img_test1 = cv2.resize(img, (120,120))
# params = [cv2.IMWRITE_JPEG_QUALITY,30]  # ratio:0~100
cv2.imwrite('D:/desktop/1163557.jpg',img_test1,[int(cv2.IMWRITE_JPEG_QUALITY),50])

