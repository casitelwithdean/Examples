#  *_* coding:utf8 *_*
from tkinter import *
import tkinter
import paho.mqtt.client as mqtt
import time
import matplotlib.pyplot as plt

from pywinauto.keyboard import send_keys
from pywinauto import Application
import time

x = [i for i in range(1, 11)]
y = [1, 2, 3, 4, 5, 6, 7, 8, 9, 0]
huatu = 0
dick1 = "66666"
HOST = "2id04vf.mqtt.iot.gz.baidubce.com"
# "broker.hivemq.com"
PORT = 1883
username = "2id04vf/esp32"
password = "JLkTNyinRfSt5tYt"
mqttClient = mqtt.Client()
tt = 0
fig, ax = plt.subplots()


# def test():
#     client = mqtt.Client()
#     client.username_pw_set(username, password)
#     client.connect(HOST, PORT, 60)
#     client.publish("chat", "hello zzx", 2)
#     client.loop_forever()
#
# while True:
#     test()
#     time.sleep(2)


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
    for i in range(tt):
        y1.append(int(dick1))

        ax.cla()
        ax.set_title("Loss")
        ax.set_xlabel("Iteration")
        ax.set_ylabel("Loss")
        ax.plot(y1, label='train')

        plt.pause(0.1)


# 消息处理函数
def on_message_come(client, userdata, msg):
    print(msg.topic + " " + ":" + str(msg.payload))
    print(type(msg))
    global tt
    global huatu
    global y1
    global dick1
    tt = tt + 1

    dick = str(msg.payload)
    dick1 = dick[2:len(dick) - 1]
    if (dick1 == "4"):
        send_keys('^%{LEFT}')
    if (dick1 == "6"):
        send_keys('^%{RIGHT}')
    if (dick1 == "5"):
        send_keys('^%{F5}')


# huatu()


# print(dick1)
# name=str(msg.payload)
# y[tt]=int(dick1)
# tt=tt+1
# if(tt==10):
# tt=0
# plt.plot (x, y, marker='o' , mfc='w')
# plt.xlim(1,10)
# plt.show()

# subscribe 消息
def on_subscribe():
    mqttClient.subscribe("onoff", 1)
    mqttClient.on_message = on_message_come  # 消息到来处理函数


def main():
    on_mqtt_connect()
    # on_publish("chat", "Hello Python!", 1)
    on_subscribe()
   # while True:
        # on_publish("onoff", "8888", 1)

      #  time.sleep(1)
        # pass

def say_hi():
    print("hello ~ !")


if __name__ == '__main__':
    y1 = []

    main()

    root = Tk()

    frame1 = Frame(root)
    frame2 = Frame(root)
    root.title("tkinter frame")

    label = Label(frame1, text="欢迎来到远程切歌", justify=LEFT)
    label.pack(side=LEFT)

    hi_there = Button(frame2, text="say hi~", command=say_hi)
    hi_there.pack()

    frame1.pack(padx=1, pady=1)
    frame2.pack(padx=10, pady=10)

    root.mainloop()