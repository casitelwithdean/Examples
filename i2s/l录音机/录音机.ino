#include "driver/i2s.h"
#include "WAV_C.h"
                                const i2s_port_t I2S_PORT = I2S_NUM_0;
#define LEDC_CHANNEL_0 0
#define LEDC_TIMER_12_BIT 10
#define LEDC_BASE_FREQ 16000
#define LED_PIN 18

void setup()
{
    Serial.begin(115200);
    esp_err_t err;

    // The I2S config as per the example
    const i2s_config_t i2s_config = {
        .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX), // Receive, not transfer
        .sample_rate = 16000,                              // 16KHz
        .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,      // could only get it to work with 32bits
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,       // use right channel
        .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1, // Interrupt level 1
        .dma_buf_count = 4,                       // number of buffers
        .dma_buf_len = 8                          // 8 samples per buffer (minimum)//至少8最多1024
    };

    // The pin config as per the setup
    const i2s_pin_config_t pin_config = {
        .bck_io_num = 26,                  // Serial Clock (SCK)
        .ws_io_num = 25,                   // Word Select (WS)
        .data_out_num = I2S_PIN_NO_CHANGE, // not used (only for speakers)
        .data_in_num = 33                  // Serial Data (SD)
    };

    err = i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
    if (err != ESP_OK)
    {
        Serial.printf("Failed installing driver: %d\n", err);
        while (true)
            ;
    }
    err = i2s_set_pin(I2S_PORT, &pin_config);
    if (err != ESP_OK)
    {
        Serial.printf("Failed setting pin: %d\n", err);
        while (true)
            ;
    }
    sigmaDeltaSetup(18, 0, 312500);
    sigmaDeltaWrite(0, 0);
    sigmaDeltaSetup(19, 1, 312500);
    sigmaDeltaWrite(1, 128);

    delay(1000);
    Serial.println(millis());
    for (int i = 0; i < SOUND_LENGTH; i++)
    {
        sigmaDeltaWrite(0, DATA[i]);
        //  Serial.println(DATA[i]);//由此可以看出串口打印是非常耗时的，每次打印都需要356us的时间  20662-1025//3895-1025
        delayMicroseconds(22);
    }
    Serial.println(millis());
}
int16_t record[16000];
int32_t sBuffer[8];
void loop()
{
      Serial.println("开始录音");
     Serial.println(micros());
    for(int a=0;a<16000;a++)
    {
    size_t bytes_read = 0;
    i2s_read(I2S_PORT, sBuffer, sizeof(sBuffer), &bytes_read, portMAX_DELAY); //采样率是16000  所以写进去一个buffer就是1/16000s，那么写进八个buffer就是8/16000=500us。所以执行一句i2s_read就会耗时500us，八个数组都会有数据/所以如果想用此存储录音的话，那么要么提高采样频率，要么就每次采集都使用8个数组，但这样会导致数组过大。
    //另外2khz的音质好像也并没有太差，还是可以勉强接受的
    if (bytes_read > 0)
    {
        int16_t xx = (sBuffer[6] >> 22);//32位数据  但是麦克风的24位是在高位的  最高位是符号位
        xx = xx + 128;
        //record[a]=xx;
        sigmaDeltaWrite(0, xx);
    }
    delayMicroseconds(62);
    }
    Serial.println("开始播放");
    // delay(2);
     Serial.println(micros());
     for(int d=0;d<16000;d++)
     {
       sigmaDeltaWrite(0, record[d]);
     delayMicroseconds(62);
     }
}