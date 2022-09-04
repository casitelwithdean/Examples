/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

/****************************************************************************
* This is a demo for bluetooth config wifi connection to ap. You can config ESP32 to connect a softap
* or config ESP32 as a softap to be connected by other device. APP can be downloaded from github
* android source code: https://github.com/EspressifApp/EspBlufi
* iOS source code: https://github.com/EspressifApp/EspBlufiForiOS
****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_bt.h"
#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "driver/i2c.h"
#include "driver/gpio.h"
#include "mqtt_client.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "esp_blufi_api.h"
#include "blufi_example.h"
#include "cJSON.h"
#include "esp_wifi.h"
#include "esp_blufi.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "blufi_example.h"
#include "esp_bt_device.h"
#include "esp_efuse.h"
#include "nvs_flash.h"
#include "esp_sleep.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include <time.h>
#include <sys/time.h>
#include "driver/uart.h"
#include "driver/ledc.h"
#include "esp_timer.h"
#include <flashdb.h>
#include "esp_tls.h"
#include "WAV_C.h"
#include "open8.h"
#include "esp_http_client.h"
#include "sdkconfig.h"

#include "esp_sleep.h"
static void periodic_timer_callback(void *arg);
static void speaker(void *arg);
#define ACK_CHECK_EN 0x1           /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS 0x0          /*!< I2C master will not check ack from slave */
#define ACK_VAL 0x0                /*!< I2C ack value */
#define NACK_VAL 0x1               /*!< I2C nack value */
extern const uint8_t DATA[13560];  //密码错误
extern const uint8_t OPEN8[14320]; //开门成功
#define LEDC_LS_TIMER LEDC_TIMER_1
#define LEDC_LS_MODE LEDC_LOW_SPEED_MODE

#define LEDC_LS_CH0_GPIO (4)
#define LEDC_LS_CH0_CHANNEL LEDC_CHANNEL_0
#define I2C_MASTER_TX_BUF_DISABLE 0 /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0 /*!< I2C master doesn't need buffer */

#define LEDC_LS_TIMER LEDC_TIMER_1


void DelayUs(uint32_t nCount)
{
    ets_delay_us(nCount);
}

void ledcinit()
{
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_TIMER_10_BIT, // resolution of PWM duty
        .freq_hz = 8000,                      // frequency of PWM signal
        .speed_mode = LEDC_LS_MODE,           // timer mode
        .timer_num = LEDC_LS_TIMER,           // timer index
        .clk_cfg = LEDC_AUTO_CLK,             // Auto select the source clock
    };
    // Set configuration of timer0 for high speed channels
    ledc_timer_config(&ledc_timer);
    ledc_channel_config_t ledc_channel = {
        .channel = LEDC_LS_CH0_CHANNEL,
        .duty = 0,
        .gpio_num = LEDC_LS_CH0_GPIO,
        .speed_mode = LEDC_LS_MODE,
        .hpoint = 0,
        .timer_sel = LEDC_LS_TIMER};
    ledc_channel_config(&ledc_channel);
}
void open_success()
{
    for (int i = 0; i < 14320; i++)
    {
        ledc_set_duty(LEDC_LS_MODE, LEDC_LS_CH0_CHANNEL, OPEN8[i] * 4);
        ledc_update_duty(LEDC_LS_MODE, LEDC_LS_CH0_CHANNEL);
        DelayUs(125);
        //
        //  delay_clock(160*125);
    }
    ledc_set_duty(LEDC_LS_MODE, LEDC_LS_CH0_CHANNEL, 0);
    ledc_update_duty(LEDC_LS_MODE, LEDC_LS_CH0_CHANNEL);
}
void open_fail()
{
    for (int i = 0; i < 13560; i++)
    {
        ledc_set_duty(LEDC_LS_MODE, LEDC_LS_CH0_CHANNEL, DATA[i] * 4);
        ledc_update_duty(LEDC_LS_MODE, LEDC_LS_CH0_CHANNEL);
        //  delay_clock(160*125);
        DelayUs(125);
    }
    ledc_set_duty(LEDC_LS_MODE, LEDC_LS_CH0_CHANNEL, 0);
    ledc_update_duty(LEDC_LS_MODE, LEDC_LS_CH0_CHANNEL);
}
#define FDB_LOG_TAG "[main]"
#define MAX_HTTP_RECV_BUFFER 512
#define MAX_HTTP_OUTPUT_BUFFER 2048
static uint32_t boot_count = 0;
static time_t boot_time[10] = {0, 1, 2, 3};
/* default KV nodes */
static struct fdb_default_kv_node default_kv_table[] = {
    {"username", "armink", 0},                       /* string KV */
    {"password", "123456", 0},                       /* string KV */
    {"boot_count", &boot_count, sizeof(boot_count)}, /* int type KV */
    {"boot_time", &boot_time, sizeof(boot_time)},    /* int array type KV */
};
/* KVDB object */
static struct fdb_kvdb kvdb = {0};
/* TSDB object */
struct fdb_tsdb tsdb = {0};
/* counts for simulated timestamp */
static int counts = 0;
static SemaphoreHandle_t s_lock = NULL;

static void lock(fdb_db_t db)
{
    xSemaphoreTake(s_lock, portMAX_DELAY);
}

static void unlock(fdb_db_t db)
{
    xSemaphoreGive(s_lock);
}

static fdb_time_t get_time(void)
{
    /* Using the counts instead of timestamp.
     * Please change this function to return RTC time.
     */
    return ++counts;
}
static void example_event_callback(esp_blufi_cb_event_t event, esp_blufi_cb_param_t *param);

char pub[100] = "111111                 111";
char sub[100] = "111111                 111";

static const char *TAG = "MQTT_EXAMPLE";
#define WIFI_LIST_NUM 10

int MQTT_START = 0;
int mqtt_connenct = 0;
int wifi_connect = 0;
int sub_succeed = 0;
int speaker_cnt = 0;

static wifi_config_t sta_config;
static wifi_config_t ap_config;
char pub_topic_1[] = "wuwo/f/";
char sub_topic[] = "wuwo/d/";
/* FreeRTOS event group to signal when we are connected & ready to make a request */
static EventGroupHandle_t wifi_event_group;

/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
const int CONNECTED_BIT = BIT0;

/* store the station info for send back to phone */
static bool gl_sta_connected = false;
static bool ble_is_connected = false;
static uint8_t gl_sta_bssid[6];
static uint8_t gl_sta_ssid[32];
static int gl_sta_ssid_len;
char mac_addr[12];
int shangbao = 0;
char update[50]; //上报的字符串
int D0 = 1;
int D1 = 1;
int D2 = 1;
int D3 = 1;
int sum = 0;
char password[30];
int dijiwei = 0;
int i = 0;
int renshu = 0;
char pressed;
int LED_START = 0;
int dijigeshanchu = 0;
int delete = 0;
int delete_endtime = 0;
int led_count = 0;
int righttimes = 0;
int right = 0;
int mima_right = 0;
int wrong = 0;
int password_changed = 0;
int delete_all = 0;
int youkongde = 0;
char *timestamp = "                 ";
int non = 0;
char endtime_del[50][20] = {0};
char right_password[50][20] = {
    {"122"}, {"351"}, {"66666"}};

char cunchu[50][20] = {
    {"122"}, {""}, {"66666"}};
char cunchu_endtime[50][20] = {
    {"122"}, {""}, {"66666"}};
