#include "driver/i2s.h"
#include "WAV_C.h"
const i2s_port_t I2S_PORT = I2S_NUM_0;
#define LEDC_CHANNEL_0     0
#define LEDC_TIMER_12_BIT  10
#define LEDC_BASE_FREQ     16000
#define LED_PIN            18


int brightness = 0;    // how bright the LED is
int fadeAmount = 5;    // how many points to fade the LED by

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

  // The pin config as per the setup
  const i2s_pin_config_t pin_config = {
      .bck_io_num = 26,   // Serial Clock (SCK)
      .ws_io_num = 25,    // Word Select (WS)
      .data_out_num = I2S_PIN_NO_CHANGE, // not used (only for speakers)
      .data_in_num = 33   // Serial Data (SD)
  };

  // Configuring the I2S driver and pins.
  // This function must be called before any I2S driver read/write operations.
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
//  Serial.println("I2S driver installed.");
//    sigmaDeltaSetup(18,0, 312500);
//    sigmaDeltaWrite(0, 0);
  ledcSetup(LEDC_CHANNEL_0, LEDC_BASE_FREQ, LEDC_TIMER_12_BIT);
  ledcAttachPin(LED_PIN, LEDC_CHANNEL_0);

  for(int i=0;i<SOUND_LENGTH;i++)
  {
//    sigmaDeltaWrite(0, DATA[i]);
      ledcWrite(LEDC_CHANNEL_0,DATA[i]);
//      Serial.println(DATA[i]*4);
      delayMicroseconds(155);
  }
}
int32_t sBuffer[8];
void loop() {
  // Read a single sample and log it for the Serial Plotter.
  size_t bytes_read = 0;
 i2s_read(I2S_PORT, sBuffer, sizeof(sBuffer), &bytes_read, portMAX_DELAY);//目前可以理解为subffer是接受i2s数据的数组，bytes_read是是否读进去内存，如果读进去那么就是32位，大于0
  if (bytes_read > 0) {
    Serial.println(sBuffer[0]);
  }
  delay(5);

}
