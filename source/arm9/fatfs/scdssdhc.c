/*
    SuperCard DSONE
    Card IO routines

    Copyright (C) 2023 lifehackerhansol

    SPDX-License-Identifier: Zlib
*/

#include "libtwl_ext.h"
#include "card.h"
#include <stdint.h>
#include <stdbool.h>

#include "scdssdhc.h"

static uint32_t isSDHC = 0;

static uint32_t SCDSSDHC_SDHostSetMode(uint8_t sdio, uint32_t parameter, uint8_t response_type, uint32_t latency) {
    uint64_t command = ((uint64_t)SCDSSDHC_CMD_SD_HOST_PARAM << 56) | ((uint64_t)parameter << 24) |
                  ((uint64_t)sdio << 16) | ((uint64_t)response_type << 8);
	return cardExt_ReadData4Byte(command, SCDSSDHC_CTRL_READ_4B | MCCNT1_LATENCY1(latency));
}

static uint32_t SCDSSDHC_IsSDHostBusy(void) {
    REG_SCDSSDHC_MCCMD[0] = SCDSSDHC_CMD_SD_HOST_BUSY;
    card_romStartXfer(SCDSSDHC_CTRL_READ_4B, false);
    card_romWaitDataReady();
    return card_romGetData();
}

static uint32_t SCDSSDHC_SDHostGetResponse(void) {
    REG_SCDSSDHC_MCCMD[0] = SCDSSDHC_CMD_SD_HOST_RESPONSE;
    card_romStartXfer(SCDSSDHC_CTRL_READ_4B, false);
    card_romWaitDataReady();
    return __builtin_bswap32(card_romGetData());
}

static void SCDSSDHC_SDSendR0Command(uint8_t sdio, uint32_t parameter, uint32_t latency) {
    SCDSSDHC_SDHostSetMode(sdio, parameter, SCDSSDHC_SD_HOST_NORESPONSE, latency);
    while (SCDSSDHC_IsSDHostBusy())
        ;
}

static uint32_t SCDSSDHC_SDSendR1Command(uint8_t sdio, uint32_t parameter, uint32_t latency) {
    SCDSSDHC_SDHostSetMode(sdio, parameter, SCDSSDHC_SD_HOST_READ_4B, latency);
    while (SCDSSDHC_IsSDHostBusy())
        ;
    return SCDSSDHC_SDHostGetResponse();
}

// TODO: save the response to a buffer (also figure out in which order they're sent)
static void SCDSSDHC_SDSendR2Command(uint8_t sdio, uint32_t parameter, uint32_t latency) {
    SCDSSDHC_SDHostSetMode(sdio, parameter, SCDSSDHC_SD_HOST_READ_4B_MULTI, latency);
    while (SCDSSDHC_IsSDHostBusy())
        ;

    // TODO: parse this response
    SCDSSDHC_SDHostGetResponse();

    for (int i = 0; i < 4; i++) {
        SCDSSDHC_SDHostSetMode(sdio, parameter, SCDSSDHC_SD_HOST_NEXT_4B, latency);
        while (SCDSSDHC_IsSDHostBusy())
            ;
        // TODO: parse this response
        SCDSSDHC_SDHostGetResponse();
    }
    SCDSSDHC_SDHostSetMode(sdio, parameter, SCDSSDHC_SD_HOST_SEND_STOP_CLK, 0);
    while (SCDSSDHC_IsSDHostBusy())
        ;
}

void waitByLoop(uint32_t count);

static void SCDSSDHC_SDHostSetRegister(uint8_t bits) {
    uint64_t command = ((uint64_t)SCDSSDHC_CMD_SD_HOST_SET_REGISTER << 56) | ((uint64_t)(0x30 | bits) << 48);
	cardExt_ReadData4Byte(command, SCDSSDHC_CTRL_READ_4B);
    waitByLoop(0x300);
}

static uint32_t SCDSSDHC_IsSDFIFOBusy(void) {
    REG_SCDSSDHC_MCCMD[0] = SCDSSDHC_CMD_FIFO_BUSY;
    card_romStartXfer(SCDSSDHC_CTRL_READ_4B, false);
    card_romWaitDataReady();
    return card_romGetData();
}

// Reads max 512 bytes
static void SCDSSDHC_SDFIFOReadData(void* buffer, uint32_t length) {
    REG_SCDSSDHC_MCCMD[0] = SCDSSDHC_CMD_FIFO_READ_DATA;
    card_romStartXfer(SCDSSDHC_CTRL_READ_512B, false);
    if ((uint32_t)buffer & 3)
        card_romCpuReadUnaligned((uint8_t*)buffer, length);
    else
        card_romCpuRead(buffer, length);
}