char *endtime_temp;
char *gonna_delete;
char *username;
int shangbao_username = 0;
int strtoint(char *x) //string转成int
{
    char *c = &x[0];
    int n = atoi(c);
    return n;
}
#define _I2C_NUMBER(num) I2C_NUM_##num
#define I2C_NUMBER(num) _I2C_NUMBER(num)
#define I2C_MASTER_NUM I2C_NUMBER(0) /*!< I2C port number for master dev */
void delete_xiafa();
#define WRITE_BIT I2C_MASTER_WRITE /*!< I2C master write */
#define READ_BIT I2C_MASTER_READ   /*!< I2C master read */

static esp_err_t i2c_master_init(void)
{
    int i2c_master_port = I2C_MASTER_NUM;
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = 19,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = 18,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000,
        // .clk_flags = 0,          /*!< Optional, you can use I2C_SCLK_SRC_FLAG_* flags to choose i2c source clock here. */
    };
    esp_err_t err = i2c_param_config(i2c_master_port, &conf);
    if (err != ESP_OK)
    {
        return err;
    }
    return i2c_driver_install(i2c_master_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}

static void http_test_task1(void *pvParameters)
{
    vTaskDelay(1000);
    //02-1 定义需要的变量
    char output_buffer[MAX_HTTP_OUTPUT_BUFFER] = {0}; //用于接收通过http协议返回的数据
    int content_length = 0;                           //http协议头的长度
    //02-2 配置http结构体
    //定义http配置结构体，并且进行清零
    esp_http_client_config_t config;
    memset(&config, 0, sizeof(config));
    //向配置结构体内部写入url
    static const char *URL = "http://api.m.taobao.com/rest/api3.do?api=mtop.common.getTimestamp"; //"http://api.k780.com/?app=life.time&appkey=10003&sign=b59bc3ef6191eb9f747dd4e83c99f2a4&format=json"; //"http://api.uukit.com/time";//"http://quan.suning.com/getSysTime.do";
    config.url = URL;
    //初始化结构体
    esp_http_client_handle_t client = esp_http_client_init(&config); //初始化http连接
    //设置发送请求
    esp_http_client_set_method(client, HTTP_METHOD_GET);
    //02-3 循环通讯
    while (1)
    {
        //与目标主机创建连接，并且声明写入内容长度为0
        esp_err_t err = esp_http_client_open(client, 0);

        // //如果连接失败
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
        }
        //如果连接成功
        else
        {

            //读取目标主机的返回内容的协议头
            content_length = esp_http_client_fetch_headers(client);

            //如果协议头长度小于0，说明没有成功读取到
            if (content_length < 0)
            {
                ESP_LOGE(TAG, "HTTP client fetch headers failed");
            }

            //如果成功读取到了协议头
            else
            {

                //读取目标主机通过http的响应内容
                int data_read = esp_http_client_read_response(client, output_buffer, MAX_HTTP_OUTPUT_BUFFER);
                if (data_read >= 0)
                {

                    //打印响应内容，包括响应状态，响应体长度及其内容
                    // ESP_LOGI(TAG, "HTTP GET Status = %d, content_length = %d",
                    // esp_http_client_get_status_code(client),				//获取响应状态信息
                    // esp_http_client_get_content_length(client));			//获取响应信息长度
                    // printf("data:%s\n", output_buffer);
                    //对接收到的数据作相应的处理
                    cJSON *root = NULL;
                    root = cJSON_Parse(output_buffer);

                    if (root != NULL)
                    {
                        cJSON *root1 = cJSON_GetObjectItem(root, "data");
                        if (root1 != NULL)
                        {
                            timestamp = cJSON_GetObjectItem(root1, "t")->valuestring;
                            //  ESP_LOGI(TAG, "时间戳%s",timestamp);
                            //  ESP_LOGI(TAG, "人数：%d",renshu);

                            for (int r = 0; r < renshu; r++)
                            {
                                // ESP_LOGI(TAG, "timestamp%d",strtoint(timestamp));
                                // ESP_LOGI(TAG, "endtime_del%d",atoi(endtime_del[r]));

                                if (strcmp(timestamp, endtime_del[r]) > 0) //大于时间戳则删除密码
                                                                           //  if(strtoint(timestamp)>atoi(endtime_del[r]))//大于时间戳则删除密码
                                {

                                    dijigeshanchu = r;
                                    delete = 1;
                                    ESP_LOGI(TAG, "要删除的endtime[r]是%d", atoi(endtime_del[r]));
                                    ESP_LOGI(TAG, "开始删除");
                                    vTaskDelay(1000);
                                }
                            }
                        }
                    }

                    //     for(int r=0;r<strlen(time->valuestring);r++)
                    // {timestamp[r]=(time->valuestring)[r];}
                    // ESP_LOGI(TAG, "%s",timestamp);
                    cJSON_Delete(root);
                }
                //如果不成功
                else
                {
                    ESP_LOGE(TAG, "Failed to read response");
                }
            }
        }

        //关闭连接
        esp_http_client_close(client);

        //延时
        vTaskDelay(3000 / portTICK_PERIOD_MS);
    }
}
static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0)
    {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    // your_context_t *context = event->context;
    switch (event->event_id)
    {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED连接成功了");
        MQTT_START = 0;
        mqtt_connenct = 1;
        esp_mqtt_client_publish(client, pub, "连接上MQTT", 0, 1, 0);

        msg_id = esp_mqtt_client_subscribe(client, sub, 0);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        MQTT_START = 1;
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "订阅成功");
        sub_succeed = 1;
        // ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:

        ESP_LOGI(TAG, "取消订阅");
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);

        break;
    case MQTT_EVENT_DATA:

        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        //  ESP_LOGI(TAG, "不科学");
        //   printf("DATA=%s\r\n",  event->data);
        char temp[] = "                                                                              ";
        sprintf(temp, "%.*s", event->data_len, event->data);

        cJSON *root = NULL;
        root = cJSON_Parse(temp);

        int result_array_size = cJSON_GetArraySize(root); /*求results键值对数组中有多少个元素*/
                                                          // printf("Presult array size is %d\n",result_array_size);
        if (result_array_size == 2)                       //2就是删除密码
        {
            cJSON *time = cJSON_GetObjectItem(root, "msg");
            if (strcmp(time->valuestring, "all") == 0)
            {
                delete_all = 1;
            }
            else
            {

                gonna_delete = cJSON_GetObjectItem(root, "msg")->valuestring;
                delete_xiafa();
            }
            ESP_LOGI(TAG, "删除密码了");
        }
        if (result_array_size == 3) //3就是下发密码
        {
            char *kaisuo = cJSON_GetObjectItem(root, "head")->valuestring;
            if (kaisuo != NULL)
            {
                if (strcmp(kaisuo, "qr") == 0)
                {

                    if (root != NULL)
                        username = cJSON_GetObjectItem(root, "name")->valuestring;
                    // sprintf(username,"%s",time->valuestring);
                    shangbao_username = 1;

                    ESP_LOGI(TAG, "开锁了");
                }

                else
                {
                    cJSON *time = cJSON_GetObjectItem(root, "endtime");

                    endtime_temp = cJSON_GetObjectItem(root, "endtime")->valuestring;
                    ESP_LOGI(TAG, "endtime_temp%s", endtime_temp);
                    cJSON *mima = cJSON_GetObjectItem(root, "msg");

                    if (mima != NULL)
                    {
                        for (int d = 0; d < strlen(cJSON_GetObjectItem(root, "msg")->valuestring); d++)
                        {

                            right_password[renshu][d] = (cJSON_GetObjectItem(root, "msg")->valuestring)[d];
                        }

                        ESP_LOGI(TAG, "%s", right_password[renshu]);

                        esp_mqtt_client_publish(client, pub, cJSON_GetObjectItem(root, "msg")->valuestring, 0, 1, 0);
                    }
                    ESP_LOGI(TAG, "下发密码了");

                    //   right_password[renshu]=cJSON_GetObjectItem(root,"msg")->valuestring;
                    password_changed = 1;
                }
            }
        }

        //ESP_LOGI(TAG,"%s\n",time1->valuestring);

        //sprintf(shangbao,"{\n\"head\":\"state\",\n\"state\":\"d\",\n\"mode\":\"00\",\n\"num\":\"%d\"\n}");

        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT)
        {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno", event->error_handle->esp_transport_sock_errno);
            ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
        }
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
    return ESP_OK;
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    mqtt_event_handler_cb(event_data);
}

