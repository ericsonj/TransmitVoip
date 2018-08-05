/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2016        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "diskio.h"		/* FatFs lower layer API */
#include "stm32_adafruit_sd.h"

/* Definitions of physical drive number for each drive */
#define DEV_RAM		0	/* Example: Map Ramdisk to physical drive 0 */
#define DEV_MMC		1	/* Example: Map MMC/SD card to physical drive 1 */
#define DEV_USB		2	/* Example: Map USB MSD to physical drive 2 */

static volatile DSTATUS Stat = STA_NOINIT;

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status(BYTE pdrv /* Physical drive nmuber to identify the drive */
) {

	if (pdrv) {
		return STA_NOINIT;
	}

	if (Stat & STA_NODISK) {
		return Stat;
	}
	uint8_t resp = BSP_SD_GetCardState();

	if (resp != BSP_SD_OK) {
		Stat = STA_NOINIT;
	}

	return Stat;
}

/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize(BYTE pdrv /* Physical drive nmuber to identify the drive */
) {

	if (pdrv) {
		return STA_NOINIT;
	}

	if (Stat & STA_NODISK) {
		return Stat;
	}

	uint8_t resp = BSP_SD_Init();

	if (resp == BSP_SD_OK) {
		Stat &= ~STA_NOINIT;
	}

	return Stat;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read(BYTE pdrv, /* Physical drive nmuber to identify the drive */
BYTE *buff, /* Data buffer to store read data */
DWORD sector, /* Start sector in LBA */
UINT count /* Number of sectors to read */
) {

	if (pdrv || !count) {
		return RES_PARERR;
	}

	if (Stat & STA_NOINIT) {
		return RES_NOTRDY;
	}

	uint8_t resp = BSP_SD_ReadBlocks(buff, sector, 1, 100);
	if (resp == BSP_SD_OK) {
		return RES_OK;
	}

	return RES_ERROR;
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

DRESULT disk_write(BYTE pdrv, /* Physical drive nmuber to identify the drive */
const BYTE *buff, /* Data to be written */
DWORD sector, /* Start sector in LBA */
UINT count /* Number of sectors to write */
) {

	if (pdrv || !count) {
		return RES_PARERR;
	}
	if (Stat & STA_NOINIT) {
		return RES_NOTRDY;
	}
	if (Stat & STA_PROTECT) {
		return RES_WRPRT;
	}

	uint8_t resp = BSP_SD_WriteBlocks(buff, sector, 1, 100);
	if (resp == BSP_SD_OK) {
		return RES_OK;
	}

	return RES_ERROR;
}

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl(BYTE pdrv, /* Physical drive nmuber (0..) */
BYTE cmd, /* Control code */
void *buff /* Buffer to send/receive control data */
) {
	if (pdrv) {
		return STA_NOINIT;
	}

	return RES_OK;
}