// Writes max 512 bytes
static void SCDSSDHC_SDFIFOWriteData(const void* buffer, uint32_t length) {
    REG_SCDSSDHC_MCCMD[0] = SCDSSDHC_CMD_FIFO_WRITE_DATA;
    card_romStartXfer(SCDSSDHC_CTRL_WRITE_512B, false);
    if ((uint32_t)buffer & 3)
        card_romCpuWriteUnaligned((uint8_t*)buffer, length);
    else
        card_romCpuWrite(buffer, length);
}

static uint32_t SCDSSDHC_SRAMReadData(uint32_t address) {
    uint64_t command = ((uint64_t)SCDSSDHC_CMD_SRAM_READ_DATA << 56) | ((uint64_t)address << 48);
	return cardExt_ReadData4Byte(command, SCDSSDHC_CTRL_READ_4B);
}

void SCDSSDHC_SDGetSDHCStatusFromSRAM(void) {
    isSDHC = SCDSSDHC_SRAMReadData(0x7F9E0) != 0 ? 1 : 0;
}

bool SCDSSDHC_SDInitialize(void) {
    uint32_t isSD20 = 0;
    uint32_t response = 0;

    isSDHC = 0;

    // TODO: What is this command doing?
	cardExt_ReadData4Byte(0x6600000000000000ull, 0xA7586000);

    // Reset SD host
    SCDSSDHC_SDHostSetRegister(0);

    // Init
    SCDSSDHC_SDHostSetRegister(SCDSSDHC_SD_HOST_REG_RESET | SCDSSDHC_SD_HOST_REG_400KHZ_CLK |
                               SCDSSDHC_SD_HOST_REG_CLEAN_ROM_MODE);
    SCDSSDHC_SDHostSetMode(0, 0, SCDSSDHC_SD_HOST_SEND_CLK, SCDSSDHC_CTRL_SD_LOW_CLK_LATENCY);
    waitByLoop(0x1000);

    // CMD0
    SCDSSDHC_SDSendR0Command(0, 0, SCDSSDHC_CTRL_SD_LOW_CLK_LATENCY);
    SCDSSDHC_SDHostSetMode(0, 0, SCDSSDHC_SD_HOST_SEND_STOP_CLK, SCDSSDHC_CTRL_SD_LOW_CLK_LATENCY);

    // CMD8
    SCDSSDHC_SDHostSetMode(8, 0x1AA, SCDSSDHC_SD_HOST_READ_4B, SCDSSDHC_CTRL_SD_LOW_CLK_LATENCY);

    uint32_t retryCount = 9999;
    while (1) {
        if (!SCDSSDHC_IsSDHostBusy()) {
            response = SCDSSDHC_SDHostGetResponse();
            break;
        }
        if (--retryCount == 0) {
            SCDSSDHC_SDHostSetRegister(0);
            SCDSSDHC_SDHostSetRegister(SCDSSDHC_SD_HOST_REG_RESET |
                                       SCDSSDHC_SD_HOST_REG_400KHZ_CLK |
                                       SCDSSDHC_SD_HOST_REG_CLEAN_ROM_MODE);
            response = 0;
            break;
        }
    }

    if (response == 0x1AA) isSD20 = 1;

    do {
        // CMD55
        SCDSSDHC_SDHostSetMode(55, 0, SCDSSDHC_SD_HOST_READ_4B, SCDSSDHC_CTRL_SD_LOW_CLK_LATENCY);
        retryCount = 9999;
        while (SCDSSDHC_IsSDHostBusy()) {
            if (--retryCount == 0) {
                SCDSSDHC_SDHostSetRegister(0);
                SCDSSDHC_SDHostSetRegister(SCDSSDHC_SD_HOST_REG_RESET |
                                           SCDSSDHC_SD_HOST_REG_400KHZ_CLK |
                                           SCDSSDHC_SD_HOST_REG_CLEAN_ROM_MODE);
                return false;
            }
        }
        SCDSSDHC_SDHostGetResponse();

        // ACMD41
        uint32_t parameter = 0x00FC0000;
        if (isSD20) parameter |= BIT(30);
        response = SCDSSDHC_SDSendR1Command(41, parameter, SCDSSDHC_CTRL_SD_LOW_CLK_LATENCY);
    } while (!(response & BIT(31)));

    isSDHC = response & BIT(30) ? 1 : 0;

    // CMD2
    SCDSSDHC_SDSendR2Command(2, 0, SCDSSDHC_CTRL_SD_LOW_CLK_LATENCY);

    // CMD3
    response = SCDSSDHC_SDSendR1Command(3, 0, SCDSSDHC_CTRL_SD_LOW_CLK_LATENCY);
    uint32_t sdio_rca = response & 0xFFFF0000;

    // CMD7
    SCDSSDHC_SDSendR1Command(7, sdio_rca, SCDSSDHC_CTRL_SD_LOW_CLK_LATENCY);

    // ACMD6
    SCDSSDHC_SDSendR1Command(55, sdio_rca, SCDSSDHC_CTRL_SD_LOW_CLK_LATENCY);
    SCDSSDHC_SDSendR1Command(6, 2, SCDSSDHC_CTRL_SD_LOW_CLK_LATENCY);

    // CMD16
    SCDSSDHC_SDSendR1Command(16, 512, SCDSSDHC_CTRL_SD_LOW_CLK_LATENCY);

    SCDSSDHC_SDHostSetRegister(0);
    SCDSSDHC_SDHostSetRegister(SCDSSDHC_SD_HOST_REG_RESET | SCDSSDHC_SD_HOST_REG_CLEAN_ROM_MODE);
    if (isSDHC)
        SCDSSDHC_SDHostSetRegister(SCDSSDHC_SD_HOST_REG_RESET |
                                   SCDSSDHC_SD_HOST_REG_CLEAN_ROM_MODE | SCDSSDHC_SD_HOST_REG_SDHC);

    *(volatile uint32_t *)0x02FFFC24 = isSDHC == 0 ? 0 : ~0;

    return true;
}