static void ip_event_handler(void *arg, esp_event_base_t event_base,
                             int32_t event_id, void *event_data)
{
    wifi_mode_t mode;

    switch (event_id)
    {
    case IP_EVENT_STA_GOT_IP:
    {
        esp_blufi_extra_info_t info;

        xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
        esp_wifi_get_mode(&mode);

        memset(&info, 0, sizeof(esp_blufi_extra_info_t));
        memcpy(info.sta_bssid, gl_sta_bssid, 6);
        info.sta_bssid_set = true;
        info.sta_ssid = gl_sta_ssid;
        info.sta_ssid_len = gl_sta_ssid_len;
        if (ble_is_connected == true)
        {
            esp_blufi_send_wifi_conn_report(mode, ESP_BLUFI_STA_CONN_SUCCESS, 0, &info);
        }
        else
        {
            BLUFI_INFO("BLUFI BLE is not connected yet\n");
        }
        //ESP_LOGI(TAG, "我觉的就是这个是地方了");
        //     wifi_connect=1;

        break;
    }
    default:
        break;
    }
    return;
}

static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    wifi_event_sta_connected_t *event;
    wifi_mode_t mode;

    switch (event_id)
    {
    case WIFI_EVENT_STA_START:
        esp_wifi_connect();
        break;
    case WIFI_EVENT_STA_CONNECTED:
        gl_sta_connected = true;
        event = (wifi_event_sta_connected_t *)event_data;
        memcpy(gl_sta_bssid, event->bssid, 6);
        memcpy(gl_sta_ssid, event->ssid, event->ssid_len);
        gl_sta_ssid_len = event->ssid_len;
        // ESP_LOGI(TAG, "这个地方也不是不肯");
        wifi_connect = 1;
        break;
    case WIFI_EVENT_STA_DISCONNECTED:
        /* This is a workaround as ESP32 WiFi libs don't currently
           auto-reassociate. */
        gl_sta_connected = false;
        memset(gl_sta_ssid, 0, 32);
        memset(gl_sta_bssid, 0, 6);
        gl_sta_ssid_len = 0;
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
        wifi_connect = 0;

        break;
    case WIFI_EVENT_AP_START:
        esp_wifi_get_mode(&mode);

        /* TODO: get config or information of softap, then set to report extra_info */
        if (ble_is_connected == true)
        {
            if (gl_sta_connected)
            {
                esp_blufi_send_wifi_conn_report(mode, ESP_BLUFI_STA_CONN_SUCCESS, 0, NULL);
            }
            else
            {
                esp_blufi_send_wifi_conn_report(mode, ESP_BLUFI_STA_CONN_FAIL, 0, NULL);
            }
        }
        else
        {
            BLUFI_INFO("BLUFI BLE is not connected yet\n");
        }
        break;
    case WIFI_EVENT_SCAN_DONE:
    {
        uint16_t apCount = 0;
        esp_wifi_scan_get_ap_num(&apCount);
        if (apCount == 0)
        {
            BLUFI_INFO("Nothing AP found");
            break;
        }
        wifi_ap_record_t *ap_list = (wifi_ap_record_t *)malloc(sizeof(wifi_ap_record_t) * apCount);
        if (!ap_list)
        {
            BLUFI_ERROR("malloc error, ap_list is NULL");
            break;
        }
        ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&apCount, ap_list));
        esp_blufi_ap_record_t *blufi_ap_list = (esp_blufi_ap_record_t *)malloc(apCount * sizeof(esp_blufi_ap_record_t));
        if (!blufi_ap_list)
        {
            if (ap_list)
            {
                free(ap_list);
            }
            BLUFI_ERROR("malloc error, blufi_ap_list is NULL");
            break;
        }
        for (int i = 0; i < apCount; ++i)
        {
            blufi_ap_list[i].rssi = ap_list[i].rssi;
            memcpy(blufi_ap_list[i].ssid, ap_list[i].ssid, sizeof(ap_list[i].ssid));
        }

        if (ble_is_connected == true)
        {
            esp_blufi_send_wifi_list(apCount, blufi_ap_list);
        }
        else
        {
            BLUFI_INFO("BLUFI BLE is not connected yet\n");
        }

        esp_wifi_scan_stop();
        free(ap_list);
        free(blufi_ap_list);
        break;
    }
    default:
        break;
    }
    return;
}

static void initialise_wifi(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &ip_event_handler, NULL));

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
}

static esp_blufi_callbacks_t example_callbacks = {
    .event_cb = example_event_callback,
    .negotiate_data_handler = blufi_dh_negotiate_data_handler,
    .encrypt_func = blufi_aes_encrypt,
    .decrypt_func = blufi_aes_decrypt,
    .checksum_func = blufi_crc_checksum,
};

