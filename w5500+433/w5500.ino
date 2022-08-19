//先修改w5100.cpp里面的spi引脚

#include <Ethernet.h>
#include <EthernetUdp.h>
//HardwareSerial mySerial1(1);
#include "EEPROM.h"
#include <ArduinoJson.h>
char udp_reci[600]={0};
char kw_udp[70]={0};
char tj_data[90]={0};
int wendu = 30;
int ID_HIGH = 0;
int ID_LOW = 0;
int address = 0;
int cnt = 0;
int ID = 0;
int ID_BASE[600] = {0};
int kw_state[600] = {0};
int num = 0;
int dayin = 0;
int yicun = 0;
int yicun_0 = 0;
char udp_data[99]={0};
int Socket_ID=0;
int LED_RED_PIN[10]={42,35,48,21,13,11,9,3,19,16};
int LED_GREEN_PIN[10]={41,45,47,14,12,10,46,20,8,15};
char cmd[100] = "cmd";
//char doorFloor[20]={0};
int doorFloor=-1;
char doorName[20]={0};
int tijiao=0;
int gengxin=0;
int work=0;
int zidong=0;
char json_num[]={0};
char boyorgirl[]={0};
#define UDP_MAX_SIZE 512
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress server(39, 97, 216, 195); // numeric IP for Google (no DNS)
IPAddress udp_ip(192, 168, 0, 102);
unsigned int localPort = 8802;      // local port to listen on
char packetBuffer[UDP_MAX_SIZE];  // buffer to hold incoming packet,
char ReplyBuffer[] = "acknowledged";        // a string to send back
EthernetUDP Udp;
EthernetClient client;
char socket_data[300] = {0};

void setup() {
  Serial.begin(9600);
  for(int i=0;i<10;i++)
  {
    pinMode(LED_RED_PIN[i],OUTPUT);
    pinMode(LED_GREEN_PIN[i],OUTPUT);
  }

   for(int i=0;i<10;i++)
  {

    digitalWrite(LED_RED_PIN[i],0);
    digitalWrite(LED_GREEN_PIN[i],0);
  }

// mySerial1.begin(9600,SERIAL_8N1,47,45); //47是RX
  Ethernet.begin(mac);
//sprintf(socket_data, "{\"id\":2,\"doorFloor\":6,\"doorName\":\"\u5f85\u914d\u7f6e\",\"doorType\":\"boy\",\"kenNum\":5,\"nowNum\":2}");
//sprintf(udp_data,"{\"cmd\":\"suozhuantaiAll\",\n\"wendu\":\"%d\",\n\"shidu\":\"40\",\n\"state\":{\"1\":0,\"2\":0,\"3\":0,\"4\":0,\"5\":0}\n}",wendu);

  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
eeprom_setup();
  // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    while (true) {
      delay(1); // do nothing, no point running without Ethernet hardware
    }
  }
  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Ethernet cable is not connected.");
  }
  // start UDP
  Udp.begin(localPort);
  Serial.print("My IP address: ");
  Serial.println(Ethernet.localIP());
  udp_ip = Ethernet.localIP();
  udp_ip[3]=255;//修改UDP服务器
  Serial.print("My IP address: ");
  Serial.println(udp_ip);
  if (client.connect(server, 6011)) {
    Serial.print("connected to ");
    Serial.println(client.remoteIP());
  } else {
    Serial.println("connection failed");
  }
  work=1;
}
void udp_send(char udp_buffer[])
{
       Udp.beginPacket(udp_ip,8802);
       Udp.write(udp_buffer);
       Udp.endPacket();
}
  

