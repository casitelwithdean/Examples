
#include <stdio.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "protocol_examples_common.h"
#include "lwip/apps/sntp.h"
#include "sh2lib.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "mbedtls/md5.h"
#include "mbedtls/sha1.h"
#include "mbedtls/base64.h"
#include "esp_websocket_client.h"
#include "esp_event.h"
#include "driver/i2s.h"
#include "driver/gpio.h"
#include "sha1.h"
#include "cJSON.h"
#include "esp_http_client.h"
#include "lwip/apps/sntp.h"
//麦克风模块  SCK20    WS19   SD21   LR接地 
#define KEY_OFF 12  //停止录音键
#define BD_S 13  //百度短语音
#define BD_L 14  //百度长
#define XF_S 15  //讯飞短
#define XF_L 16  //讯飞长
#define LED 5   //灯 

unsigned char asr_key[] = "2e3a45385892d13ec42e2ef7d8f14454";//讯飞KEY
char xf_appid[30] = "9a94c591";                           //讯飞ID
char *appid = "27668235";                             //百度
char *appkey = "avpH1pczKXhXWrVKCAdKIdvS";            //百度
char *secretkey = "ZdpaCeYqHZHA6jEswLQQvhvgF9fpCof8"; //百度
int stop = 0;
void baidu_post();
void key_task();
int sec = 0;
int post_done = 0;
int baidu_s = 0;
int baidu_l = 0;
int xf = 0;

#define ECHO_TEST_TXD (1)
#define ECHO_TEST_RXD (3)
#define ECHO_TEST_RTS (UART_PIN_NO_CHANGE)
#define ECHO_TEST_CTS (UART_PIN_NO_CHANGE)
#define ECHO_UART_PORT_NUM (2)
#define ECHO_UART_BAUD_RATE (115200)
#define ECHO_TASK_STACK_SIZE (4096)
int init_start = 0;
#define MAX_HTTP_OUTPUT_BUFFER 2048
const i2s_port_t I2S_PORT = I2S_NUM_0;
#define NO_DATA_TIMEOUT_SEC 100
#define pcm_data_len 16000
uint8_t i2s_data[pcm_data_len] = {0};
static const char *TAG = "WEBSOCKET";
char start_frame[300] = {0};
char finish_frame[30] = {0};
char cancel_frame[30] = {0};
char *xfend_frame = "{\"end\": true}";
int32_t sBuffer[8];
// static TimerHandle_t shutdown_signal_timer;
// static SemaphoreHandle_t shutdown_sema;
unsigned char ts[] = "             ";
char timestamp[20] = {0};
char rec_data[5000];
char rec_json[5000];

char *bd_result = "                                                                           ";

char access_token[200] = {0};
char xf_url[200] = {0};
int get_token = 0;
#define BUF_SIZE (1024)
char json[1500] = "{\"action\":\"result\", \"code\":\"0\",\"data\":{\"cn\":{\"st\":{\"bg\":\"820\",\"ed\":\"0\",\"rt\":[{\"ws\":[{\"cw\":[{\"w\":\"啊\",\"wp\":\"n\"}],\"wb\":0,\"we\":0},{\"cw\":[{\"w\":\"喂\",\"wp\":\"n\"}],\"wb\":0,\"we\":0},{\"cw\":[{\"w\":\"！\",\"wp\":\"p\"}],\"wb\":0,\"we\":0},{\"cw\":[{\"w\":\"你好\",\"wp\":\"n\"}],\"wb\":0,\"we\":0},{\"cw\":[{\"w\":\"！\",\"wp\":\"p\"}],\"wb\":0,\"we\":0},{\"cw\":[{\"w\":\"我\",\"wp\":\"n\"}],\"wb\":0,\"we\":0},{\"cw\":[{\"w\":\"是\",\"wp\":\"n\"}],\"wb\":0,\"we\":0},{\"cw\":[{\"w\":\"上\",\"wp\":\"n\"}],\"wb\":0,\"we\":0}]}],\"type\":\"1\"}},\"seg_id\":5},\"desc\":\"success\",\"sid\":\"rta0000000e@ch312c0e3f6bcc9f0900\"}";
#define HTTP2_SERVER_URI "https://aip.baidubce.com"
char HTTP2_STREAMING_GET_PATH[200] = {0};
int handle_get_response(struct sh2lib_handle *handle, const char *data, size_t len, int flags)
{
    if (len)
    {
        printf("[get-response] %.*s\n", len, data);
        char json[1299] = {0};
        sprintf(json, "%.*s", len, data);
        cJSON *root = NULL;
        //printf("[get-ddddddddddddddd] %s\n", json);
        root = cJSON_Parse(json);
        if (!root)
        {
            printf("Error before: [%s]\n", cJSON_GetErrorPtr());
        }
        else
        {

            char *item = cJSON_GetObjectItem(root, "access_token")->valuestring; //
            sprintf(access_token, "%s", item);
            printf("[get-dd] %s\n", access_token);
        }
        cJSON_Delete(root);
        get_token = 1;
    }
    if (flags == DATA_RECV_FRAME_COMPLETE)
    {
        printf("[get-response] Frame fully received\n");
    }
    if (flags == DATA_RECV_RST_STREAM)
    {
        printf("[get-response] Stream Closed\n");
    }
    return 0;
}