static void example_event_callback(esp_blufi_cb_event_t event, esp_blufi_cb_param_t *param)
{
    /* actually, should post to blufi_task handle the procedure,
     * now, as a example, we do it more simply */
    switch (event)
    {
    case ESP_BLUFI_EVENT_INIT_FINISH:
        BLUFI_INFO("BLUFI init finish\n");

        esp_blufi_adv_start();
        break;
    case ESP_BLUFI_EVENT_DEINIT_FINISH:
        BLUFI_INFO("BLUFI deinit finish\n");
        break;
    case ESP_BLUFI_EVENT_BLE_CONNECT:
        BLUFI_INFO("BLUFI ble connect\n");
        ble_is_connected = true;
        esp_blufi_adv_stop();
        blufi_security_init();
        break;
    case ESP_BLUFI_EVENT_BLE_DISCONNECT:
        BLUFI_INFO("BLUFI ble disconnect\n");
        ble_is_connected = false;
        blufi_security_deinit();
        esp_blufi_adv_start();
        break;
    case ESP_BLUFI_EVENT_SET_WIFI_OPMODE:
        BLUFI_INFO("BLUFI Set WIFI opmode %d\n", param->wifi_mode.op_mode);
        ESP_ERROR_CHECK(esp_wifi_set_mode(param->wifi_mode.op_mode));
        break;
    case ESP_BLUFI_EVENT_REQ_CONNECT_TO_AP:
        BLUFI_INFO("BLUFI requset wifi connect to AP\n");
        /* there is no wifi callback when the device has already connected to this wifi
        so disconnect wifi before connection.
        */

        esp_wifi_disconnect();
        esp_wifi_connect();
        MQTT_START = 1;
        break;
    case ESP_BLUFI_EVENT_REQ_DISCONNECT_FROM_AP:
        //   BLUFI_INFO("BLUFI requset wifi disconnect from AP\n");
        esp_wifi_disconnect();
        break;
    case ESP_BLUFI_EVENT_REPORT_ERROR:
        // BLUFI_ERROR("BLUFI report error, error code %d\n", param->report_error.state);
        esp_blufi_send_error_info(param->report_error.state);
        break;
    case ESP_BLUFI_EVENT_GET_WIFI_STATUS:
    {
        wifi_mode_t mode;
        esp_blufi_extra_info_t info;

        esp_wifi_get_mode(&mode);

        if (gl_sta_connected)
        {
            memset(&info, 0, sizeof(esp_blufi_extra_info_t));
            memcpy(info.sta_bssid, gl_sta_bssid, 6);
            info.sta_bssid_set = true;
            info.sta_ssid = gl_sta_ssid;
            info.sta_ssid_len = gl_sta_ssid_len;
            esp_blufi_send_wifi_conn_report(mode, ESP_BLUFI_STA_CONN_SUCCESS, 0, &info);
        }
        else
        {
            esp_blufi_send_wifi_conn_report(mode, ESP_BLUFI_STA_CONN_FAIL, 0, NULL);
        }
        BLUFI_INFO("BLUFI get wifi status from AP\n");
        //MQTT_START=1;
        break;
    }
    case ESP_BLUFI_EVENT_RECV_SLAVE_DISCONNECT_BLE:
        BLUFI_INFO("blufi close a gatt connection");
        esp_blufi_disconnect();
        break;
    case ESP_BLUFI_EVENT_DEAUTHENTICATE_STA:
        /* TODO */
        break;
    case ESP_BLUFI_EVENT_RECV_STA_BSSID:
        memcpy(sta_config.sta.bssid, param->sta_bssid.bssid, 6);
        sta_config.sta.bssid_set = 1;
        esp_wifi_set_config(WIFI_IF_STA, &sta_config);
        BLUFI_INFO("Recv STA BSSID %s\n", sta_config.sta.ssid);
        break;
    case ESP_BLUFI_EVENT_RECV_STA_SSID:
        strncpy((char *)sta_config.sta.ssid, (char *)param->sta_ssid.ssid, param->sta_ssid.ssid_len);
        sta_config.sta.ssid[param->sta_ssid.ssid_len] = '\0';
        esp_wifi_set_config(WIFI_IF_STA, &sta_config);
        BLUFI_INFO("Recv STA SSID %s\n", sta_config.sta.ssid);
        break;
    case ESP_BLUFI_EVENT_RECV_STA_PASSWD:
        strncpy((char *)sta_config.sta.password, (char *)param->sta_passwd.passwd, param->sta_passwd.passwd_len);
        sta_config.sta.password[param->sta_passwd.passwd_len] = '\0';
        esp_wifi_set_config(WIFI_IF_STA, &sta_config);
        BLUFI_INFO("Recv STA PASSWORD %s\n", sta_config.sta.password);
        break;
    case ESP_BLUFI_EVENT_RECV_SOFTAP_SSID:
        strncpy((char *)ap_config.ap.ssid, (char *)param->softap_ssid.ssid, param->softap_ssid.ssid_len);
        ap_config.ap.ssid[param->softap_ssid.ssid_len] = '\0';
        ap_config.ap.ssid_len = param->softap_ssid.ssid_len;
        esp_wifi_set_config(WIFI_IF_AP, &ap_config);
        BLUFI_INFO("Recv SOFTAP SSID %s, ssid len %d\n", ap_config.ap.ssid, ap_config.ap.ssid_len);
        break;
    case ESP_BLUFI_EVENT_RECV_SOFTAP_PASSWD:
        strncpy((char *)ap_config.ap.password, (char *)param->softap_passwd.passwd, param->softap_passwd.passwd_len);
        ap_config.ap.password[param->softap_passwd.passwd_len] = '\0';
        esp_wifi_set_config(WIFI_IF_AP, &ap_config);
        BLUFI_INFO("Recv SOFTAP PASSWORD %s len = %d\n", ap_config.ap.password, param->softap_passwd.passwd_len);
        break;
    case ESP_BLUFI_EVENT_RECV_SOFTAP_MAX_CONN_NUM:
        if (param->softap_max_conn_num.max_conn_num > 4)
        {
            return;
        }
        ap_config.ap.max_connection = param->softap_max_conn_num.max_conn_num;
        esp_wifi_set_config(WIFI_IF_AP, &ap_config);
        BLUFI_INFO("Recv SOFTAP MAX CONN NUM %d\n", ap_config.ap.max_connection);
        break;
    case ESP_BLUFI_EVENT_RECV_SOFTAP_AUTH_MODE:
        if (param->softap_auth_mode.auth_mode >= WIFI_AUTH_MAX)
        {
            return;
        }
        ap_config.ap.authmode = param->softap_auth_mode.auth_mode;
        esp_wifi_set_config(WIFI_IF_AP, &ap_config);
        BLUFI_INFO("Recv SOFTAP AUTH MODE %d\n", ap_config.ap.authmode);
        break;
    case ESP_BLUFI_EVENT_RECV_SOFTAP_CHANNEL:
        if (param->softap_channel.channel > 13)
        {
            return;
        }
        ap_config.ap.channel = param->softap_channel.channel;
        esp_wifi_set_config(WIFI_IF_AP, &ap_config);
        BLUFI_INFO("Recv SOFTAP CHANNEL %d\n", ap_config.ap.channel);
        break;
    case ESP_BLUFI_EVENT_GET_WIFI_LIST:
    {
        wifi_scan_config_t scanConf = {
            .ssid = NULL,
            .bssid = NULL,
            .channel = 0,
            .show_hidden = false};
        esp_wifi_scan_start(&scanConf, true);
        break;
    }
    case ESP_BLUFI_EVENT_RECV_CUSTOM_DATA:
        BLUFI_INFO("Recv Custom Data %d\n", param->custom_data.data_len);
        esp_log_buffer_hex("Custom Data", param->custom_data.data, param->custom_data.data_len);
        break;
    case ESP_BLUFI_EVENT_RECV_USERNAME:
        /* Not handle currently */
        break;
    case ESP_BLUFI_EVENT_RECV_CA_CERT:
        /* Not handle currently */
        break;
    case ESP_BLUFI_EVENT_RECV_CLIENT_CERT:
        /* Not handle currently */
        break;
    case ESP_BLUFI_EVENT_RECV_SERVER_CERT:
        /* Not handle currently */
        break;
    case ESP_BLUFI_EVENT_RECV_CLIENT_PRIV_KEY:
        /* Not handle currently */
        break;
        ;
    case ESP_BLUFI_EVENT_RECV_SERVER_PRIV_KEY:
        /* Not handle currently */
        break;
    default:
        break;
    }
}