void loop() {

  int packetSize = Udp.parsePacket();
  if (packetSize) {
    Serial.print("Received packet of size ");
    Serial.println(packetSize);
    Udp.read(packetBuffer, UDP_MAX_SIZE);
    Serial.println(packetBuffer);
    sprintf(udp_reci,"%s",packetBuffer);
    delay(2);
  StaticJsonDocument<400> doc;
  DeserializationError error = deserializeJson(doc,udp_reci);
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }
   Serial.print("udp数据：");
   Serial.println(udp_reci);
   Serial.println((const char*)doc["cmd"]);
    if ((const char*)doc["cmd"]) {
      Serial.print("解析正常");
        Serial.println((const char*)doc["cmd"]);
      sprintf(cmd,"%s",(const char*) doc["cmd"]);
      if (strcmp(cmd, "peizhi") == 0)
      {
        doorFloor=(int) doc["doorFloor"];
        
        Socket_ID=(int) doc["id"];
        sprintf(doorName,"%s",(const char*) doc["name"]);
        sprintf(boyorgirl,"%s",(const char*) doc["boy"]);
     address = 0;
     EEPROM.writeInt(address, Socket_ID);
     address = sizeof(int);
     EEPROM.writeInt(address, doorFloor);
     address = 2*sizeof(int);
     EEPROM.writeString(address, doorName);
     address = 2*sizeof(int)+10;
     EEPROM.writeString(address, boyorgirl);
              EEPROM.commit();
      }
       if (strcmp(cmd, "gengxin") == 0)
       {
        address = 2*sizeof(int)+20;
        num=doc.size()-1;
        EEPROM.writeInt(address, num);
      
        for(int z=0;z<num;z++)
          { 
            
             sprintf(json_num,"%d",z+1);
             ID_BASE[z]=(int) doc[json_num];
             Serial.println(ID_BASE[z]);
             address = (z+3) * sizeof(int)+20;
             EEPROM.writeInt(address, ID_BASE[z]);
          }  
        EEPROM.commit();
       }
      
      if (strcmp(cmd,"zidong")==0){Serial.println(cmd);zidong=1;tijiao=0;work=0;}
      if (strcmp(cmd,"tijiao")==0)tijiao=1;
      if (strcmp(cmd,"work")==0){Serial.println(cmd);zidong=0;tijiao=0;work=1;}

    }
    memset(packetBuffer, 0, sizeof(packetBuffer));
  }
  if(work==1)work_mode();
  if(zidong==1)auto_peidui();
  if(tijiao==1){tj_udp();tijiao=0;}
  delay(10);
}


void eeprom_setup() {
  if (!EEPROM.begin(1000)) {
    Serial.println("Failed to initialise EEPROM  Restarting...");
    delay(1000);
    ESP.restart();
  }
  address=0;
  Socket_ID=EEPROM.readInt(address);
  address=sizeof(int);
  doorFloor=EEPROM.readInt(address);
  address =2*sizeof(int);
  sprintf(doorName,"%s",EEPROM.readString(address));
  address =2*sizeof(int)+10;
  sprintf(boyorgirl,"%s",EEPROM.readString(address));

  address =2*sizeof(int)+20;
 
  num = EEPROM.readInt(address);
  Serial.print("num:");
  Serial.println(num);
  if (num < 0)num = 0;
  for (int i = 0; i < num; i++)
  {
    address += sizeof(int);
    ID_BASE[i] = EEPROM.readInt(address);
    Serial.print(i);
    Serial.print(":");
    Serial.println(ID_BASE[i]);
  }
}


