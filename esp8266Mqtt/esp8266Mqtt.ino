/*
    项目名称：ESP8266MQTT库示例
    版本号：v1.0
    修改时间：2019.03.19
    使用开发板：WiFiduino（Arduino UNO+ESP8266）
    知识产权归 InTron™版权所有©保留权力。
*/

#include "ESP8266MQTT.h"

void onConnectionEstablished();

ESP8266MQTT client(
  "HONOR30",             // Wifi ssid
  "123456789",             // Wifi password
  "2id04vf.mqtt.iot.gz.baidubce.com",    // MQTT broker ip
  1883,               // MQTT broker port
  "2id04vf/esp32",              // MQTT username
  "JLkTNyinRfSt5tYt",         // MQTT password
  "DeviceId-0yh1pkghir",            // Client name
  onConnectionEstablished, // Connection established callback
  true,               // Enable web updater
  true                // Enable debug messages
);



void setup()
{
  Serial.begin(115200);
}



void onConnectionEstablished()
{
  // 订阅主题并且将该主题内收到的消息通过串口发送
  client.subscribe("onoff", [](const String &payload) {
    Serial.println(payload);//此处可以编写一个函数来代替
  });

  // 向某个主题发送消息
  client.publish("onoff", "This is a message");

}

void loop()
{
  client.loop();
  while (Serial.available()>0)

  {

String datafd= Serial.readString();
Serial.println(datafd);//此处可以编写一个函数来代替
    delay(2);
      char *c = &datafd[0];
if(strcmp(c,"666")==0)
{
Serial.println("6666666666666666666666666666666666666");//此处可以编写一个函数来代替
}
  }
//Serial.println("not exact loop");
}