void compare()
{

    for (int p = 0; p < 50; p++)
    {
        if (dijiwei != 0)
        {

            if (dijiwei == strlen(right_password[p])) //i代表输入密码的长度
            {
                // ESP_LOGI(TAG,"看下每个密码的strlen吧%d",strlen(right_password[p]));

                for (int c = 0; c < dijiwei; c++)
                {
                    // ESP_LOGI(TAG,"正确密码的每一个字符%c",right_password[p][c]);
                    // ESP_LOGI(TAG,"输入密码的每一个字符%c",password[c]);
                    if (password[c] == right_password[p][c])
                        righttimes++;
                }
                if (righttimes == dijiwei)
                {
                    //    ESP_LOGI(TAG,"right");
                    right = 1;
                    vTaskDelay(10);
                    righttimes = 0;
                }
                else
                {
                    //    ESP_LOGI(TAG,"wrong");
                    // right=0;
                    vTaskDelay(10);
                }
                righttimes = 0;
            }
            else
            {
                //right=0;
                //    ESP_LOGI(TAG,"wrong");
            }
        }
    }
    shangbao_username = 0;
    // ESP_LOGI(TAG,"我估是捏啊");
    shangbao = 1;
}
void Touch_detect()
{

    while (1)
    {
        D0 = gpio_get_level(15);
        D1 = gpio_get_level(16);
        D2 = gpio_get_level(17);
        D3 = gpio_get_level(19);
        // ESP_LOGI(TAG,"%d %d  %d  %d",D0,D1,D2,D3);

        sum = D0 + D1 + D2 + D3;
        if (sum != 15)
        {
            i++;
            // gpio_set_level(5,1);
            LED_START = 1;
            led_count = 0;
        }
        switch (sum)
        {
        case 0:
            password[i] = '1';
            ESP_LOGI(TAG, "%c", password[i]);
            break;
        case 1:
            password[i] = '2';
            ESP_LOGI(TAG, "%c", password[i]);
            break;
        case 2:
            password[i] = '3';
            ESP_LOGI(TAG, "%c", password[i]);
            break;
        case 3:
            password[i] = '4';
            ESP_LOGI(TAG, "%c", password[i]);
            break;
        case 4:
            password[i] = '8';
            ESP_LOGI(TAG, "%c", password[i]);
            break;
        case 5:
            password[i] = '9';
            ESP_LOGI(TAG, "%c", password[i]);
            break;
        case 6:
            password[i] = '6';
            ESP_LOGI(TAG, "%c", password[i]);
            break;
        case 7:
            password[i] = '5';
            ESP_LOGI(TAG, "%c", password[i]);
            break;
        case 8:
            password[i] = '7';
            ESP_LOGI(TAG, "%c", password[i]);
            break;
        case 9: //K10
            compare();

            break;
        case 10:

            password[i] = '0';
            ESP_LOGI(TAG, "%c", password[i]);
            break;
        case 11: //K12
            LED_START = 0;
            // gpio_set_level(5,0);
            i = 0;
            break;

        default:
            break;
        }
        vTaskDelay(1000);
    }
}
void led_task() //十秒之后自动关掉
{

    while (1)
    {
        if (LED_START == 1)
        {
            vTaskDelay(970);
            led_count++;
            // ESP_LOGI(TAG,"倒计时%d",led_count);
            if (led_count == 10)
            {
                LED_START = 0;
                gpio_set_level(10, 1); //灯不亮
            }
        }

        vTaskDelay(30);
    }
}

void delete_xiafa()
{
    for (int r = 0; r < renshu; r++)
    {
        if (strcmp(gonna_delete, right_password[r]) == 0) //大于时间戳则删除密码
        {
            dijigeshanchu = r;
            delete = 1;
        }
    }
}
void flashdb()
{

    fdb_err_t result;

    if (s_lock == NULL)
    {
        s_lock = xSemaphoreCreateCounting(1, 1);
        assert(s_lock != NULL);
    }

    struct fdb_default_kv default_kv;

    default_kv.kvs = default_kv_table;
    default_kv.num = sizeof(default_kv_table) / sizeof(default_kv_table[0]);
    /* set the lock and unlock function if you want */
    fdb_kvdb_control(&kvdb, FDB_KVDB_CTRL_SET_LOCK, lock);
    fdb_kvdb_control(&kvdb, FDB_KVDB_CTRL_SET_UNLOCK, unlock);

    result = fdb_kvdb_init(&kvdb, "env", "fdb_kvdb1", &default_kv, NULL);

    if (result != FDB_NO_ERR)
    {
        return -1;
    }
    struct fdb_blob blob;
    fdb_kv_get_blob(&kvdb, "renshu", fdb_blob_make(&blob, &renshu, sizeof(renshu)));

    char temp_data[10] = "36C";
    ESP_LOGI(TAG, "从flash读取人数:%d", renshu);
    // fdb_kv_set(&kvdb, cunchu[0],"right_password[renshu]");
    char *return_value = {0};
    //right_password[0] = fdb_kv_get(&kvdb, "temp");
    if (renshu > 50)
        renshu = 39;
    for (int i = 0; i < 50; i++)
    {

        char *data = fdb_kv_get(&kvdb, cunchu[i]);
        //  for(int d=0;d<sizeof(return_value);d++)
        //  {
        //   right_password[i][d]=  return_value[d];
        //  }
        if (data != NULL)
        {
            sprintf(right_password[i], "%s", data);
            ESP_LOGI(TAG, "从flash读取密码%s", right_password[i]);
        }
        char *flash_endtime = fdb_kv_get(&kvdb, cunchu_endtime[i]);
        if (flash_endtime != NULL)
        {
            sprintf(endtime_del[i], "%s", flash_endtime);
            ESP_LOGI(TAG, "从flash读取密码的删除时间%s", endtime_del[i]);
        }
    }
    while (1)
    {
        if (password_changed == 1) //下发密码
        {

            password_changed = 0;
            for (int r = 0; r < renshu; r++)
            {
                if (strcmp(right_password[r], "") == 0)
                {
                    fdb_kv_set(&kvdb, cunchu[r], right_password[renshu]);
                    youkongde = 1;
                    sprintf(endtime_del[r], "%s", endtime_temp);
                    fdb_kv_set(&kvdb, cunchu_endtime[r], endtime_del[renshu]);
                    ESP_LOGI(TAG, "可能");
                }
            }
            if (youkongde == 0)
            {
                sprintf(endtime_del[renshu], "%s", endtime_temp);
                fdb_kv_set(&kvdb, cunchu_endtime[renshu], endtime_del[renshu]);
                fdb_kv_set(&kvdb, cunchu[renshu], right_password[renshu]);

                //  ESP_LOGI(TAG, "就这%d",atoi(endtime_del[renshu]));
            }
            else
            {
                youkongde = 0;
                //    ESP_LOGI(TAG, "有趣");
            }

            renshu++;
            ESP_LOGI(TAG, "存储成功");
            fdb_kv_set_blob(&kvdb, "renshu", fdb_blob_make(&blob, &renshu, sizeof(renshu)));
        }
        if (delete == 1) //下发删除或者到时间自动删除
        {
            renshu = renshu - 1;
            memset(right_password[dijigeshanchu], 0, sizeof(right_password[dijigeshanchu]));
            memset(endtime_del[dijigeshanchu], 0, sizeof(endtime_del[dijigeshanchu]));
            fdb_kv_set(&kvdb, cunchu[dijigeshanchu], right_password[dijigeshanchu]);
            fdb_kv_set(&kvdb, cunchu_endtime[dijigeshanchu], endtime_del[dijigeshanchu]);
            delete = 0;
            ESP_LOGI(TAG, "删除成功");
            fdb_kv_set_blob(&kvdb, "renshu", fdb_blob_make(&blob, &renshu, sizeof(renshu)));
        }

        if (delete_all == 1)
        {
            for (int d = 0; d < 50; d++)
            {
                memset(right_password[d], 0, sizeof(right_password[d]));
                memset(endtime_del[d], 0, sizeof(endtime_del[d]));
                fdb_kv_set(&kvdb, cunchu_endtime[d], endtime_del[d]);
                fdb_kv_set(&kvdb, cunchu[d], right_password[d]);
            }
            delete_all = 0;
            renshu = 0;
            ESP_LOGI(TAG, "全部删除成功");
            fdb_kv_set_blob(&kvdb, "renshu", fdb_blob_make(&blob, &renshu, sizeof(renshu)));
        }

        vTaskDelay(100);
    }
}
void endtime()
{

    while (1)
    {
        vTaskDelay(1000);
    }
}
void i2c()
{
    uint8_t data = 21;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);

    i2c_master_write_byte(cmd, 0xA0 | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, 0XB2, ACK_CHECK_EN); //
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, 0xA0 | READ_BIT, ACK_CHECK_EN);
    i2c_master_read(cmd, data, 100, ACK_CHECK_DIS); //
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    ESP_LOGI(TAG, "第一个寄存器%d", data);
    if (((data >> 0) & 0x01))
        pressed = '1';
    if (((data >> 1) & 0x01))
        pressed = '2';
    if (((data >> 2) & 0x01))
        pressed = '3';
    if (((data >> 3) & 0x01))
        pressed = '4';
    if (((data >> 4) & 0x01))
        pressed = '5';
    if (((data >> 5) & 0x01))
        pressed = '6';
    if (((data >> 6) & 0x01))
        pressed = '7';
    if (((data >> 7) & 0x01))
        pressed = '8';
    // if(data==0){non=0;}
    // else{non=1;}
    //         cmd = i2c_cmd_link_create();
    //     i2c_master_start(cmd);
    //     i2c_master_write_byte(cmd, 0xA0 | I2C_MASTER_WRITE, ACK_CHECK_EN);
    //     i2c_master_write_byte(cmd, 0XB5, ACK_CHECK_EN);//
    //     i2c_master_start(cmd);
    //     i2c_master_write_byte(cmd, 0xA0 | I2C_MASTER_READ, ACK_CHECK_EN);
    //     i2c_master_read(cmd, &data, 100, I2C_MASTER_LAST_NACK);//
    //     i2c_master_stop(cmd);
    //     i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
    //     i2c_cmd_link_delete(cmd);
    //     for(int i=0;i<4;i++)
    //     {
    //         if(((data>>i)&0x01))
    //         {

    // if(i==0)pressed='9';
    // if(i==1)pressed='0';
    // if(i==2)
    // {
    //    // pressed='*';
    //    LED_START=0;
    //    led_count=0;
    //     dijiwei=0;}//取消
    // if(i==3)
    // {
    // //pressed='#';
    // //compare();
    // }
    // non=1;
    //         }

    //     }
    //     if(non==0)
    //     {
    //         pressed='!';
    //     //   gpio_set_level(6,0);   //蜂鸣器不响

    //     }
    //     else{
    //     // gpio_set_level(6,1);   //蜂鸣器响
    //      gpio_set_level(10,0);   //灯亮
    //     }
}

