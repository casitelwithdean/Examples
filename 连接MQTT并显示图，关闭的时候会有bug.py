#  *_* coding:utf8 *_*
import tkinter
import threading
import paho.mqtt.client as mqtt
import matplotlib
matplotlib.use('TkAgg')
import matplotlib.pyplot as plt
from tkinter import *

import hashlib
import time
import matplotlib as mt
import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg,NavigationToolbar2Tk

dick1 = "0"
HOST = "2id04vf.mqtt.iot.gz.baidubce.com"
PORT = 1883
username = "2id04vf/esp32"
password = "JLkTNyinRfSt5tYt"
mqttClient = mqtt.Client()
tt = 3
fig, ax = plt.subplots()
init_window = Tk()
LOG_LINE_NUM = 0
wo="555"
class MY_GUI():
    def __init__(self,init_window_name):
        self.init_window_name = init_window_name


    #设置窗口
    def set_init_window(self):
        self.init_window_name.title("文本处理工具_v1.2")           #窗口名
        #self.init_window_name.geometry('320x160+10+10')                         #290 160为窗口大小，+10 +10 定义窗口弹出时的默认展示位置
        self.init_window_name.geometry('1068x681+10+10')
        #self.init_window_name["bg"] = "pink"                                    #窗口背景色，其他背景色见：blog.csdn.net/chl0000/article/details/7657887
        #self.init_window_name.attributes("-alpha",0.9)                          #虚化，值越小虚化程度越高
        #标签
        self.init_data_label = Label(self.init_window_name, text="待处理数据")
        self.init_data_label.grid(row=0, column=0)

        self.L1 = Label(self.init_window_name, text="主机名")
        self.L1.grid(row=3, column=0)
        self.L1 = Label(self.init_window_name, text="端口")
        self.L1.grid(row=4, column=0)
        self.L1 = Label(self.init_window_name, text="用户名")
        self.L1.grid(row=5, column=0)
        self.L1 = Label(self.init_window_name, text="密码")
        self.L1.grid(row=6, column=0)
        self.E1 = Entry(self.init_window_name, bd=5)
        self.E1.grid(row=3, column=3)
        self.E1.insert(0, HOST)
        self.E2 = Entry(self.init_window_name, bd=5)
        self.E2.grid(row=4, column=3)
        self.E2.insert(0,PORT )
        self.E3 = Entry(self.init_window_name, bd=5)
        self.E3.grid(row=5, column=3)
        self.E3.insert(0, username)
        self.E4 = Entry(self.init_window_name, bd=5)
        self.E4.grid(row=6, column=3)
        self.E4.insert(0, password)
       # top.mainloop()

        #文本框

        # self.log_data_Text = Text(self.init_window_name, width=66, height=9)  # 日志框
        # self.log_data_Text.grid(row=23, column=0, columnspan=10)
        #按钮
        self.str_trans_to_md5_button = Button(self.init_window_name, text="显示图像", bg="lightblue", width=10,command=huatu)  # 调用内部方法  加()为直接调用
        self.str_trans_to_md5_button.grid(row=1, column=11)

        self.str_trans_to_md5_button = Button(self.init_window_name, text="连接", bg="lightblue", width=10,
                                          command=self.gettext1)  # 调用内部方法  加()为直接调用
        self.str_trans_to_md5_button.grid(row=3, column=11)

    #功能函数
    def gettext1(self):
      abc=self.E1.get()
      print(abc)

      abc=self.E2.get()
      print(abc)

      abc=self.E3.get()
      print(abc)

      abc = self.E4.get()
      print(abc)

      on_mqtt_connect()
      on_subscribe()
    #获取当前时间
    def get_current_time(self):
        current_time = time.strftime('%Y-%m-%d %H:%M:%S',time.localtime(time.time()))
        return current_time


def gui_start():
    global  init_window    #实例化出一个父窗口
    ZMJ_PORTAL = MY_GUI(init_window)
    # 设置根窗口默认属性
    ZMJ_PORTAL.set_init_window()

    init_window.mainloop()          #父窗口进入事件循环，可以理解为保持窗口运行，否则界面不展示

huatu = 0

# 连接MQTT服务器
def on_mqtt_connect():
    mqttClient.username_pw_set(username, password)
    mqttClient.connect(HOST, PORT, 60)
    mqttClient.loop_start()


# publish 消息
def on_publish(topic, payload, qos):
    mqttClient.publish(topic, payload, qos)


def huatu():
    global ax
    global tt
    global dick1
    global wo
    global huatu
    # mt.use('Agg')
    ax.plot(y1, color='r')


# (side=tkinter.TOP, fill=tkinter.BOTH, expand=1)
    while True:

      tt=tt+1
      for i in range(tt):
        y1.append(int(dick1))
        ax.cla()
        ax.set_title("Loss")
        ax.set_xlabel("Iteration")
        ax.set_ylabel("Loss")
        ax.plot(y1, label='train')

        plt.pause(0.1)
      print(plt.get_fignums())
      # if(plt.get_fignums()!=[1]):
      #     print("zhenshilaoliu")
      #     break
      time.sleep(1)

# 消息处理函数
def on_message_come(client, userdata, msg):
    print(msg.topic + " " + ":" + str(msg.payload))
    print(type(msg))
    global tt
    global huatu
    global y1
    global dick1
    #tt = tt + 1
    huatu=1
    dick = str(msg.payload)
    dick1 = dick[2:len(dick) - 1]


# subscribe 消息
def on_subscribe():
    mqttClient.subscribe("onoff", 1)
    mqttClient.on_message = on_message_come  # 消息到来处理函数


def main():
    on_mqtt_connect()
    # on_publish("chat", "Hello Python!", 1)
    on_subscribe()
    while True:
        on_publish("onoff", "8888", 1)
        huatu()
        time.sleep(1)
        # pass


if __name__ == '__main__':

    y1 = []



    gui_start()

    huatu()
    #main()

