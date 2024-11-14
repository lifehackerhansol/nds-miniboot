#include <stdbool.h>
#include "../../../fatfs/source/ff.h"			/* Obtains integer types */
#include "dldi.h"
#include "../../../fatfs/source/diskio.h"		/* Declarations of disk functions */
#include "scdssdhc.h"

static DSTATUS status = STA_NOINIT;

DSTATUS disk_status(BYTE pdrv) {
	return status;
}

DSTATUS disk_initialize(BYTE pdrv) {
	if (!SCDSSDHC_SDInitialize())
		status = STA_NOINIT;
	//else if (!_io_dldi_stub.isInserted())
	//	status = STA_NODISK;
	else
		status = 0;

	return status;
}

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	LBA_t sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
) {
	//if (!_io_dldi_stub.readSectors(sector, count, buff))
	//	return RES_ERROR;
	SCDSSDHC_SDReadMultiSector(sector, buff, count);
	return RES_OK;
}

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
) {
	if (cmd == CTRL_SYNC)
		return RES_OK;

	return RES_PARERR;
}