void beep()
{

    while (1)
    {
        if (non == 0)
        {

            //  gpio_set_level(6,0);   //蜂鸣器不响
        }
        else
        {
            //  gpio_set_level(6,1);   //蜂鸣器响
            gpio_set_level(10, 0); //灯亮
            LED_START = 1;
            led_count = 0;
        }
        vTaskDelay(100);
        /* code */
    }
}
uint8_t SHUJU = 21;
uint8_t SHUJU1 = 16;
void i2c_detect()
{

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);

    i2c_master_write_byte(cmd, 0xA0 | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, 0X08, ACK_CHECK_EN); //
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, 0xA0 | READ_BIT, ACK_CHECK_EN);
    i2c_master_read(cmd, &SHUJU, 100, ACK_CHECK_DIS); //
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    // ESP_LOGI(TAG,"第一个寄存器%d",SHUJU);
    if (((SHUJU >> 0) & 0x01))
        pressed = '3';
    if (((SHUJU >> 1) & 0x01))
        pressed = '6';
    if (((SHUJU >> 2) & 0x01))
        pressed = '9';
    if (((SHUJU >> 3) & 0x01))
        pressed = '5';
    if (((SHUJU >> 4) & 0x01))
        pressed = '4';
    if (((SHUJU >> 5) & 0x01))
        pressed = '8';
    if (((SHUJU >> 6) & 0x01))
        pressed = '7';
    if (((SHUJU >> 7) & 0x01))
    {
        pressed = '#';
        led_count = 0;
        // compare();
        dijiwei = 0;
    } //确认

    if (SHUJU == 0)
    {
        non = 0;
    }
    else
    {
        non = 1;
        led_count = 0;
    }
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, 0xA0 | I2C_MASTER_WRITE, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, 0X09, ACK_CHECK_EN); //
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, 0xA0 | I2C_MASTER_READ, ACK_CHECK_EN);
    i2c_master_read(cmd, &SHUJU, 100, ACK_CHECK_DIS); //
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    for (int i = 0; i < 4; i++)
    {
        if (((SHUJU >> i) & 0x01))
        {
            led_count = 0;
            if (i == 0)
                pressed = '0';
            if (i == 2)
                pressed = '1';
            if (i == 1)
            {
                pressed = '*';
                LED_START = 0;
                gpio_set_level(10, 1); //灯灭
                dijiwei = 0;
                ESP_LOGI(TAG, "第一个寄存器");
            } //取消
            if (i == 3)
            {
                pressed = '2';
                //compare();
            }
            non = 1;
        }
    }
    if (non == 0)
    {
        pressed = '!';
        gpio_set_level(6, 0); //蜂鸣器不响
    }
    else
    {
        gpio_set_level(6, 1);  //蜂鸣器响
        gpio_set_level(10, 0); //灯亮
        if (pressed == '*' || pressed == '#')
            gpio_set_level(6, 0);
        LED_START = 1;
    }
    //   ESP_LOGI(TAG,"字符%c",pressed);
}
char pressed_old;

int dog = 0;

