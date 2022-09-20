/* esp_timer (high resolution timer) example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "esp_timer.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "esp_err.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "driver/rmt.h"
#include "ir_tools.h"


#define  MAXSPEED  800//范围是0-1023
#define  MINSPEED  350//差速转弯



#define DEFAULT_VREF    1100        //Use adc2_vref_to_gpio() to obtain a better estimate
#define NO_OF_SAMPLES   64          //Multisampling


#define LEDC_LS_MODE           LEDC_HIGH_SPEED_MODE
#define LEDC_HS_TIMER          LEDC_TIMER_0
#define LEDC_HS_MODE           LEDC_HIGH_SPEED_MODE
#define LEDC_HS_CH0_GPIO       (22)
#define LEDC_HS_CH0_CHANNEL    LEDC_CHANNEL_0
#define LEDC_HS_CH1_GPIO       (23)
#define LEDC_HS_CH1_CHANNEL    LEDC_CHANNEL_1

#define LEDC_HS_CH2_GPIO       (4)
#define LEDC_HS_CH2_CHANNEL    LEDC_CHANNEL_2
#define LEDC_HS_CH3_GPIO       (5)
#define LEDC_HS_CH3_CHANNEL    LEDC_CHANNEL_3

static void periodic_timer_callback(void* arg);
static void timer_interrupt1(void* arg);
static void timer_interrupt2(void* arg);

int sensor_LL = 0;  //左左侧红外传感器的检测值，默认值为1，在黑线外
int sensor_L = 1;  //左侧红外传感器的检测值，默认值为1，在黑线上
int sensor_M= 1;
int sensor_R = 1;  //右侧红外传感器的检测值，默认值为1，在黑线上
int sensor_RR = 0;  //右右侧红外传感器的检测值，默认值为0，在黑线外
int Stickx  = 0;  //-1  0  1
int Sticky  = 0; 
int xunji  = 0; 
int hongwai  = 0; 

static const char* TAG = "example";

static esp_adc_cal_characteristics_t *adc_chars;

static const adc_channel_t channel = ADC_CHANNEL_4;     //GPIO32 if ADC1, GPIO14 if ADC2
static const adc_channel_t channel2 = ADC_CHANNEL_6;     //GPIO34 if ADC1, GPIO14 if ADC2
static const adc_bits_width_t width = ADC_WIDTH_BIT_12;  //十二位分辨率

static const adc_atten_t atten = ADC_ATTEN_DB_11;   //选择量程
static const adc_unit_t unit = ADC_UNIT_1;  //ADC1

static rmt_channel_t example_rx_channel = RMT_CHANNEL_2;  //选择红外遥控通道

static void check_efuse(void)//ADC的校准程序
{

    //Check if TP is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK) {
        printf("eFuse Two Point: Supported\n");
    } else {
        printf("eFuse Two Point: NOT supported\n");
    }
    //Check Vref is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_VREF) == ESP_OK) {
        printf("eFuse Vref: Supported\n");
    } else {
        printf("eFuse Vref: NOT supported\n");
    }

}


static void print_char_val_type(esp_adc_cal_value_t val_type)//打印调试信息
{
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
        printf("Characterized using Two Point Value\n");
    } else if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        printf("Characterized using eFuse Vref\n");
    } else {
        printf("Characterized using Default Vref\n");
    }
}


void adcstart(void)//开启ADC
{
    //Check if Two Point or Vref are burned into eFuse
    check_efuse();

    //Configure ADC
    if (unit == ADC_UNIT_1) {
        adc1_config_width(width);
        adc1_config_channel_atten(channel, atten);
    } else {
        adc2_config_channel_atten((adc2_channel_t)channel, atten);
    }

    //Characterize ADC
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(unit, atten, width, DEFAULT_VREF, adc_chars);
    print_char_val_type(val_type);

    //Continuously sample ADC1
    while (1) {
        uint32_t adc_reading = 0;
        uint32_t adc_reading2 = 0;
        //Multisampling
        for (int i = 0; i < NO_OF_SAMPLES; i++) {
            if (unit == ADC_UNIT_1) {
                adc_reading += adc1_get_raw((adc1_channel_t)channel);
                adc_reading2 += adc1_get_raw((adc1_channel_t)channel2);
            } else {
                int raw;
                adc2_get_raw((adc2_channel_t)channel, width, &raw);
                adc_reading += raw;
                adc2_get_raw((adc2_channel_t)channel2, width, &raw);
                adc_reading2 += raw;
            }
        }
        adc_reading /= NO_OF_SAMPLES;
        adc_reading2 /= NO_OF_SAMPLES;
        //Convert adc_reading to voltage in mV
        uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
        uint32_t voltage2 = esp_adc_cal_raw_to_voltage(adc_reading2, adc_chars);
        printf("Raw: %d\tVoltage: %dmV\n", adc_reading, voltage);
        printf("Raw: %d\t111111111111111Voltage: %dmV\n", adc_reading2, voltage2);
        if(adc_reading<1300)//adc范围0-4095
        {
            Stickx=-1;
        }
        else if(adc_reading>2700)
        {
            Stickx=1;
        }
        else 
        {
            Stickx=0;
        }
        if(adc_reading2<1300)
        {
            Sticky=-1;
        }
        else if(adc_reading2>2700)
        {
            Sticky=1;
        }
        else 
        {
            Sticky=0;
        }
        vTaskDelay(2);//这个多任务的延时函数
    }
}

void ledcinit()//PWM输出     一个电机要  2个PWM、 对应4个
{
  ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_TIMER_10_BIT, // resolution of PWM duty
        .freq_hz = 50 ,                      // frequency of PWM signal
        .speed_mode = LEDC_HS_MODE,           // timer mode
        .timer_num = LEDC_HS_TIMER,            // timer index
        .clk_cfg = LEDC_AUTO_CLK,              // Auto select the source clock
    };
    // Set configuration of timer0 for high speed channels
    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel[4] = {

       {

            .channel    = LEDC_HS_CH0_CHANNEL,
            .duty       = 29,//占空比范围0-1023
            .gpio_num   = LEDC_HS_CH0_GPIO,
            .speed_mode = LEDC_HS_MODE,
            .hpoint     = 0,
            .timer_sel  = LEDC_HS_TIMER
        },
        {
            .channel    = LEDC_HS_CH1_CHANNEL,
            .duty       = 29,
            .gpio_num   = LEDC_HS_CH1_GPIO,
            .speed_mode = LEDC_HS_MODE,
            .hpoint     = 0,
            .timer_sel  = LEDC_HS_TIMER
        },
         {

            .channel    = LEDC_HS_CH2_CHANNEL,
            .duty       = 0,
            .gpio_num   = LEDC_HS_CH2_GPIO,
            .speed_mode = LEDC_HS_MODE,
            .hpoint     = 0,
            .timer_sel  = LEDC_HS_TIMER
        },
        {
            .channel    = LEDC_HS_CH3_CHANNEL,
            .duty       = 0,
            .gpio_num   = LEDC_HS_CH3_GPIO,
            .speed_mode = LEDC_HS_MODE,
            .hpoint     = 0,
            .timer_sel  = LEDC_HS_TIMER
        },
        };
        ledc_channel_config(&ledc_channel[0]);
        ledc_channel_config(&ledc_channel[1]);
        ledc_channel_config(&ledc_channel[2]);
        ledc_channel_config(&ledc_channel[3]);//使初始化生效
}
//ch0左轮正极  ch1右轮正极   ch2左轮负极   ch3右轮
void  go_forward_high_speed()
{
 ledc_set_duty(LEDC_HS_MODE, LEDC_HS_CH0_CHANNEL, MAXSPEED);
 ledc_update_duty(LEDC_HS_MODE, LEDC_HS_CH0_CHANNEL);//更新才能生效
    
ledc_set_duty(LEDC_HS_MODE, LEDC_HS_CH1_CHANNEL, MAXSPEED);
 ledc_update_duty(LEDC_HS_MODE, LEDC_HS_CH1_CHANNEL);

  ledc_set_duty(LEDC_HS_MODE, LEDC_HS_CH2_CHANNEL, 0);
 ledc_update_duty(LEDC_HS_MODE, LEDC_HS_CH2_CHANNEL);

 ledc_set_duty(LEDC_HS_MODE, LEDC_HS_CH3_CHANNEL, 0);
 ledc_update_duty(LEDC_HS_MODE, LEDC_HS_CH3_CHANNEL);
}
void  go_forward_right()
{
 ledc_set_duty(LEDC_HS_MODE, LEDC_HS_CH0_CHANNEL, MAXSPEED);
 ledc_update_duty(LEDC_HS_MODE, LEDC_HS_CH0_CHANNEL);

 ledc_set_duty(LEDC_HS_MODE, LEDC_HS_CH1_CHANNEL, MINSPEED);
 ledc_update_duty(LEDC_HS_MODE, LEDC_HS_CH1_CHANNEL);//左轮转的比右轮快

  ledc_set_duty(LEDC_HS_MODE, LEDC_HS_CH2_CHANNEL, 0);
 ledc_update_duty(LEDC_HS_MODE, LEDC_HS_CH2_CHANNEL);

 ledc_set_duty(LEDC_HS_MODE, LEDC_HS_CH3_CHANNEL, 0);
 ledc_update_duty(LEDC_HS_MODE, LEDC_HS_CH3_CHANNEL);
}
void  go_forward_left()
{
 ledc_set_duty(LEDC_HS_MODE, LEDC_HS_CH0_CHANNEL, MINSPEED);
 ledc_update_duty(LEDC_HS_MODE, LEDC_HS_CH0_CHANNEL);

 ledc_set_duty(LEDC_HS_MODE, LEDC_HS_CH1_CHANNEL, MAXSPEED);
 ledc_update_duty(LEDC_HS_MODE, LEDC_HS_CH1_CHANNEL);

 ledc_set_duty(LEDC_HS_MODE, LEDC_HS_CH2_CHANNEL, 0);
 ledc_update_duty(LEDC_HS_MODE, LEDC_HS_CH2_CHANNEL);

 ledc_set_duty(LEDC_HS_MODE, LEDC_HS_CH3_CHANNEL, 0);
 ledc_update_duty(LEDC_HS_MODE, LEDC_HS_CH2_CHANNEL);
}
void  stop_with_brake()
{
 ledc_set_duty(LEDC_HS_MODE, LEDC_HS_CH0_CHANNEL, 0);
 ledc_update_duty(LEDC_HS_MODE, LEDC_HS_CH0_CHANNEL);

 ledc_set_duty(LEDC_HS_MODE, LEDC_HS_CH1_CHANNEL, 0);
 ledc_update_duty(LEDC_HS_MODE, LEDC_HS_CH1_CHANNEL);

 ledc_set_duty(LEDC_HS_MODE, LEDC_HS_CH2_CHANNEL, 0);
 ledc_update_duty(LEDC_HS_MODE, LEDC_HS_CH2_CHANNEL);

 ledc_set_duty(LEDC_HS_MODE, LEDC_HS_CH3_CHANNEL, 0);
 ledc_update_duty(LEDC_HS_MODE, LEDC_HS_CH3_CHANNEL);

}
void  back_up()//倒车
{
 ledc_set_duty(LEDC_HS_MODE, LEDC_HS_CH0_CHANNEL, 0);
 ledc_update_duty(LEDC_HS_MODE, LEDC_HS_CH0_CHANNEL);

 ledc_set_duty(LEDC_HS_MODE, LEDC_HS_CH1_CHANNEL, 0);
 ledc_update_duty(LEDC_HS_MODE, LEDC_HS_CH1_CHANNEL);

 ledc_set_duty(LEDC_HS_MODE, LEDC_HS_CH2_CHANNEL, MAXSPEED);
 ledc_update_duty(LEDC_HS_MODE, LEDC_HS_CH2_CHANNEL);

 ledc_set_duty(LEDC_HS_MODE, LEDC_HS_CH3_CHANNEL, MAXSPEED);
 ledc_update_duty(LEDC_HS_MODE, LEDC_HS_CH3_CHANNEL);

}
void  back_up_left()
{
 ledc_set_duty(LEDC_HS_MODE, LEDC_HS_CH0_CHANNEL, 0);
 ledc_update_duty(LEDC_HS_MODE, LEDC_HS_CH0_CHANNEL);

 ledc_set_duty(LEDC_HS_MODE, LEDC_HS_CH1_CHANNEL, 0);
 ledc_update_duty(LEDC_HS_MODE, LEDC_HS_CH1_CHANNEL);

 ledc_set_duty(LEDC_HS_MODE, LEDC_HS_CH2_CHANNEL, MINSPEED);
 ledc_update_duty(LEDC_HS_MODE, LEDC_HS_CH2_CHANNEL);

 ledc_set_duty(LEDC_HS_MODE, LEDC_HS_CH3_CHANNEL, MAXSPEED);
 ledc_update_duty(LEDC_HS_MODE, LEDC_HS_CH3_CHANNEL);

}
void  back_up_right()
{
 ledc_set_duty(LEDC_HS_MODE, LEDC_HS_CH0_CHANNEL, 0);
 ledc_update_duty(LEDC_HS_MODE, LEDC_HS_CH0_CHANNEL);

 ledc_set_duty(LEDC_HS_MODE, LEDC_HS_CH1_CHANNEL, 0);
 ledc_update_duty(LEDC_HS_MODE, LEDC_HS_CH1_CHANNEL);

 ledc_set_duty(LEDC_HS_MODE, LEDC_HS_CH2_CHANNEL, MAXSPEED);
 ledc_update_duty(LEDC_HS_MODE, LEDC_HS_CH2_CHANNEL);

 ledc_set_duty(LEDC_HS_MODE, LEDC_HS_CH3_CHANNEL, MINSPEED);
 ledc_update_duty(LEDC_HS_MODE, LEDC_HS_CH3_CHANNEL);

}
void IRSEARCH()//循迹
{
 
sensor_LL =gpio_get_level(14);//获取gpio口的电平
sensor_L = gpio_get_level(15);
sensor_M= gpio_get_level(16);
sensor_R = gpio_get_level(17);
sensor_RR =gpio_get_level(18);
 if(sensor_LL==1 && sensor_L==1 && sensor_M==0 && sensor_R==1 && sensor_RR==1)  //小车在黑线上
  {
    go_forward_high_speed();   //前进
  }

else if(sensor_L==0 && sensor_M==1)  //小车右偏
  {
     go_forward_right();  //小车右转100毫秒，时间可调整
 //   usleep(100);
  }
else if(sensor_L==0 && sensor_LL==1)  //小车右偏
  {
     go_forward_right();  //小车右转100毫秒，时间可调整
  //   usleep(100);
  }

else if(sensor_M==1 && sensor_R==0)  //小车左偏
  {
     go_forward_left();   //小车左转100毫秒，时间可调整
  //   usleep(100);
  }
else if(sensor_RR==1 && sensor_R==0)  //小车左偏
  {
     go_forward_left();   //小车左转100毫秒，时间可调整
  //   usleep(100);
  }
else if(sensor_LL==0 && sensor_L==0 && sensor_R==0 && sensor_RR==0)  //小车在停止线上
  {
      if(xunji==1&&hongwai==0)
      {
     stop_with_brake();  //制动2秒
   //  usleep(2000);
      }
  }
}

static void example_ir_rx_task(void *arg)//红外接收  NEC协议
{
    uint32_t addr = 0;
    uint32_t cmd = 0;
    size_t length = 0;
    bool repeat = false;
    RingbufHandle_t rb = NULL;
    rmt_item32_t *items = NULL;

    rmt_config_t rmt_rx_config = RMT_DEFAULT_CONFIG_RX(19, example_rx_channel);
    rmt_config(&rmt_rx_config);
    rmt_driver_install(example_rx_channel, 1000, 0);//驱动安装
    ir_parser_config_t ir_parser_config = IR_PARSER_DEFAULT_CONFIG((ir_dev_t)example_rx_channel);
    ir_parser_config.flags |= IR_TOOLS_FLAGS_PROTO_EXT; // Using extended IR protocols (both NEC and RC5 have extended version)
    ir_parser_t *ir_parser = NULL;

    ir_parser = ir_parser_rmt_new_nec(&ir_parser_config);


    //get RMT RX ringbuffer
    rmt_get_ringbuf_handle(example_rx_channel, &rb);
    assert(rb != NULL);
    // Start receive
    rmt_rx_start(example_rx_channel, true);
    while (1) {
        items = (rmt_item32_t *) xRingbufferReceive(rb, &length, portMAX_DELAY);
        if (items) {
            length /= 4; // one RMT = 4 Bytes
            if (ir_parser->input(ir_parser, items, length) == ESP_OK) {
                if (ir_parser->get_scan_code(ir_parser, &addr, &cmd, &repeat) == ESP_OK) {
                 //ESP_LOGI(TAG, "Scan Code %s --- addr: 0x%04x cmd: 0x%04x", repeat ? "(repeat)" : "", addr, cmd);
                 ESP_LOGI(TAG, "cmd: %d", cmd);//十六进制数据 
                 if(cmd==47430)//前进// 0xba45换成十进制就47430
                 {
                     hongwai=1;//听红外的话
                    go_forward_high_speed();
                    
                 }
                 if(cmd==59925)//后退
                 {
                    hongwai=1;//听红外的话
                    back_up();
                 }
                 if(cmd==47940)//左前
                 {
                     hongwai=1;//听红外的话
              go_forward_right();
                 }
                 if(cmd==48195)//右前
                 {
                     hongwai=1;
                go_forward_left();
                
                 }
                 if(cmd==47685)//刹车
                 {
                      hongwai=0;//不听红外的话
                       stop_with_brake(); 
                 }
                }
            }
            //after parsing the data, return spaces to ringbuffer.
            vRingbufferReturnItem(rb, (void *) items);
        }
        vTaskDelay(1);
    }
    ir_parser->del(ir_parser);
    rmt_driver_uninstall(example_rx_channel);
    vTaskDelete(NULL);
}

void app_main(void)//主程序
{
    /*gpio初始化*/
    gpio_config_t gp;
    gp.intr_type = GPIO_INTR_DISABLE;
    gp.mode = GPIO_MODE_INPUT;//输入模式
    gp.pin_bit_mask = GPIO_SEL_14|GPIO_SEL_15|GPIO_SEL_16|GPIO_SEL_17|GPIO_SEL_18;
    gpio_config(&gp);//使能一下初始化


    const esp_timer_create_args_t periodic_timer_args = {
            .callback = &periodic_timer_callback,//我的中断叫什么名字
            .name = "periodic"//方便你记忆
    };//先创造一个
   const esp_timer_create_args_t periodic_timer_args2 = {
            .callback = &timer_interrupt2,
            .name = "periodic2"
    };


    /* The timer has been created but is not running yet */
    const esp_timer_create_args_t periodic_timer_args1 = {
            .callback = &timer_interrupt1,
            .name = "periodic1"
    };
   esp_timer_handle_t periodic_timer1;
   ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args1, &periodic_timer1));//真正的创造一次
    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
         esp_timer_handle_t periodic_timer2;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args2, &periodic_timer2));
        
    /* Start the timers *///调成100000  就代表100000us
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer2, 100000));//真正的开启
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer1, 50000));
  //  ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, 500000));
   

  
  
    // ESP_ERROR_CHECK(esp_timer_stop(periodic_timer));//定时器停止
    // ESP_ERROR_CHECK(esp_timer_delete(periodic_timer));//删除定时器
    ledcinit();//初始化pwm