static void set_time(void)
{
    struct timeval tv = {
        .tv_sec = 1509449941,
    };
    struct timezone tz = {
        0, 0};
    settimeofday(&tv, &tz);

    /* Start SNTP service */
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_init();
}

static void http2_task(void *args)
{
    /* Set current time: proper system time is required for TLS based
     * certificate verification.
     */
    set_time();

    /* HTTP2: one connection multiple requests. Do the TLS/TCP connection first */
    printf("Connecting to server\n");
    struct sh2lib_handle hd;
    if (sh2lib_connect(&hd, HTTP2_SERVER_URI) != 0)
    {
        printf("Failed to connect\n");
        vTaskDelete(NULL);
        return;
    }
    printf("Connection done\n");

    /* HTTP GET  */
    sh2lib_do_get(&hd, HTTP2_STREAMING_GET_PATH, handle_get_response);

    while (1)
    {
        /* Process HTTP2 send/receive */
        if (sh2lib_execute(&hd) < 0)
        {
            printf("Error in send/receive\n");
            break;
        }
        vTaskDelay(2);
    }

    sh2lib_free(&hd);
    vTaskDelete(NULL);
}
static void echo_task(void *arg)
{

    /* Configure parameters of an UART driver,
     * communication pins and install the driver */
    uart_config_t uart_config = {
        .baud_rate = ECHO_UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    int intr_alloc_flags = 0;

#if CONFIG_UART_ISR_IN_IRAM
    intr_alloc_flags = ESP_INTR_FLAG_IRAM;
#endif

    ESP_ERROR_CHECK(uart_driver_install(ECHO_UART_PORT_NUM, BUF_SIZE * 2, 0, 0, NULL, intr_alloc_flags));
    ESP_ERROR_CHECK(uart_param_config(ECHO_UART_PORT_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(ECHO_UART_PORT_NUM, ECHO_TEST_TXD, ECHO_TEST_RXD, ECHO_TEST_RTS, ECHO_TEST_CTS));

    // Configure a temporary buffer for the incoming data
    uint8_t *data = (uint8_t *)malloc(BUF_SIZE);

    while (1)
    {
        // Read data from the UART
        int len = uart_read_bytes(ECHO_UART_PORT_NUM, data, BUF_SIZE, 20 / portTICK_RATE_MS);
        if (len > 3)
        {
            char uart_data[200] = {0};
            sprintf(uart_data, "%c%c%c%c", data[0], data[1], data[2], data[3]);
            ESP_LOGI(TAG, "串口接受到的数据：“%s”", uart_data);
            if (strcmp(uart_data, "bd_d") == 0)
            {
                baidu_l = 0;
                baidu_s = 1;
                xf = 0;
                stop = 0;

                ESP_LOGI(TAG, "百度短");
            }
            if (strcmp(uart_data, "bd_c") == 0)
            {
                baidu_l = 1;
                baidu_s = 0;
                xf = 0;
                stop = 0;

                ESP_LOGI(TAG, "百度长");
            }
            if (strcmp(uart_data, "xf_d") == 0)
            {
                xf = 1;
                baidu_l = 0;
                baidu_s = 0;
                stop = 0;

                ESP_LOGI(TAG, "讯飞短");
            }
            if (strcmp(uart_data, "xf_c") == 0)
            {
                xf = 1;
                baidu_l = 0;
                baidu_s = 0;
                stop = 0;

                ESP_LOGI(TAG, "讯飞长");
            }
            if (strcmp(uart_data, "stop") == 0)
            {
                xf = 0;
                baidu_l = 0;
                baidu_s = 0;
                stop = 1;
                ESP_LOGI(TAG, "停止");
            }
        }
        vTaskDelay(10);
    }
}
static void shutdown_signaler(TimerHandle_t xTimer)
{
    ESP_LOGI(TAG, "No data received for %d seconds, signaling shutdown", NO_DATA_TIMEOUT_SEC);
    // xSemaphoreGive(shutdown_sema);
}
void record();
void get_timestamp();

void printJson(cJSON *root) //以递归的方式打印json的最内层键值对
{
    for (int i = 0; i < cJSON_GetArraySize(root); i++)
    {
        cJSON *item = cJSON_GetArrayItem(root, i);
        if (cJSON_Object == item->type)
            printJson(item);
        else
        {
            printf("%s->", item->string);
        }
    }
}
void i2s_init()
{

    unsigned char encrypt[19];
    for (int i = 0; i < sizeof(xf_appid) - 1; i++)
    {
        encrypt[i] = xf_appid[i];
    }
    for (int i = 8; i < 19; i++)
    {
        encrypt[i] = timestamp[i - 8];
    }

    // printf("这是xppid+ts%d     %s ", sizeof(encrypt), encrypt);

    printf("MD5 加密\n");
    mbedtls_md5_context md5_ctx;
    unsigned char md5_decrypt[16];
    char md5_base[33] = {0};
    mbedtls_md5_init(&md5_ctx);
    mbedtls_md5_starts_ret(&md5_ctx);
    mbedtls_md5_update_ret(&md5_ctx, encrypt, 18);
    mbedtls_md5_finish_ret(&md5_ctx, md5_decrypt);
    printf("MD5加密前:[%s]\n", encrypt);
    printf("MD5加密后(32位):");
    // for (int i = 0; i < 16; i++)
    // {
    //     printf("%02x", md5_decrypt[i]);
    // }
    sprintf(md5_base, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", md5_decrypt[0], md5_decrypt[1], md5_decrypt[2], md5_decrypt[3], md5_decrypt[4], md5_decrypt[5], md5_decrypt[6], md5_decrypt[7], md5_decrypt[8], md5_decrypt[9], md5_decrypt[10], md5_decrypt[11], md5_decrypt[12], md5_decrypt[13], md5_decrypt[14], md5_decrypt[15]);
    printf("\r\n");
    printf("MD5加密后的字符串%s\r\n", md5_base);
    mbedtls_md5_free(&md5_ctx);

    unsigned char signa[20] = {0};
   
    unsigned char basestring[32];
    for (int i = 0; i < 32; i++)
    {
        basestring[i] = md5_base[i];
    }
    // printf("sha测试changdu%d\n", sizeof(md5_decrypt));

    hmac_sha1(asr_key, sizeof(asr_key) - 1, basestring, 32, signa);
    char final_signa[41] = {0};

    // printf("结果是=\n");
    // sprintf(final_signa, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
    //         signa[0], signa[1], signa[2], signa[3], signa[4],
    //         signa[5], signa[6], signa[7], signa[8], signa[9],
    //         signa[10], signa[11], signa[12], signa[13], signa[14],
    //         signa[15], signa[16], signa[17], signa[18], signa[19]);
    // printf("sha5加密:[%s]\n", final_signa);

    unsigned char xf_signa[132] = {0};
    // unsigned char temp_signa[132] = {0};
    // for (int i = 0; i < 40; i++)
    // {
    //     temp_signa[i] = final_signa[i];
    // }

    size_t len;
    mbedtls_base64_encode(xf_signa, sizeof(xf_signa) - 1, &len, signa, 20);

    char xfsigna[70] = {0};
    for (int i = 0; i < len; i++)
    {
        xfsigna[i] = xf_signa[i];
    }
    printf("\r\n%s%s\r\n", xfsigna, xfsigna);
    printf("\r\n%d\r\n", len);

    sprintf(xf_url, "ws://rtasr.xfyun.cn/v1/ws?appid=%s&ts=%s&signa=%s", xf_appid, timestamp, xfsigna);
    sprintf(finish_frame, "{\"type\":\"FINISH\"}");
    sprintf(cancel_frame, "{\"type\":\"CANCEL\"}");
    sprintf(start_frame, "{\"type\":\"START\",\"data\": {\"appid\": %s,\"appkey\":\"%s\",\"dev_pid\": 15372,\"cuid\": \"cui111\",\"format\":\"pcm\",\"sample\": 16000}}", appid, appkey);

    esp_err_t err;

    // The I2S config as per the example
    const i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX), // Receive, not transfer
        .sample_rate = 16000,                                // 16KHz
        .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,        // could only get it to work with 32bits
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,         // use right channel
        .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1, // Interrupt level 1
        .dma_buf_count = 4,                       // number of buffers
        .dma_buf_len = 8                          // 8 samples per buffer (minimum)
    };

    // The pin config as per the setup
    const i2s_pin_config_t pin_config = {
        .bck_io_num = 20,                  // Serial Clock (SCK)
        .ws_io_num = 19,                   // Word Select (WS)
        .data_out_num = I2S_PIN_NO_CHANGE, // not used (only for speakers)
        .data_in_num = 21                  // Serial Data (SD)
    };

    err = i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
    if (err != ESP_OK)
    {
        //Serial.printf("Failed installing driver: %d\n", err);
        while (true)
            ;
    }
    err = i2s_set_pin(I2S_PORT, &pin_config);
    if (err != ESP_OK)
    {
        //  Serial.printf("Failed setting pin: %d\n", err);
        while (true)
            ;
    }
}
static void websocket_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_websocket_event_data_t *data = (esp_websocket_event_data_t *)event_data;
    switch (event_id)
    {
    case WEBSOCKET_EVENT_CONNECTED:
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_CONNECTED");
        break;
    case WEBSOCKET_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_DISCONNECTED");
        break;
    case WEBSOCKET_EVENT_DATA:
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_DATA");
        ESP_LOGI(TAG, "Received opcode=%d", data->op_code);
        if (data->op_code == 0x08 && data->data_len == 2)
        {
            ESP_LOGW(TAG, "Received closed message with code=%d", 256 * data->data_ptr[0] + data->data_ptr[1]);
        }
        else
        {
            ESP_LOGW(TAG, "Received=%.*s", data->data_len, (char *)data->data_ptr);
        }
        ESP_LOGW(TAG, "Total payload length=%d, data_len=%d, current payload offset=%d\r\n", data->payload_len, data->data_len, data->payload_offset);

        if (baidu_l == 1)
        {
            char bd_temp[700] = {0};
            sprintf(bd_temp, "%.*s", data->data_len, (char *)data->data_ptr);
            cJSON *bd = cJSON_Parse(bd_temp);
            if (!bd)
            {
                printf("Error before: [%s]\n", cJSON_GetErrorPtr());
            }
            else
            {
                char *type = "                          ";
                type = cJSON_GetObjectItem(bd, "type")->valuestring; //
                if (strcmp(type, "HEARTBEAT") != 0 && strcmp(type, "FIN_TEXT") == 0)
                {
                    bd_result = cJSON_GetObjectItem(bd, "result")->valuestring; //
                    if (bd_result != NULL)
                    {
                        printf("%s", bd_result);
                        uart_write_bytes(ECHO_UART_PORT_NUM, (const char *)bd_result, strlen(bd_result));
                    }
                }
            }
        }
        if (xf == 1 && data->data_len > 3)
        {

            char *result = "          ";
            cJSON *root = NULL;
            cJSON *rootd = NULL;
            root = cJSON_Parse((char *)data->data_ptr);

            result = cJSON_GetObjectItem(root, "action")->valuestring;
            if (strcmp(result, "success") == 0)
                gpio_set_level(LED, 1);
            if (strcmp(result, "result") == 0)
            {
                result = cJSON_GetObjectItem(root, "data")->valuestring;
                printf("数据%s\n", result);
                cJSON *cd = NULL;
                rootd = cJSON_Parse(result);
                cd = cJSON_GetObjectItem(rootd, "cn"); //
                if (!cd)
                {
                    printf("Error before: [%s]\n", cJSON_GetErrorPtr());
                }

                cJSON *item1 = cJSON_GetObjectItem(cd, "st");
                if (item1 != NULL)
                {
                    char *is_end = cJSON_GetObjectItem(item1, "type")->valuestring;
                    if (strcmp(is_end, "0") == 0)
                    {
                        cJSON *rt = cJSON_GetObjectItem(item1, "rt");
                        cJSON *rt_list = rt->child;
                        cJSON *ip_arry = cJSON_GetObjectItem(rt_list, "ws"); //clientlist 是使用 cjson对象
                        if (NULL != ip_arry)
                        {
                            cJSON *client_list = ip_arry->child;
                            while (client_list != NULL)
                            {
                                cJSON *cw = cJSON_GetObjectItem(client_list, "cw");
                                if (NULL != cw)
                                {
                                    cJSON *wlist = cw->child;
                                    char *han = cJSON_GetObjectItem(wlist, "w")->valuestring;
                                    printf("%s", han);
                                    uart_write_bytes(ECHO_UART_PORT_NUM, (const char *)han, strlen(han));

                                    cJSON_Delete(wlist); // 释放内存
                                }
                                client_list = client_list->next;
                            }
                        }
                        printf("        %s\n", "解析正常");
                    }
                }

                // memset(rec_data, 0, 5000);
                // sprintf(rec_data, "%.*s", data->data_len, (char *)data->data_ptr);
                // sprintf(rec_data, "%s", strstr(rec_data, "{\\\"rt"));
                // printf("%d     %d     %s\n\n", strlen(rec_data), sizeof(rec_data), rec_data);
                // int pos = 0;
                // for (int i = 11; i < strlen(rec_data); i++)
                // {
                //     if (rec_data[i] == ',' && rec_data[i + 1] == '\\' && rec_data[i + 2] == 'b' && rec_data[i + 1] == 'g')
                //     {
                //         rec_data[i] = '}';
                //         pos = i + 1;
                //         break;
                //     }
                // }
                // sprintf(rec_json, "%.*s", pos, rec_data);
                // printf("%s\n\n", rec_json);
                // for (int i = 0; i < strlen(rec_json); i++)
                // {
                //     if (rec_json[i] == 92)
                //         rec_json[i] = ' ';
                // }
                // printf("%s\n\n", rec_json);

                // cJSON *rec = NULL;
                // rec = cJSON_Parse(rec_json);
                // cJSON *item = NULL; //cjson对象

                // if (!rec)
                // {
                //     printf("Error before: [%s]\n", cJSON_GetErrorPtr());
                // }
                // else
                // {

                //     item = cJSON_GetObjectItem(rec, "cn"); //
                //                                            // printf("%s\n", cJSON_Print(item));//内存泄漏
                //     if (!item)
                //     {
                //         printf("Error before: [%s]\n", cJSON_GetErrorPtr());
                //     }
                //     printf("%s\n", "获取slots下的cjson对象");
                //     // printf("%s\n", "获取name下的cjson对象");
                //     // char *AA="                         ";
                //     //    int  AAa= cJSON_GetObjectItem(rec, "seg_id")->valueint;
                //     //                         printf("%d\n", AAa);//内存泄漏
                //     cJSON *item1 = cJSON_GetObjectItem(item, "st");

                //     // item = cJSON_GetObjectItem(item, "st");
                //     // item = cJSON_GetObjectItem(item, "rt");

                //     if (item != NULL)
                //     {
                //         cJSON *rt = cJSON_GetObjectItem(item, "rt");
                //         cJSON *rt_list = rt->child;
                //         cJSON *ip_arry = cJSON_GetObjectItem(rt_list, "ws"); //clientlist 是使用 cjson对象
                //         if (NULL != ip_arry)
                //         {
                //             cJSON *client_list = ip_arry->child;
                //             while (client_list != NULL)
                //             {
                //                 cJSON *cw = cJSON_GetObjectItem(client_list, "cw");
                //                 if (NULL != cw)
                //                 {
                //                     cJSON *wlist = cw->child;
                //                     char *han = cJSON_GetObjectItem(wlist, "w")->valuestring;
                //                     printf("%s", han);
                //                 }
                //                 client_list = client_list->next;
                //             }
                //         }
                //         printf("%s\n", "一切都ok");
                //     }

                //     // printJson(root);
                // }
                if (root)
                    cJSON_Delete(root); // 释放内存
            }
        }
        // xTimerReset(shutdown_signal_timer, portMAX_DELAY);
        break;
    case WEBSOCKET_EVENT_ERROR:
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_ERROR");
        break;
    }
}

