/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "flash.h"
#include "string.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
uint8_t  ubuf[64];
#define UI_printf(...) HAL_UART_Transmit(&huart3, \
                  (uint8_t *)ubuf, \
                 sprintf((char *)ubuf,__VA_ARGS__), \
                  0xffff)
uint8_t init_mode=8;//前三位
	uint8_t on_off=1;
	uint8_t fuckyou=8;//前三?
	uint8_t on=0;
	uint8_t temperature=26;
uint8_t wind=0;
								 int wind_for_bt=0;
								 int uart_mode=0;
								 		 int uart_wind=0;
								 int uart_temp=0;
								int no_temp=0;
	void turn_on();
	int beijian=0;
	char oled_temp[10]={0};
		char oled_onoff[10]={0};
			char oled_mode[10]={0};
				char oled_wind[10]={0};
	uint8_t aRxBuffer[111];
 uint8_t TEXT_Buffer[]={"STM32 FLASH TEST"};
#define SIZE sizeof(TEXT_Buffer)	 	//数组长度
#define FLASH_SAVE_ADDR  0x0800F000 	//设置FLASH 保存地址(必须为偶数，且其值要大于本代码所占用FLASH的大小+0X08000000)
//#define FLASH_SAVE_ADDR_Mode  0X08002000 	//设置FLASH 保存地址(必须为偶数，且其值要大于本代码所占用FLASH的大小+0X08000000)

uint8_t datatemp[SIZE];
uint8_t shabi[SIZE];
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
uint16_t count=0;
void user_delaynus_tim(uint32_t nus)
{
 
 uint16_t  differ = 0xffff-nus-5;

  __HAL_TIM_SetCounter(&htim2,differ);

  HAL_TIM_Base_Start(&htim2);
 
  while( differ<0xffff-5)
 {
  differ = __HAL_TIM_GetCounter(&htim2);
 };

  HAL_TIM_Base_Stop(&htim2);
}
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#define delayUs(x) { unsigned int _dcnt; \
      _dcnt=(x*16); \
      while(_dcnt-- > 0) \
      { continue; }\
     }
uint32_t count1=0;

void code(uint8_t arg)
{
	__HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_1, 8);
		user_delaynus_tim(560);
		__HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_1, 0);
			if(arg==0)
			{
user_delaynus_tim(560);
			}
				else
			{
user_delaynus_tim(1680);
			}
}
void code_temp_rev(uint8_t arg)
{
	int temp;
	for(int dock=0;dock<4;dock++)
	{
		if(init_mode==2)beijian=6;
		if(init_mode==4)beijian=4;
		if(init_mode==8)beijian=3;
		if(init_mode==12)beijian=5;


		temp=((arg-16+beijian)%16)>>dock;
		temp=temp&0x01;
		

		if(dock==3)
		{
			if(on_off==0)
			{
				if(temp==0)temp=1;
				if(temp==1)temp=0;
			}
			
		}
		if(init_mode==0)
			{
				if(dock==0)temp=1;
				if(dock==1)temp=1;
				if(dock==2)temp=0;
				if(dock==3)
				{
						if(on_off==0)
			{
			temp=0;
			}
			else
			{
					temp=1;
			}
				}
			}
code(temp);
	}
}
//void code_temp_rev(uint8_t arg)
//{
//	int temp;
//	for(int dock=0;dock<4;dock++)
//	{
//		temp=((arg-13)%16)>>dock;
//		temp=temp&0x01;
//code(temp);
//	}
//}


void code_temp(uint8_t arg)
{
	int temp;
	for(int dock=0;dock<4;dock++)
	{
		temp=(arg-16)>>dock;
		temp=temp&0x01;
				if(init_mode==0)
			{
				if(dock==0)temp=1;
				if(dock==1)temp=0;
				if(dock==2)temp=0;
				if(dock==3)temp=1;
			}
code(temp);
	
	}
}
void code_hex(uint8_t arg)
{
	int temp1;
	for(int dock=0;dock<4;dock++)
	{
		temp1=arg>>(3-dock);
		temp1=temp1&0x01;
code(temp1);
	}
}


void code0()
{
	__HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_1, 8);
user_delaynus_tim(560);
		__HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_1, 0);
	user_delaynus_tim(560);