xTaskCreate(example_ir_rx_task, "ir_rx_task", 2048, NULL, 10, NULL);

xTaskCreate(&adcstart, "adcstart", 1024 * 5, NULL, 5, NULL);//开启多任务



}

static void periodic_timer_callback(void* arg)
{
    // int64_t time_since_boot = esp_timer_get_time();
    // ESP_LOGI(TAG, "Periodic timer called, time since boot: %lld us", time_since_boot);
}
static void timer_interrupt1(void* arg)
{
    IRSEARCH();
}
static void timer_interrupt2(void* arg)
{

   if(Stickx==0&&Sticky==0)//没动
   {
       stop_with_brake();
       xunji=1;//现在可以循迹
   }
   if(Stickx==0&&Sticky==1)//往前走
   {
       go_forward_high_speed();
       // ESP_LOGI(TAG, "gogogogo");
       xunji=0;//你不能循迹了
   }
   if(Stickx==0&&Sticky==-1)//往后退
   {
       back_up();
        ESP_LOGI(TAG, "backup");
        xunji=0;//你不能循迹了
   }
   if(Stickx==-1&&Sticky==-1)//后退着左转
   {
       back_up_left();
        xunji=0;
   }
   if(Stickx==1&&Sticky==-1)
   {
      back_up_right();
       xunji=0;
   }
   if(Stickx==-1&&Sticky==1)
   {
       go_forward_left();
        xunji=0;
   }
   if(Stickx==1&&Sticky==1)
   {
      go_forward_right();
       xunji=0;
   }

}