void auto_peidui() {
  while (Serial.available() > 0)
  {
    yicun = 0;
    yicun_0 = 0;
    uint8_t data = Serial.read();
    if (data == 0x7E && cnt == 0)  cnt = 1;
    if (data == 0x08 && cnt == 1)  cnt = 2;
    if (data == 0x0D && cnt == 2)  {
      cnt = 3;
      break;
    }
    if (cnt == 3)   {
      Serial.print("认证ID:");
      Serial.print(data);
      Serial.print(" ");
      cnt = 4;
      ID_HIGH = data;
      break;
    }
    if (cnt == 4)   {
      Serial.println(data); ID = ID_HIGH * 256 + data;
      for (int i = 0; i < num; i++) {
        if (ID == ID_BASE[i])yicun = 1;
      }
      if (yicun == 0) {
        ID_BASE[num++] = ID_HIGH * 256 + data;
        address = 2*sizeof(int)+20;
        EEPROM.writeInt(address, num);
        address = address + num * sizeof(int);
        EEPROM.writeInt(address, ID_BASE[num - 1]);
        EEPROM.commit();
      }
      yicun = 0;
      cnt = 5;
      break;
    }
    if (cnt == 5 || cnt == 6 || cnt == 7 || cnt == 8 || cnt == 9) cnt++;
    if (cnt == 10)    {
      if (data == 0x08) Serial.println("按下");
      if (data == 0x04) Serial.println("松开");  cnt = 0;
    }
  }
  dayin++;
  if (dayin == 300)
  {
    for (int d = 0; d < num; d++)
    { Serial.print(d);
      Serial.print(":");
      Serial.println(ID_BASE[d]);
    }
    dayin = 0;
  }
  delay(1);
}
int dijige=-1;//坑位-1
void work_mode() {

  while (Serial.available() > 0)
  {
    uint8_t data = Serial.read();
    if (data == 0x7E && cnt == 0)  cnt = 1;
    if (data == 0x08 && cnt == 1)  cnt = 2;
    if (data == 0x0D && cnt == 2)  {
      cnt = 3;
      continue;
    }
    if (cnt == 3)   {
      Serial.print("认证ID:");
      Serial.print(data);
      Serial.print(" ");
      cnt = 4;
      ID_HIGH = data;
     continue;
    }
    if (cnt == 4)   {
      Serial.println(data); ID = ID_HIGH * 256 + data;
      for (int i = 0; i < num; i++) {
        if (ID == ID_BASE[i]){Serial.println("出现:");Serial.println(i);dijige=i;Serial.println(dijige);}
      }

      cnt = 5;
      continue;
    }
    if (cnt == 5 || cnt == 6 || cnt == 7 || cnt == 8 || cnt == 9) cnt++;
    if (cnt == 10)    {
    Serial.println("总人数:");
    Serial.println(num);
  
      if (data == 0x08) {Serial.println("按下"); if(dijige!=-1)kw_state[dijige]=1;digitalWrite(LED_GREEN_PIN[dijige],1);digitalWrite(LED_RED_PIN[dijige],0);}
      if (data == 0x04) {Serial.println("松开"); if(dijige!=-1)kw_state[dijige]=0;digitalWrite(LED_GREEN_PIN[dijige],0);digitalWrite(LED_RED_PIN[dijige],1);}
      cnt = 0;
    }

  }
  if(dijige!=-1)
  {
sprintf(kw_udp,"{");
for(int w=0;w<num;w++)
      {
        if(w==0){sprintf(kw_udp,"%s\"%d\":%d",kw_udp,w+1,kw_state[w]);}
       else {sprintf(kw_udp,"%s,\"%d\":%d",kw_udp,w+1,kw_state[w]);}
      }

      sprintf(udp_data,"{\"cmd\":\"suozhuantaiAll\",\"wendu\":\"25\",\"shidu\":\"50\",\"state\":%s}}",kw_udp);
      Serial.println("UDP传输的数据");
      Serial.println(udp_data);
      udp_send(udp_data);
      int nownum=0;
      for(int c=0;c<num;c++)
      { nownum=kw_state[c]+nownum;}//\u5f85\u914d\u7f6e
  sprintf(socket_data, "{\"id\":%d,\"doorFloor\":%d,\"doorName\":\"%s\",\"doorType\":\"%s\",\"kenNum\":%d,\"nowNum\":%d}",Socket_ID,doorFloor,doorName,boyorgirl,num,nownum);
client.print(socket_data);
      
      dijige=-1;
  }
  
}

void tj_udp()
{
  sprintf(tj_data,"{\"cmd\":\"tijiao\",");
  for(int i=0;i<num;i++)
    {
   if(i==0){ sprintf(tj_data,"%s\"%d\":%d",tj_data,i+1,ID_BASE[i]);}
    else {sprintf(tj_data,"%s,\"%d\":%d",tj_data,i+1,ID_BASE[i]);}      
    }
    sprintf(tj_data,"%s}",tj_data);
    udp_send(tj_data);
}