TIM_ForcedOC1Config(TIM3, TIM_ForcedAction_InActive);//??????,?TIM3_CH1?????,
}
void code1()
{
	__HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_1, 8);
user_delaynus_tim(560);
		__HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_1, 0);
user_delaynus_tim(1680);
		
}
void code_0()
{
	__HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_1, 8);
user_delaynus_tim(560);
		__HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_1, 0);
	user_delaynus_tim(560);

}
void code_1()
{
	__HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_1, 8);
user_delaynus_tim(560);
		__HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_1, 0);
user_delaynus_tim(1680);
		
}
void code0times(int arg)
{
	for(int i=0;i<arg;i++)
	{
	code0();
	}
}
void code1times(int arg)
{
	for(int i=0;i<arg;i++)
	{
	code1();
	}
}








void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if(GPIO_Pin==GPIO_PIN_0)//PA0 模式
	{
		switch(init_mode){
			case 2:
				init_mode=4;
			break;
			case 4:
				init_mode=12;
			break;
			case 12:
				init_mode=8;
			break;
			case 8:
				init_mode=0;
			break;
			case 0:
				init_mode=2;
			break;
			default:
				break;
		}
				HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_13);

	}
		if(GPIO_Pin==GPIO_PIN_1)//PA1  风速
	{
		switch(wind){
			case 0:
				wind=2;
			break;
			case 1:
				wind=3;
			break;
			case 2:
				wind=1;
			break;
			case 3:
				wind=0;
			break;
			default:
				wind=0;
				break;
		}
				HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_13);

	}
		if(GPIO_Pin==GPIO_PIN_2)//PA2  +
	{
	
		temperature++;
		if(temperature>29)
		{
			temperature=30;
		}
			on=1;
		sprintf(oled_temp,"%d",temperature);
						HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_13);

	}
		if(GPIO_Pin==GPIO_PIN_3)//PA3  -
	{
		
			temperature--;
		if(temperature<16)
		{
			temperature=16;
		}
on=1;
	sprintf(oled_temp,"%d",temperature);
						HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_13);

	}
		if(GPIO_Pin==GPIO_PIN_4)//PA4  开关
	{
		on_off=(on_off==0)?1:0;
		on=1;
				HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_13);

	}
		UI_printf("%d%d%d%d",temperature,(int)init_mode/2,wind,(int)on_off);
	
	
		sprintf(oled_temp,"%d",temperature);
		OLED_ShowStr(50, 0, oled_temp, 2);
		OLED_ShowStr(50, 2, "   ", 2);
	OLED_ShowStr(50, 4, "         ", 2);
		OLED_ShowStr(50, 6, "    ", 2);
	if(on_off==1)OLED_ShowStr(50, 2, "ON", 2);
		if(on_off==0)OLED_ShowStr(50, 2, "OFF", 2);
if(init_mode==0)OLED_ShowStr(50, 4, "Auto", 2);
		if(init_mode==2)OLED_ShowStr(50, 4, "Warm", 2);
	if(init_mode==4)OLED_ShowStr(50, 4, "Dehumidify", 2);
		if(init_mode==12)OLED_ShowStr(50, 4, "Blowing-in", 2);
			if(init_mode==8)OLED_ShowStr(50, 4, "Cold", 2);
		
	sprintf(oled_wind,"%d",wind);
	if(wind==0)
	{
			OLED_ShowStr(50, 6, "Auto", 2);
	}
	if(wind==1)
	{
			OLED_ShowStr(50, 6, "2", 2);
	}
	if(wind==2)
	{
			OLED_ShowStr(50, 6, "1", 2);
	}
	if(wind==3)
	{
			OLED_ShowStr(50, 6, "3", 2);
	}
	sprintf((char *)TEXT_Buffer,"%d-%d",temperature,init_mode);
FlashWriteBuff( DEVICE_INFO_ADDRESS, TEXT_Buffer,sizeof(TEXT_Buffer) );
	on=1;

}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(uart_mode==1)
	{
		init_mode=2*(aRxBuffer[0]-48);
		uart_mode=0;
		no_temp=1;
	}
	if(uart_wind==1)
	{
		wind_for_bt=aRxBuffer[0]-48;
wind=wind_for_bt;
		if(wind_for_bt==1)wind=2;
		if(wind_for_bt==2)wind=1;
		
		uart_wind=0;
		no_temp=1;
	}

	if(aRxBuffer[0]=='!')
	{
uart_mode=1;
		no_temp=1;
	}	
