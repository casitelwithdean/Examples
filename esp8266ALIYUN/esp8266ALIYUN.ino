#include <ESP8266WiFi.h>
/* PubSubClient 2.4.0 */
#include <PubSubClient.h>
/* ArduinoJson 5.13.4 */
#include <ArduinoJson.h>
/* DHT sensor library 1.3.0 */
#include "DHT.h"
/* Crypto 0.2.0 */
#include "SHA256.h"

#define DHTPIN 13    // nodeMCU  pin
#define DHTTYPE DHT11   // Define DHT type

#define BAUD_RATE 115200
#define DELAY_TIME 60*1000 //60s interval between updating data

/* 连接您的WIFI SSID和密码 */
#define WIFI_SSID         "HONOR30"
#define WIFI_PASSWD       "123456789"

/* 设备的三元组信息 */
#define PRODUCT_KEY "a1ChnvnTw4u"                        //产品ID
#define DEVICE_NAME "esp32"                     //设备名
#define DEVICE_SECRET "d73ca15f238efa500603eefbc9347d0e" //设备key

#define REGION_ID         "cn-shanghai"

/* IoT物联网平台Endpoint 域名和端口号 */
#define MQTT_SERVER       PRODUCT_KEY ".iot-as-mqtt." REGION_ID ".aliyuncs.com"
#define MQTT_PORT         1883
#define MQTT_USRNAME      DEVICE_NAME "&" PRODUCT_KEY
//用于身份验证的 MQTT_PASSWD和CLIENT_ID
#define CONTENT_STR_FORMAT    "clientIdesp8266deviceName" DEVICE_NAME "productKey" PRODUCT_KEY "timestamp%d"
char  CLIENT_ID[80] = {'\0'};
char * MQTT_PASSWD;

/* topic和payload */
#define PROP_POST_TOPIC "/sys/" PRODUCT_KEY "/" DEVICE_NAME "/thing/event/property/post"
#define BODY_FORMAT     "{\"id\":\"123\",\"version\":\"1.0\",\"method\":\"thing.event.property.post\",\"params\":{\"temperature\":%d,\"humidity\":%d}}"

DHT dht(DHTPIN, DHTTYPE);
WiFiClient espClient;
PubSubClient  client(espClient);

//set up module like main founction
void setup()
{
  Serial.begin(BAUD_RATE);
  //initalize DHT sensor
  dht.begin();

  //生成身份验证的 MQTT_PASSWD和CLIENT_ID
  char hashs[32];
  char buff[128] = {'\0'};

  int timestamp = millis();
  sprintf(CLIENT_ID, "esp8266|securemode=3,signmethod=hmacsha256,timestamp=%d|", timestamp);
  sprintf(buff, CONTENT_STR_FORMAT, timestamp);


  SHA256 sha256;
  sha256.resetHMAC(DEVICE_SECRET, strlen(DEVICE_SECRET));
  sha256.update(buff, strlen(buff));
  sha256.finalizeHMAC(DEVICE_SECRET, strlen(DEVICE_SECRET), hashs, strlen(hashs));

  MQTT_PASSWD = hex_to_str(hashs, sizeof(hashs)) ;

  wifiInit();
}


//this runs over and over
void loop() {

  // if Read temperature as Fahrenheit then set a parameter (isFahrenheit = true)
  int temperature = 11;
  int humidity = 15;

  // Check if any reads failed and exit early (to try again).


    Serial.print("read temperature:");
    Serial.println(temperature);
    Serial.print("read humidity:");
    Serial.println(humidity);

    char jsonBuf[128];

    sprintf(jsonBuf, BODY_FORMAT, temperature, humidity);
    Serial.println(jsonBuf);
    if (client.connected()) {
      boolean d = client.publish(PROP_POST_TOPIC, jsonBuf);
      Serial.print("publish:1=成功，0=失败，Code=");
      Serial.println(d);
    

  }
  client.loop();
  //wait for delay time before attempting to post again
  delay(DELAY_TIME);
}

void wifiInit()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWD);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("WiFi not Connect");
  }

  Serial.println("Connected to AP");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  /* 连接WiFi之后，连接MQTT服务器 */
  client.setServer(MQTT_SERVER, MQTT_PORT);
  client.setCallback(callback);

  mqttCheckConnect();
}

void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  payload[length] = '\0';
  Serial.println((char *)payload);

}

void mqttCheckConnect()
{
  while (!client.connected())
  {
    Serial.println("Connecting to MQTT Server ...");
    Serial.println(MQTT_PASSWD);
    Serial.println(CLIENT_ID);

    if (client.connect(CLIENT_ID, MQTT_USRNAME, MQTT_PASSWD))
    {
      Serial.println("MQTT Connected!");
    }
    else
    {
      Serial.print("MQTT Connect err:");
      Serial.println(client.state());
    }
  }
}


char* hex_to_str(char *data, int len) {

  char *str = ( char*)malloc(2 * len);
  memset(str, 0, 2 * len);

  int i;
  char lo, hi;
  for (i = 0; i < len; i++) {
    hi = (data[i] >> 4) & 0xf;
    lo = (data[i] >> 0) & 0xf;

    if (hi <= 9) {
      str[2 * i] = hi + ('0' - 0);
    } else {
      str[2 * i] = hi + ('A' - 10);
    }

    if (lo <= 9) {
      str[2 * i + 1] = lo + ('0' - 0);
    } else {
      str[2 * i + 1] = lo + ('A' - 10);
    }
  }

  return str;
}