void SCDSSDHC_SDReadSingleSector(uint32_t sector, void* buffer) {
    uint64_t command = ((uint64_t)SCDSSDHC_CMD_SD_READ_SINGLE_BLOCK << 56) |
                  ((uint64_t)(isSDHC ? sector : sector << 9) << 24);
	cardExt_ReadData4Byte(command, SCDSSDHC_CTRL_READ_4B);

    // wait until data ready
    while (SCDSSDHC_IsSDFIFOBusy())
        ;

    // retrieve data
    SCDSSDHC_SDFIFOReadData(buffer, 128);
}

void SCDSSDHC_SDReadMultiSector(uint32_t sector, void* buffer, uint32_t num_sectors) {
    uint64_t command = ((uint64_t)SCDSSDHC_CMD_SD_READ_MULTI_BLOCK << 56) |
                  ((uint64_t)(isSDHC ? sector : sector << 9) << 24);
	cardExt_ReadData4Byte(command, SCDSSDHC_CTRL_READ_4B);

    while (1) {
        // wait until data ready
        while (SCDSSDHC_IsSDFIFOBusy())
            ;

        // retrieve data
        SCDSSDHC_SDFIFOReadData(buffer, 128);
        buffer = (uint8_t*)buffer + 0x200;
        num_sectors--;
        if (num_sectors == 0) break;
        SCDSSDHC_SDHostSetMode(0, 0, SCDSSDHC_SD_HOST_NEXT_DATABLOCK, 0);
    };

    // end read
    SCDSSDHC_SDSendR1Command(12, 0, 0);
}

void SCDSSDHC_SDWriteSingleSector(uint32_t sector, const void* buffer) {
    // instruct cart where to write
    SCDSSDHC_SDSendR1Command(24, isSDHC ? sector : sector << 9, 0);

    // write
    SCDSSDHC_SDFIFOWriteData(buffer, 128);
    while (SCDSSDHC_IsSDHostBusy())
        ;

    // end write
    REG_SCDSSDHC_MCCMD[0] = SCDSSDHC_CMD_SD_WRITE_END;
    card_romStartXfer(SCDSSDHC_CTRL_READ_4B, false);
    card_romWaitDataReady();
    card_romGetData();
    while (SCDSSDHC_IsSDHostBusy())
        ;
}

void SCDSSDHC_SDWriteMultiSector(uint32_t sector, const void* buffer, uint32_t num_sectors) {
    // instruct cart where to write
    SCDSSDHC_SDSendR1Command(25, isSDHC ? sector : sector << 9, 0);

    while (num_sectors--) {
        // end write
        // well, it's supposed to be end write. But doing it first is a no-op, and it's also
        // a little simpler to write this way
        REG_SCDSSDHC_MCCMD[0] = SCDSSDHC_CMD_SD_WRITE_END;
        card_romStartXfer(SCDSSDHC_CTRL_READ_4B, false);
        card_romWaitDataReady();
        card_romGetData();
        while (SCDSSDHC_IsSDHostBusy())
            ;
        // write
        SCDSSDHC_SDFIFOWriteData(buffer, 128);
        while (SCDSSDHC_IsSDHostBusy())
            ;
        buffer = (uint8_t*)buffer + 0x200;
    }

    // *really* end write
    SCDSSDHC_SDSendR1Command(12, 0, 0);
    REG_SCDSSDHC_MCCMD[0] = SCDSSDHC_CMD_SD_WRITE_END;
    card_romStartXfer(SCDSSDHC_CTRL_READ_4B, false);
    card_romWaitDataReady();
    card_romGetData();
    while (SCDSSDHC_IsSDHostBusy())
        ;
}
