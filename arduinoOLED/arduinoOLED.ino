#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define OLED_RESET     4   //不用改
#define screen_x 128
#define screen_y 32
Adafruit_SSD1306 display(screen_x, screen_y, &Wire,OLED_RESET);
 
void setup() 
{
  display.begin(SSD1306_SWITCHCAPVCC,0x3C);
}
 
void loop() 
{
 words();
 display.display(); // 开显示
}
void words()
{
 display.setTextColor(WHITE);//开像素点发光
  display.clearDisplay();//清屏
  
//  display.setTextSize(1); //设置字体大小  
//  display.setCursor(0,0);//设置显示位置
//  display.println("test");
 
  display.setTextSize(1);//设置字体大小  
  display.setCursor(0,0);//设置显示位置
  display.println("gooaad");
}
