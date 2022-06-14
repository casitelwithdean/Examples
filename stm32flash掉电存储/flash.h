/*
 * flash.h
 *
 * Created: 2016-04-22
 * Author: zhanglifu
 */
#ifndef _flash_H_
#define _flash_H_


#include "stm32f1xx_hal.h"


/*********************************************************************/
//                        ��������
/*********************************************************************/
//-- ��;����
// 0x0800FC00-0x0800FFFF -- ʹ�����4k ���ֽ�������ŵ����Ϣ
#define FMC_FLASH_BASE      0x08000000 	// FLASH����ʼ��ַ
#define APP_MAX_SIZE        0x00010000  // 


#define FMC_FLASH_END        0x08010000  // FLASH�Ľ�����ַ 256
#define DEVICE_INFO_ADDRESS  0x0800C000  //��STM32_FLASH_END - DEVICE_INFO_SIZE��   // �豸��Ϣ��ʼ��ַ
#define DEVICE_LOG_ADDRESS   0x0800E000  //��STM32_FLASH_END - 2*DEVICE_INFO_SIZE�� // �豸��־��ʼ��ַ



#define FMC_FLASH_SIZE 64			          // ����Flash��С����λKB


#if FMC_FLASH_SIZE < 256
#define FMC_SECTOR_SIZE 1024            // �ֽ�
#define MOD_SECTOR_SIZE 0X3FF
#define PAGE_COUNT_BY16 512
#else
#define FMC_SECTOR_SIZE	2048
#define MOD_SECTOR_SIZE 0X7FF
#define PAGE_COUNT_BY16 1024
#endif





/*********************************************************************/
//                        ��������
/*********************************************************************/
void FlashWriteBuff( const uint32_t destination_address, uint8_t *const buffer,uint32_t length );
void FlashReadBuff(const uint32_t source_address,uint8_t *const buffer,uint16_t length);




#endif