if(aRxBuffer[0]=='w')
	{
    uart_wind=1;
		no_temp=1;
	}	 	
	if(aRxBuffer[0]=='k')
	{
on_off=1;
		no_temp=1;
	}	 
	if(aRxBuffer[0]=='g')
	{
on_off=0;
		no_temp=1;
		
	}	 
	if(aRxBuffer[0]=='-')
	{
uart_temp=2;
		
	}	 
	
	if((uart_temp+no_temp)==0)
	{
temperature=(int)aRxBuffer[0]-25;			
		if(temperature>29)
		{
			temperature=30;
		}
	}
		no_temp=0;
		if(uart_temp==1)
	{

		temperature=71-(int)aRxBuffer[0];
		if(temperature<16)
		{
			temperature=16;
		}
		uart_temp=0;
		
	}
	if(uart_temp==2)
		uart_temp--;
//UI_printf("  %d",uart_time++);	  
	UI_printf("%d%d%d%d",temperature,(int)init_mode/2,wind_for_bt,(int)on_off);
	sprintf(oled_temp,"%d",temperature);
		OLED_ShowStr(50, 0, oled_temp, 2);
	OLED_ShowStr(50, 4, "        ", 2);
		OLED_ShowStr(50, 6, "     ", 2);
	OLED_ShowStr(50, 2, "     ", 2);
	if(on_off==1)OLED_ShowStr(50, 2, "ON", 2);
		if(on_off==0)OLED_ShowStr(50, 2, "OFF", 2);
	OLED_ShowStr(50, 4, "           ", 2);
if(init_mode==0)OLED_ShowStr(50, 4, "Auto", 2);
		if(init_mode==2)OLED_ShowStr(50, 4, "Warm", 2);
	if(init_mode==4)OLED_ShowStr(50, 4, "Dehumidify", 2);
		if(init_mode==12)OLED_ShowStr(50, 4, "Blowing-in", 2);
			if(init_mode==8)OLED_ShowStr(50, 4, "Cold", 2);
		
	sprintf(oled_wind,"%d",wind);
	if(wind==0)
	{
			OLED_ShowStr(50, 6, "Auto", 2);
	}
	if(wind==1)
	{
			OLED_ShowStr(50, 6, "2", 2);
	}
	if(wind==2)
	{
			OLED_ShowStr(50, 6, "1", 2);
	}
	if(wind==3)
	{
			OLED_ShowStr(50, 6, "3", 2);
	}

		sprintf((char *)TEXT_Buffer,"%d-%d",temperature,init_mode);
FlashWriteBuff( DEVICE_INFO_ADDRESS, TEXT_Buffer,sizeof(TEXT_Buffer) );  
on=1;
		HAL_UART_Receive_IT(&huart3, (uint8_t *)aRxBuffer, 1);
	
}

void ok_on(uint8_t mode,uint8_t kaiguan,uint8_t wendu);
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_USART3_UART_Init();
  /* USER CODE BEGIN 2 */
//__HAL_TIM_ENABLE_IT(&htim3, TIM_CHANNEL_1);
HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);


HAL_Delay(100);
			sprintf((char *)TEXT_Buffer,"%d",temperature);
		STMFLASH_Write(FLASH_SAVE_ADDR,(uint16_t *)TEXT_Buffer,SIZE);
HAL_Delay(100);


//		STMFLASH_Write(FLASH_SAVE_ADDR_Mode,(uint16_t *)TEXT_Buffer,SIZE);
	OLED_Init();
	OLED_CLS();
		HAL_Delay(100);
	/* USER CODE BEGIN 0 */
uint8_t FlashWBuff [255];
uint8_t FlashRBuff [255];
/* USER CODE END 0 */

/* USER CODE BEGIN 1 */
	uint8_t i;
	uint8_t FlashTest[] = "Test DEMO";
  /* USER CODE END 1 */
  /* USER CODE BEGIN SysInit */
