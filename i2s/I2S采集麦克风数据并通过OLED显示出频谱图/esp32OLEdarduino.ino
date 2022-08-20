#include <Wire.h>
#include "SSD1306.h"
#include "arduinoFFT.h"
#include "driver/i2s.h"
const i2s_port_t I2S_PORT = I2S_NUM_0;
#define samples 64 //采样点数，2的N次幂
#define halfsamples samples/2
#define NumofCopy halfsamples*sizeof(double)
#define Interval 128/(halfsamples)
SSD1306 display(0x3c, 21, 23);
double vReal[samples];
double vImag[samples];
double vTemp[halfsamples];

arduinoFFT FFT = arduinoFFT();
void setup() {
  Serial.begin(115200);
   esp_err_t err;

  // The I2S config as per the example
  const i2s_config_t i2s_config = {
      .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX), // Receive, not transfer
      .sample_rate = 16000,                         // 16KHz
      .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT, // could only get it to work with 32bits
      .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT, // use right channel
      .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
      .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,     // Interrupt level 1
      .dma_buf_count = 4,                           // number of buffers
      .dma_buf_len = 8                          // 8 samples per buffer (minimum)
  };
  const i2s_pin_config_t pin_config = {
      .bck_io_num = 26,   // Serial Clock (SCK)
      .ws_io_num = 25,    // Word Select (WS)
      .data_out_num = I2S_PIN_NO_CHANGE, // not used (only for speakers)
      .data_in_num = 33   // Serial Data (SD)
  };
  err = i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
  if (err != ESP_OK) {
    Serial.printf("Failed installing driver: %d\n", err);
    while (true);
  }
  err = i2s_set_pin(I2S_PORT, &pin_config);
  if (err != ESP_OK) {
    Serial.printf("Failed setting pin: %d\n", err);
    while (true);
  }
  display.init();
  display.setFont(ArialMT_Plain_24);
//  display.drawString(0, 0, "Hello World");
  display.display();
  
}
 size_t bytes_read = 0;
 int32_t sBuffer[8];
void loop() {
 for(int i=0;i<samples;i++)
  {
 bytes_read = 0;
   i2s_read(I2S_PORT, sBuffer, sizeof(sBuffer), &bytes_read, portMAX_DELAY);//目前可以理解为subffer是接受i2s数据的数组，bytes_read是是否读进去内存，如果读进去那么就是32位，大于0
  if (bytes_read > 0) {
    vReal[i]=sBuffer[0]>>21;
//    Serial.println(vReal[i]);
    vImag[i] = 0.0;
  }
delayMicroseconds(100);
  }
  display.clear();
  FFT.Windowing(vReal, samples, FFT_WIN_TYP_HAMMING, FFT_FORWARD);//加窗
  FFT.Compute(vReal, vImag, samples, FFT_FORWARD);  //计算fft
  FFT.ComplexToMagnitude(vReal, vImag, samples);  //计算幅度
  for(int i=0;i<halfsamples;i++)
  {
//    display.setPixel(i*Interval,vTemp[halfsamples-i-1]*0.007+1); //显示
    display.drawLine(i*Interval, 0, i*Interval,vReal[halfsamples-i-1]*0.017);
  }
  display.display();
  memcpy(vTemp, vReal, NumofCopy);
  double x = FFT.MajorPeak(vReal, samples,2000);
//  if((int)x>30)
//  Serial.println(x, 6);
  delay(10);  //改为128点时可以注释掉，防止刷新太慢
}
