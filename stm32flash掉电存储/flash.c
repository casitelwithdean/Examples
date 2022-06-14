/*
 * flash.c
 *
 * Created: 2018-01-29
 * Author: zhanglifu
 */
 
/*********************************************************************/
//                        ͷ�ļ�
/*********************************************************************/
#include "flash.h"


// ������д��
// WriteAddr:��ʼ��ַ
// pBuffer:  ����ָ��
// NumToWrite:�ֽ�����
void FlashWriteNoCheck( uint32_t WriteAddr,uint8_t *pBuffer,uint16_t NumToWrite )
{
    uint16_t i;

    for( i=0; i<NumToWrite; i+=4 )
    {
        while( HAL_OK != HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, WriteAddr+i,*(uint32_t *)(pBuffer+i) ) );
    }

}

extern void FLASH_PageErase(uint32_t PageAddress);
void FlashWriteBuff( const uint32_t destination_address, uint8_t *const buffer,uint32_t length )
{
    uint16_t i;
    uint8_t FlashBuff[FMC_SECTOR_SIZE];
    uint32_t StartAddress = destination_address - destination_address%FMC_SECTOR_SIZE;
    uint16_t Offset = destination_address - StartAddress;
    uint8_t *pBuf = buffer;
    uint32_t remaindNum = length;

    HAL_StatusTypeDef status = HAL_ERROR;

    // ��ַ���
    if( (destination_address < FMC_FLASH_BASE) || ( destination_address + length >= FMC_FLASH_END) || (length <= 0) )
        return;

    HAL_FLASH_Unlock();	// ����
    do
    {
        // ����һҳ����
        for(i=0; i < FMC_SECTOR_SIZE; i += 4 )
            *(uint32_t *)(FlashBuff+i) = *(uint32_t *)(StartAddress+i);

        // �޸�Ҫ���������
        for ( i=0; (i+Offset)<FMC_SECTOR_SIZE && i< remaindNum; i++ )
            *(FlashBuff+Offset+i) = *(pBuf+i);


        // ����һROW����
        FLASH_PageErase( StartAddress );

        // HAL�� FLASH_PageErase��BUFF,Ҫ�����������д���
        while( status != HAL_OK )
            status = FLASH_WaitForLastOperation(FLASH_TIMEOUT_VALUE);
        CLEAR_BIT(FLASH->CR, FLASH_CR_PER);

        // д������
        FlashWriteNoCheck( StartAddress,FlashBuff,FMC_SECTOR_SIZE );

        // Ϊ��һҳ��׼��
        StartAddress +=  FMC_SECTOR_SIZE;
        remaindNum -= i;
        pBuf += i;
        Offset = 0;

    } while( remaindNum > 0 );

    HAL_FLASH_Lock();  // ����
		
}



// ��FLASH�ж�ָ����������
void FlashReadBuff(const uint32_t source_address,uint8_t *const buffer,uint16_t length)
{
    uint16_t i;
    uint8_t Offset = 0;
    uint32_t StartAddress = source_address;
    uint16_t data;

    // ��ַ���
    if( source_address + length > FMC_FLASH_END ) return;

    // ���û�ж�16��
    if( source_address & 1 )
    {
        Offset = 1;
        StartAddress = source_address-1;
    }

    // flash�Ĳ���Ҫ��16���� ��С��д����16������
    if ( Offset )
    {
        data = *(uint16_t *)(StartAddress);
        buffer[0] = data >> 8;
        StartAddress += 2;
    }

    for ( i = 0; i < (length-Offset); i += 2)
    {
        data = *(uint16_t *)(StartAddress+i);
        buffer[i+Offset] = (data & 0xFF);
        if ( (i+Offset) < (length - 1) )
            buffer[i + Offset + 1] = (data >> 8);
    }

}