//			sprintf((char *)TEXT_Buffer,"%d",init_mode);
//FlashWriteBuff( DEVICE_INFO_ADDRESS, TEXT_Buffer,sizeof(TEXT_Buffer) );        // ?????Flash
//	
//	for(i=0;i<255;i++)
//		FlashWBuff[i] = i;
//	
//  FlashWriteBuff( DEVICE_INFO_ADDRESS + sizeof(FlashTest), FlashWBuff,255 );  // ?????Flash
	FlashReadBuff(  DEVICE_INFO_ADDRESS,FlashRBuff,sizeof(FlashTest)  );  // ?Flash????
 	sprintf(oled_temp,"%d",temperature);
	  OLED_ShowStr(50, 0, oled_temp, 2);
		char *flash_mode=strchr(FlashRBuff,'-')+1;
		init_mode = atoi(flash_mode);
 //   OLED_ShowStr(90, 0, flash_mode, 1);
 /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  /* USER CODE BEGIN 2 */
	






if(init_mode==0)OLED_ShowStr(50, 4, "Auto", 2);
		if(init_mode==2)OLED_ShowStr(50, 4, "Warm", 2);
	if(init_mode==4)OLED_ShowStr(50, 4, "Dehumidify", 2);
		if(init_mode==12)OLED_ShowStr(50, 4, "Blowing-in", 2);
			if(init_mode==8)OLED_ShowStr(50, 4, "Cold", 2);
			HAL_Delay(100);

	sprintf(oled_wind,"%d",wind);
	if(wind==0)
	{
			OLED_ShowStr(50, 6, "Auto", 2);
	}
	else{
	OLED_ShowStr(50, 6, oled_wind, 2);
	}
  OLED_ShowStr(1, 0, "Temp:", 2);
	OLED_ShowStr(1, 2, "Now:", 2);

	OLED_ShowStr(1, 4, "Mode:", 2);
	OLED_ShowStr(1, 6, "Wind:", 2);
	
	HAL_UART_Receive_IT(&huart3, (uint8_t *)aRxBuffer, 1);
//__HAL_TIM_SetCompare(&htim2, TIM_CHANNEL_1, 0);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	int temper;
	init_mode=0;
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
						HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_13);

if(on==1)
{
		on=0;
turn_on();
//HAL_Delay(1000);
//	
}
//turn_on();		
HAL_Delay(500);
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void turn_on()
{
	
	
__HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_1,8);
user_delaynus_tim(9000);
__HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_1, 0);
user_delaynus_tim(4500);


   
code_hex(init_mode+on_off);
		
	code_hex(4*wind);
		
//code0();
//code0();
//code0();
//code0();
//		
code_temp(temperature);

code0();
code0();
code0();
code0();

code0();
code0();
code0();
code0();

code0();//0x00

//		
code1();
code0();
code0();

code0();
code0();
code0();
code0();

code1();
code0();
code1();

code0();


code0();
code1();
code0();


code1();//jieshu

HAL_Delay(17);
	code0();		
		code0();		
		code0();		
		code0();		
		code0();		
		code0();		
		code0();		
		code0();		
		code0();		
		code0();		
		code0();		
		code0();		
		code0();		
		code0();		
		code0();		
		code0();		
		code0();		
		code0();		
		code0();		
		code0();		
		code0();		
		code0();		
		code0();		
		code0();		
		code0();		
		code0();		
		code0();		
		code0();		
code_temp_rev(temperature);

code1();//结束码

HAL_Delay(34);
__HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_1,8);
user_delaynus_tim(9000);
__HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_1, 0);
user_delaynus_tim(4500);


code_hex(init_mode+on_off);

	code_hex(4*wind);

//code0();
//code0();
//code0();
//code0();
		
code_temp(temperature);


code0();
code0();
code0();
code0();

code0();
code0();
code0();
code0();

code0();//0x00

//		
code1();
code0();
code0();

code0();
code0();
code0();

code0();
code1();


code1();
code1();

code0();


code0();

code1();
code0();


code1();//jieshu

HAL_Delay(17);
			code0();		
		code0();		
		code0();		
		code0();		
		code0();		
		code0();		
		code0();		
		code0();		
		code0();		
		code0();		
		code0();		
		code0();		
		code0();		
		code0();		
		code0();		
		code0();		
		code0();		
		code0();		
		code0();		
		code0();		
		code0();		
		code0();		
		code0();		
		code0();		
		code0();		
		code0();		
		code0();		
		code0();		
code_temp_rev(temperature);
code1();//结束码
	HAL_Delay(1000);
}




/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