static void websocket_app_start(void)
{
    esp_websocket_client_config_t websocket_cfg = {};

    while (1)
    {

        if (baidu_s == 1)
        {
            baidu_post();
        }
        if (baidu_l == 1)
        {
            websocket_cfg.buffer_size = 4 * 1024;
            websocket_cfg.uri = "wss://vop.baidu.com/realtime_asr?sn=AAAA-aaaa-AAAA-aaaa";
            ESP_LOGI(TAG, "Connecting to %s...", websocket_cfg.uri);

            esp_websocket_client_handle_t client = esp_websocket_client_init(&websocket_cfg);
            esp_websocket_register_events(client, WEBSOCKET_EVENT_ANY, websocket_event_handler, (void *)client);

            esp_websocket_client_start(client);
            vTaskDelay(2);
            // xTimerStart(shutdown_signal_timer, portMAX_DELAY);
            char data[32];
            gpio_set_level(LED, 1);
            esp_websocket_client_send_text(client, start_frame, strlen(start_frame), portMAX_DELAY);
            while (stop == 0)
            {
                if (esp_websocket_client_is_connected(client))
                {
                    record(160);
                    esp_websocket_client_send_bin(client, (char *)i2s_data, 5120, portMAX_DELAY);
                }
            }
            esp_websocket_client_send_bin(client, xfend_frame, strlen(xfend_frame), portMAX_DELAY);
            esp_websocket_client_send_text(client, finish_frame, strlen(finish_frame), portMAX_DELAY);
            gpio_set_level(LED, 0);

            // xSemaphoreTake(shutdown_sema, portMAX_DELAY);
            esp_websocket_client_close(client, portMAX_DELAY);
            ESP_LOGI(TAG, "Websocket Stopped");
            esp_websocket_client_destroy(client);
        }
        if (xf == 1)
        {

            websocket_cfg.uri = xf_url;
            websocket_cfg.buffer_size = 4 * 1024;
            ESP_LOGI(TAG, "Connecting to %s...", websocket_cfg.uri);

            esp_websocket_client_handle_t client = esp_websocket_client_init(&websocket_cfg);
            esp_websocket_register_events(client, WEBSOCKET_EVENT_ANY, websocket_event_handler, (void *)client);

            esp_websocket_client_start(client);
            // xTimerStart(shutdown_signal_timer, portMAX_DELAY);
            char data[32];
            gpio_set_level(LED, 1);
            while (stop == 0)
            {
                if (esp_websocket_client_is_connected(client))
                {
                    record(40);
                    esp_websocket_client_send_bin(client, (char *)i2s_data, 1280, portMAX_DELAY);
                }
            }
            esp_websocket_client_send_bin(client, (char *)xfend_frame, strlen(xfend_frame), portMAX_DELAY);

            esp_websocket_client_close(client, portMAX_DELAY);
            gpio_set_level(LED, 0);
            ESP_LOGI(TAG, "Websocket Stopped");
            esp_websocket_client_destroy(client);
        }
        vTaskDelay(10);
    }
    vTaskDelete(NULL);
}

