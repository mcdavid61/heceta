/*
 * ModbusSlave.h
 *
 *  Created on: Aug 18, 2020
 *      Author: BFS
 */

#ifndef SPIFLASH_H_
#define SPIFLASH_H_

//	Commands and macros for the Serial Flash itself.
#define Write_Enable_WREN                       (uint8_t)0x06
#define Write_Disable_WRDI                      (uint8_t)0x04
#define Read_Status_Register_RDSR               (uint8_t)0x05
#define Write_Status_Register_WRSR              (uint8_t)0x01
#define Read_Data_Bytes_READ                    (uint8_t)0x03
#define Page_Program_PP                         (uint8_t)0x02

#define SPIFLASH_RDSR_BUSY_BIT 					(1 << 0)

//	Size macros
#define SPIFLASH_CHIP_SIZE				(4096)		//	Total size of serial flash in bytes, CAT25320VI-GT3
#define SPIFLASH_PAGE_SIZE				(32)		//	Page size
#define SPIFLASH_PAGE_COUNT				(SPIFLASH_CHIP_SIZE / SPIFLASH_PAGE_SIZE)	//	Page count
#define SPIFLASH_MAX_STEPS				(8)

//	Macro to construct address within the three address bytes specified.
#define SPIFLASH_CONSTRUCT_ADDRESS(pDest, nPage, nPageOffset)		*((pDest)) = (((nPage) >> 3) & 0x0F); 	\
																	*((pDest)+1) = (((nPage) << 5) & 0xE0) | (nPageOffset & 0x1F)

/*
	Struct:	SPIStep_T
	Description:
		Each SPI operation consists of a set (or array) of steps.
		This is the primary structure that stores these.

		An SPIStep is dynamic in the sense that, they are generated as needed.
*/
typedef struct
{
    void * pTransmitData;
    void * pReceiveData;
    uint32_t nByteCount;
    bool    bSetCSHigh;
} SPIStep_T;

/*
	Typedef:	SPIFlash_State_T
	Description:
		State of the SPIFlash_T.
*/
typedef enum
{
	SPIFLASH_STATE_IDLE,
	SPIFLASH_STATE_PROCESS,
	SPIFLASH_STATE_COUNT,
}	SPIFlash_State_T;

/*
	Structure:	SPIWriteStatus_T
	Description:
		Keeps track of the write in progress, if there is one.
*/
typedef struct
{
	uint8_t * pBuffer;
	uint32_t nPage;
	uint32_t nPageOffset;
	uint32_t nBytesLeft;
} SPIWriteStatus_T;

bool SPIFlash_IsFree(void);
bool SPIFlash_Write(uint8_t * pBuffer, uint16_t nPage, uint16_t nPageOffset, uint16_t nSize);
bool SPIFlash_Read(uint8_t * pBuffer, uint16_t nPage, uint16_t nPageOffset, uint16_t nSize);
void SPIFlash_Process(void);
#endif /* SPIFLASH_H_ */