void app_main(void)
{

    esp_err_t ret;
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    initialise_wifi();

    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret)
    {
        BLUFI_ERROR("%s initialize bt controller failed: %s\n", __func__, esp_err_to_name(ret));
    }

    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret)
    {
        BLUFI_ERROR("%s enable bt controller failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    ret = esp_blufi_host_init();
    if (ret)
    {
        BLUFI_ERROR("%s initialise host failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    BLUFI_INFO("BLUFI VERSION %04x\n", esp_blufi_get_version());

    ret = esp_blufi_register_callbacks(&example_callbacks);
    if (ret)
    {
        BLUFI_ERROR("%s blufi register failed, error code = %x\n", __func__, ret);
        return;
    }

    ret = esp_blufi_gap_register_callback();
    if (ret)
    {
        BLUFI_ERROR("%s gap register failed, error code = %x\n", __func__, ret);
        return;
    }

    char dick[22] = "1234567890";
    int dcik = atoi(dick);
    char *baga = "666";
    char *aaa = "1654964215727";
    char *aaabbb = "1654964213727";
    int aaw = strcmp(aaa, aaabbb);
    sprintf(dick, "%s", baga);
    ESP_LOGI(TAG, "打印一下%d", aaw);

    for (int i = 0; i < 50; i++)
    {
        sprintf(cunchu[i], "cunchu%d", i);
        sprintf(cunchu_endtime[i], "endtime%d", i);
    }

    //  ESP_LOGI(TAG,"%s",right_password[2]);
    //  memset(right_password[2],0,sizeof(right_password[2]));
    //  ESP_LOGI(TAG,"%s",right_password[2]);

    ledcinit();

    char shuzu[100];
    ESP_LOGI(TAG, "%d", strlen(shuzu));
    ESP_LOGI(TAG, "%d", sizeof(shuzu));
    ESP_ERROR_CHECK(i2c_master_init());

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);

    i2c_master_write_byte(cmd, 0xA0 | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, 0XC0, ACK_CHECK_EN); //
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, 0xA0 | READ_BIT, ACK_CHECK_EN);
    i2c_master_read(cmd, &SHUJU, 8, ACK_CHECK_DIS); //
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    ESP_LOGI(TAG, "第一个寄存器%d", SHUJU);

    // uint8_t bs[17]={0X00,0x00,0x83,0xF3,0x98,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90};
    uint8_t bs[17] = {0X01, 0x00, 0x83, 0xF3, 0xD8, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f};
    // uint8_t bs[17]={0X01,0x00,0x83,0xF3,0xD8,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10};

    uint8_t sum = 0;
    cmd = i2c_cmd_link_create();

    i2c_master_start(cmd);

    i2c_master_write_byte(cmd, 0xA0 | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, 0XB0, ACK_CHECK_EN); //

    for (int i = 0; i < 17; i++)
    {
        sum += bs[i];
        i2c_master_write_byte(cmd, bs[i], ACK_CHECK_EN); //
                                                         /* code */
    }

    // i2c_master_write(cmd,bs,15,ACK_CHECK_EN);
    i2c_master_write_byte(cmd, sum, ACK_CHECK_DIS); //
    // i2c_master_write_byte(cmd, 0X00, ACK_CHECK_DIS);//
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    ESP_LOGI(TAG, "第一个寄存器%d", SHUJU);

    vTaskDelay(800);

    gpio_config_t gp;
    gp.intr_type = GPIO_INTR_DISABLE;
    gp.mode = GPIO_MODE_OUTPUT;
    gp.pin_bit_mask = GPIO_SEL_6 | GPIO_SEL_5 | GPIO_SEL_10; //蜂鸣器
    gpio_config(&gp);

    gpio_set_level(6, 0);  //蜂鸣器不响
    gpio_set_level(10, 1); //灯不亮
    gpio_set_level(5, 1);  //继电器关

    //    i2c_cmd_handle_t  cmd = i2c_cmd_link_create();
    //         	i2c_master_start(cmd);
    //         	i2c_master_write_byte(cmd, 0x20 << 1 | WRITE_BIT, ACK_CHECK_EN);
    //         	i2c_master_write_byte(cmd, 0x00, ACK_CHECK_EN);//Access IODIRA
    //         	i2c_master_write_byte(cmd, 0x00, ACK_CHECK_EN);//Set all as outputs on A
    //         	i2c_master_stop(cmd);
    //         i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
    //         	i2c_cmd_link_delete(cmd);

    //             cmd = i2c_cmd_link_create();
    //         	i2c_master_start(cmd);
    //         	i2c_master_write_byte(cmd, 0x20 << 1 | WRITE_BIT, ACK_CHECK_EN);
    //         	i2c_master_write_byte(cmd, 0x12, ACK_CHECK_EN);//Access IODIRA
    //         	i2c_master_write_byte(cmd, 0xF0, ACK_CHECK_EN);//
    //         	i2c_master_stop(cmd);
    //         i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
    //         	i2c_cmd_link_delete(cmd);

    uint8_t *mac[8] = {0};
    int haohao[8];
    char ok[12];
    esp_efuse_mac_get_default(&mac);
    sprintf(ok, "%02x%02x", (int)mac[0], (int)mac[1]);
    mac_addr[0] = ok[6];
    mac_addr[1] = ok[7];
    mac_addr[2] = ok[4];
    mac_addr[3] = ok[5];
    mac_addr[4] = ok[2];
    mac_addr[5] = ok[3];
    mac_addr[6] = ok[0];
    mac_addr[7] = ok[1];
    mac_addr[8] = ok[10];
    mac_addr[9] = ok[11];
    mac_addr[10] = ok[8];
    mac_addr[11] = ok[9];
    ESP_LOGI(TAG, "%s", ok);
    ESP_LOGI(TAG, "%s", mac_addr);
    vTaskDelay(10);

    sprintf(pub, "wuwo/f/%s", mac_addr);
    sprintf(sub, "wuwo/d/%s", mac_addr);
    // ESP_LOGI(TAG,"%s",sub);
    // ESP_LOGI(TAG,"%s",pub);

    uint8_t derived_mac_addr[6] = {0};
    ESP_ERROR_CHECK(esp_read_mac(derived_mac_addr, ESP_MAC_BT));
    sprintf(ok, "%x%x%x%x%x%x", derived_mac_addr[0], derived_mac_addr[1], derived_mac_addr[2], derived_mac_addr[3], derived_mac_addr[4], derived_mac_addr[5]);
    sprintf(pub, "wuwo/f/%s", ok);
    sprintf(sub, "wuwo/d/%s", ok);
    ESP_LOGI(TAG, "%s", sub);
    ESP_LOGI(TAG, "%s", pub);
    ESP_LOGI("BT MAC", "%s", ok);
    ESP_LOGI("BT MAC", "%x%x%x%x%x%x",
             derived_mac_addr[0], derived_mac_addr[1], derived_mac_addr[2],
             derived_mac_addr[3], derived_mac_addr[4], derived_mac_addr[5]);

    // xTaskCreate(mqtt_app_start, "mqtt_app_start", 8096 , NULL, 10, NULL);
    xTaskCreate(led_task, "led_task", 1024 * 2, NULL, 10, NULL);

    xTaskCreate(flashdb, "flashdn", 8096, NULL, 10, NULL);
    //  i2c_detect();

    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &periodic_timer_callback,
        /* name is optional, but may help identify the timer when debugging */
        .name = "periodic"};
    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, 100000));

    const esp_timer_create_args_t speaker_timer_args = {
        .callback = &speaker,
        /* name is optional, but may help identify the timer when debugging */
        .name = "speaker"};
    esp_timer_handle_t speaker_timer;
    ESP_ERROR_CHECK(esp_timer_create(&speaker_timer_args, &speaker_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(speaker_timer, 130));
    //    xTaskCreate(BS8112A, "beep", 1024 * 2, NULL, 10, NULL);
    //  xTaskCreate(endtime, "endtime", 1024 * 2, NULL, 10, NULL);
    //  xTaskCreate(BS8112A, "BS8112A", 1024 * 5, NULL, 5, NULL);
    //    xTaskCreate(flashdb, "flashdn", 1024 * 2, NULL, 10, NULL);

    xTaskCreate(&http_test_task1, "http_test_task", 1024 * 6, NULL, 10, NULL);
    gpio_set_level(6, 0);
    vTaskDelay(500);

    esp_mqtt_client_config_t mqtt_cfg = {
        //  .uri = "mqtt://42.193.154.49:1883",

        .host = "42.193.154.49",
        .port = 1883,
        .username = "wuwoiot",
        .password = "123+123",

    };

    vTaskDelay(100);
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    while (1)
    {

        if (wifi_connect == 1)
        {
            vTaskDelay(1000);
            esp_mqtt_client_start(client);
            ESP_LOGI(TAG, "开启MQTT");
            break;
        }

        vTaskDelay(1000);
    }
    while (1)
    {

        vTaskDelay(1000);

        ESP_LOGI(TAG, "开始订阅了");

        esp_mqtt_client_subscribe(client, sub, 0);
        ESP_LOGI(TAG, "订阅的主题是：%s", sub);
        if (sub_succeed == 1)
        {
            break; /* code */
        }
    }
    ESP_LOGI(TAG, "订阅结束");
    // esp_mqtt_client_subscribe(client, pub, 0);
    while (1)
    {

        if (shangbao_username == 1)
        {
            shangbao_username = 0;
            gpio_set_level(5, 0);
            vTaskDelay(1000);
            gpio_set_level(5, 1);
            //   open_success();
            mima_right = 1;
            sprintf(update, "{\n\"head\":\"state\",\n\"state\":\"1\",\n\"mode\":\"qr\",\n\"username\":\"%s\"\n}", username);
            esp_mqtt_client_publish(client, pub, update, 0, 1, 0);
        }

        if (shangbao == 1)
        {
            shangbao = 0;
            if (right == 0)
            {
                // open_fail();
                wrong = 1;
                sprintf(update, "{\n\"head\":\"state\",\n\"state\":\"9\",\n\"mode\":\"key\",\n\"num\":\"%s\"\n}", password);
                esp_mqtt_client_publish(client, pub, update, 0, 1, 0);
                // ESP_LOGI(TAG,"上报错误开锁");

                memset(password, 0, sizeof(password));
            }
            if (right == 1)
            {
                gpio_set_level(5, 0);
                vTaskDelay(1000);
                gpio_set_level(5, 1);
                // open_success();
                mima_right = 1;
                sprintf(update, "{\n\"head\":\"state\",\n\"state\":\"1\",\n\"mode\":\"key\",\n\"num\":\"%s\"\n}", password);
                esp_mqtt_client_publish(client, pub, update, 0, 1, 0);

                right = 0;
                // ESP_LOGI(TAG,"上报成功开锁%s",pub);
                memset(password, 0, sizeof(password));
            }
        }
        vTaskDelay(20);

    } //mqtt_connect
}
static void speaker(void *arg)
{
    if (wrong == 1)
    {
        speaker_cnt++;
        ledc_set_duty(LEDC_LS_MODE, LEDC_LS_CH0_CHANNEL, DATA[speaker_cnt] * 4);
        ledc_update_duty(LEDC_LS_MODE, LEDC_LS_CH0_CHANNEL);
        if (speaker_cnt == 13558)
        {
            speaker_cnt = 0;
            wrong = 0;
            ledc_set_duty(LEDC_LS_MODE, LEDC_LS_CH0_CHANNEL, 0);
            ledc_update_duty(LEDC_LS_MODE, LEDC_LS_CH0_CHANNEL);
        }
    }
    if (mima_right == 1)
    {
        speaker_cnt++;
        ledc_set_duty(LEDC_LS_MODE, LEDC_LS_CH0_CHANNEL, OPEN8[speaker_cnt] * 4);
        ledc_update_duty(LEDC_LS_MODE, LEDC_LS_CH0_CHANNEL);
        if (speaker_cnt == 14318)
        {
            speaker_cnt = 0;
            mima_right = 0;
            ledc_set_duty(LEDC_LS_MODE, LEDC_LS_CH0_CHANNEL, 0);
            ledc_update_duty(LEDC_LS_MODE, LEDC_LS_CH0_CHANNEL);
        }
    }
}
static void periodic_timer_callback(void *arg)
{
    pressed_old = pressed;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, 0xA0 | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, 0X08, ACK_CHECK_EN); //
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, 0xA0 | READ_BIT, ACK_CHECK_EN);
    //i2c_master_read_byte(cmd, &SHUJU,ACK_CHECK_DIS);
    // i2c_master_read(cmd, &SHUJU, 8, ACK_CHECK_DIS);//
    i2c_master_read_byte(cmd, &SHUJU, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    //      if (ret == ESP_OK)
    // {
    //     ESP_LOGI(TAG, "Write OK");
    // }
    // else if (ret == ESP_ERR_TIMEOUT)
    // {
    //   ESP_LOGW(TAG, "Bus is busy");
    // }
    // else
    // {
    //     ESP_LOGW(TAG, "Write Failed");
    // }
    vTaskDelay(30);
    // ESP_LOGI(TAG,"第一个寄存器%d",SHUJU);
    if (((SHUJU >> 0) & 0x01))
        pressed = '6';
    if (((SHUJU >> 1) & 0x01))
        pressed = '3';
    if (((SHUJU >> 2) & 0x01))
        pressed = '9';
    if (((SHUJU >> 3) & 0x01))
    {
        pressed = '#';
    }
    if (((SHUJU >> 4) & 0x01))
        pressed = '5';
    if (((SHUJU >> 5) & 0x01))
        pressed = '2';
    if (((SHUJU >> 6) & 0x01))
        pressed = '8';
    if (((SHUJU >> 7) & 0x01))
        pressed = '0';

    if (SHUJU == 0)
    {
        non = 0;
    }
    else
    {
        non = 1;
        led_count = 0;
    }

    i2c_cmd_handle_t cmd2 = i2c_cmd_link_create();

    i2c_master_start(cmd2);
    i2c_master_write_byte(cmd2, 0xA0 | I2C_MASTER_WRITE, ACK_CHECK_EN);
    i2c_master_write_byte(cmd2, 0X09, ACK_CHECK_EN); //
    i2c_master_start(cmd2);
    i2c_master_write_byte(cmd2, 0xA0 | I2C_MASTER_READ, ACK_CHECK_EN);
    i2c_master_read_byte(cmd2, &SHUJU1, ACK_CHECK_EN);
    i2c_master_stop(cmd2);
    ret = i2c_master_cmd_begin(I2C_NUM_0, cmd2, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd2);
    for (int i = 0; i < 4; i++)
    {
        if (((SHUJU1 >> i) & 0x01))
        {
            led_count = 0;
            if (i == 0)
            {
                pressed = '*';
                LED_START = 0;
                gpio_set_level(10, 1); //灯灭
                dijiwei = 0;
                memset(password, 0, sizeof(password));
            }
            if (i == 1)
                pressed = '7';
            if (i == 2)
                pressed = '1';
            if (i == 3)
                pressed = '4';

            non = 1;
        }
    }
    if (non == 0)
    {
        pressed = '!';
        // gpio_set_level(6,0);   //蜂鸣器不响
    }
    else
    {
        // gpio_set_level(6,1);   //蜂鸣器响
        gpio_set_level(10, 0); //灯亮
        if (pressed == '*')
            gpio_set_level(10, 1);
        LED_START = 1;
    }

    dog++;
    if (dog == 1000)
        dog = 0;
    if (pressed_old != pressed)
    {
        if (pressed == '#')
        {
            led_count = 0;
            compare();
            dijiwei = 0;
        }
        if (pressed != '!' && pressed != '*' && pressed != '#')
        {
            password[dijiwei] = pressed;
            dijiwei++;

            ESP_LOGI(TAG, "整体的密码是:%s", password);
            // ESP_LOGI(TAG,"现在的:%c",pressed);
        }
        if (pressed != '!')
        {
            gpio_set_level(6, 1);
            vTaskDelay(80);
            gpio_set_level(6, 0);
        }
    }
}
