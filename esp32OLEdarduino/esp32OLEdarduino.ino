#include <Wire.h>
#include "SSD1306.h"

SSD1306 display(0x3c, 21, 23);

void setup() {
     Serial.begin(115200);
        Serial.println("SDA Pin = "+String(SDA));
      Serial.println("SCL Pin = "+String(SCL));
  display.init();
      Serial.println("SDA Pin = "+String(SDA));
      Serial.println("SCL Pin = "+String(SCL));
  display.setFont(ArialMT_Plain_24);
  display.drawString(0, 0, "Hello World");
  display.display();
}

void loop() {
 
}