void app_main(void)
{
    gpio_config_t io_conf;
    io_conf.pin_bit_mask = (1ULL << XF_L) | (1ULL << XF_S) | (1ULL << BD_L) | (1ULL << BD_S) | (1ULL << KEY_OFF);
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = 0;
    io_conf.pull_down_en = 1;
    gpio_config(&io_conf);
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << LED);
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);
    gpio_set_level(LED, 0);
    sprintf(HTTP2_STREAMING_GET_PATH, "/oauth/2.0/token?grant_type=client_credentials&client_id=%s&client_secret=%s", appkey, secretkey);

    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());
    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("WEBSOCKET_CLIENT", ESP_LOG_DEBUG);
    esp_log_level_set("TRANSPORT_WS", ESP_LOG_DEBUG);
    esp_log_level_set("TRANS_TCP", ESP_LOG_DEBUG);
    xTaskCreate(echo_task, "uart_echo_task", ECHO_TASK_STACK_SIZE, NULL, 10, NULL);
    xTaskCreate(&key_task, "key_task", (1024 * 6), NULL, 5, NULL);
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    ESP_ERROR_CHECK(example_connect());

    vTaskDelay(1);
    xTaskCreate(&http2_task, "http2_task", (1024 * 24), NULL, 10, NULL);
    xTaskCreate(&get_timestamp, "http_test_task", 1024 * 8, NULL, 10, NULL);

    while (!init_start)
    {
        /* code */ vTaskDelay(10);
    }

    i2s_init();
    while (!get_token)
    {
        /* code */ vTaskDelay(10);
    }
    // websocket_app_start();
    xTaskCreate(&websocket_app_start, "websocket", 1024 * 24, NULL, 5, NULL);
}
void record(int ms)
{
    for (int a = 0; a < ms * 2; a++) //160/0.5
    {
        size_t bytes_read = 0;
        i2s_read(I2S_PORT, sBuffer, sizeof(sBuffer), &bytes_read, portMAX_DELAY);
        if (bytes_read > 0)
        {
            for (int i = 0; i < 8; i++)
            {
                i2s_data[a * 16 + 2 * i + 1] = (sBuffer[i] >> 13) >> 8; //xf
                i2s_data[a * 16 + 2 * i] = (sBuffer[i] >> 13) & 0xff;   //xf
            }
        }
    }
}

void get_timestamp()
{

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

                    cJSON *root = NULL;
                    root = cJSON_Parse(output_buffer);

                    if (root != NULL)
                    {
                        cJSON *root1 = cJSON_GetObjectItem(root, "data");
                        if (root1 != NULL)
                        {
                            char *ts = "                        ";
                            ts = cJSON_GetObjectItem(root1, "t")->valuestring;

                            for (int i = 0; i < 10; i++)
                                timestamp[i] = ts[i];
                            ESP_LOGI(TAG, "时间戳%s", timestamp);
                            esp_http_client_close(client);
                            init_start = 1;
                            cJSON_Delete(root);
                            vTaskDelete(NULL);
                        }
                    }

                    cJSON_Delete(root);
                }
                else
                {
                    ESP_LOGE(TAG, "Failed to read response");
                }
            }
        }

        esp_http_client_close(client);
        vTaskDelay(3000 / portTICK_PERIOD_MS);
    }
}
esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    static char *output_buffer; // Buffer to store response of http request from event handler
    static int output_len;      // Stores number of bytes read
    switch (evt->event_id)
    {
    case HTTP_EVENT_ERROR:
        ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
        break;
    case HTTP_EVENT_ON_CONNECTED:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
        break;
    case HTTP_EVENT_HEADER_SENT:
        ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
        break;
    case HTTP_EVENT_ON_HEADER:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
        break;
    case HTTP_EVENT_ON_DATA:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
        /*
             *  Check for chunked encoding is added as the URL for chunked encoding used in this example returns binary data.
             *  However, event handler can also be used in case chunked encoding is used.
             */
        if (!esp_http_client_is_chunked_response(evt->client))
        {
            // If user_data buffer is configured, copy the response into the buffer
            if (evt->user_data)
            {
                memcpy(evt->user_data + output_len, evt->data, evt->data_len);
            }
            else
            {
                if (output_buffer == NULL)
                {
                    output_buffer = (char *)malloc(esp_http_client_get_content_length(evt->client));
                    output_len = 0;
                    if (output_buffer == NULL)
                    {
                        ESP_LOGE(TAG, "Failed to allocate memory for output buffer");
                        return ESP_FAIL;
                    }
                }
                memcpy(output_buffer + output_len, evt->data, evt->data_len);
            }
            output_len += evt->data_len;
        }

        break;
    case HTTP_EVENT_ON_FINISH:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
        if (output_buffer != NULL)
        {
            // Response is accumulated in output_buffer. Uncomment the below line to print the accumulated response
            // ESP_LOG_BUFFER_HEX(TAG, output_buffer, output_len);
            free(output_buffer);
            output_buffer = NULL;
        }
        output_len = 0;
        break;
    case HTTP_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
        int mbedtls_err = 0;
        esp_err_t err = esp_tls_get_and_clear_last_error(evt->data, &mbedtls_err, NULL);
        if (err != 0)
        {
            if (output_buffer != NULL)
            {
                free(output_buffer);
                output_buffer = NULL;
            }
            output_len = 0;
            ESP_LOGI(TAG, "Last esp error code: 0x%x", err);
            ESP_LOGI(TAG, "Last mbedtls failure: 0x%x", mbedtls_err);
        }
        break;
    }
    return ESP_OK;
}
void baidu_post()
{

    char local_response_buffer[MAX_HTTP_OUTPUT_BUFFER] = {0};
    esp_http_client_config_t config = {
        .url = "http://vop.baidu.com",
        .event_handler = _http_event_handler,
        .user_data = local_response_buffer,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_err_t err;

    sec = 50;
    gpio_set_level(LED, 1);
    uint8_t *line;
    line = heap_caps_malloc(16000 * 60 * 2 * sizeof(uint8_t), MALLOC_CAP_SPIRAM);
    assert(line != NULL);
    for (int a = 0; a < 16000 * 60 / 8; a++)
    {
        size_t bytes_read = 0;
        i2s_read(I2S_PORT, sBuffer, sizeof(sBuffer), &bytes_read, portMAX_DELAY);
        if (bytes_read > 0)
        {
            for (int i = 0; i < 8; i++)
            {

                line[a * 16 + 2 * i] = (sBuffer[i] >> 13) & 0xff;   //xf
                line[a * 16 + 2 * i + 1] = (sBuffer[i] >> 13) >> 8; //xf
            }
        }
        if (stop == 1)
        {
            sec = a / 2000;
            break;
        }
    }
    gpio_set_level(LED, 0);

    char baidu_url[300] = {0};
    sprintf(baidu_url, "/server_api?dev_pid=1537&cuid=12321&token=%s", access_token);
    esp_http_client_set_url(client, baidu_url);
    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_header(client, "Content-Type", "audio/pcm;rate=16000");
    esp_http_client_set_post_field(client, (char *)line, 16000 * sec * 2);

    err = esp_http_client_perform(client);
    if (err == ESP_OK)
    {
        ESP_LOGI(TAG, "HTTP POST Status = %d, content_length = %d",
                 esp_http_client_get_status_code(client),
                 esp_http_client_get_content_length(client));
    }
    else
    {
        ESP_LOGE(TAG, "HTTP POST request failed: %s", esp_err_to_name(err));
    }
    ESP_LOGI(TAG, "%s", local_response_buffer);
    cJSON *bd_short = NULL;
    bd_short = cJSON_Parse(local_response_buffer);
    if (!bd_short)
    {
        printf("Error before: [%s]\n", cJSON_GetErrorPtr());
    }
    else
    {
        cJSON *item = cJSON_GetObjectItem(bd_short, "result"); //
        char *ArrNumEle = cJSON_GetArrayItem(item, 0)->valuestring;
        printf("[get-data] %s\n", ArrNumEle);
        uart_write_bytes(ECHO_UART_PORT_NUM, (const char *)ArrNumEle, strlen(ArrNumEle));
    }
    cJSON_Delete(bd_short);
    esp_http_client_cleanup(client);
}
void key_task()
{

    while (1)
    {
        if (gpio_get_level(BD_S) == 1)
        {
            baidu_l = 0;
            baidu_s = 1;
            xf = 0;
            stop = 0;

            ESP_LOGI(TAG, "百度短");
        }
        if (gpio_get_level(BD_L) == 1)
        {
            baidu_l = 1;
            baidu_s = 0;
            xf = 0;
            stop = 0;

            ESP_LOGI(TAG, "百度长");
        }
        if (gpio_get_level(XF_S) == 1)
        {
            xf = 1;
            baidu_l = 0;
            baidu_s = 0;
            stop = 0;

            ESP_LOGI(TAG, "讯飞短");
        }
        if (gpio_get_level(XF_L) == 1)
        {
            xf = 1;
            baidu_l = 0;
            baidu_s = 0;
            stop = 0;
            ESP_LOGI(TAG, "讯飞长");
        }
        if (gpio_get_level(KEY_OFF) == 1)
        {
            xf = 0;
            baidu_l = 0;
            baidu_s = 0;
            stop = 1;
            ESP_LOGI(TAG, "停止");
        }
        vTaskDelay(10 / portTICK_RATE_MS);
    }
    vTaskDelete(NULL);
}