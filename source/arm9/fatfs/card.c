// clang-format off

/*
    This file is a part of libtwl (card.c)

    Copyright (C) 2023 Gericom

    SPDX-License-Identifier: Zlib
*/

#include <stdint.h>
#include "card.h"

void card_romCpuRead(uint32_t* dst, uint32_t words)
{
    uint32_t* target = dst + words;
    do
    {
        // Read data if available
        if (card_romIsDataReady())
        {
            uint32_t data = card_romGetData();
            if (dst < target)
                *dst++ = data;
        }
    } while (card_romIsBusy());
}

void card_romCpuReadUnaligned(uint8_t* dst, uint32_t words)
{
    uint8_t* target = dst + (words << 2);
    do
    {
        // Read data if available
        if (card_romIsDataReady())
        {
            uint32_t data = card_romGetData();
            if (dst < target)
            {
                *dst++ = data & 0xFF;
                *dst++ = (data >> 8) & 0xFF;
                *dst++ = (data >> 16) & 0xFF;
                *dst++ = (data >> 24) & 0xFF;
            }
        }
    } while (card_romIsBusy());
}

void card_romCpuWrite(const uint32_t* src, uint32_t words)
{
    uint32_t data = 0;
    const uint32_t* target = src + words;
    do
    {
        // Write data if ready
        if (card_romIsDataReady())
        {
            if (src < target)
                data = *src++;
            card_romSetData(data);
        }
    } while (card_romIsBusy());
}

void card_romCpuWriteUnaligned(const uint8_t* src, uint32_t words)
{
    uint32_t data = 0;
    const uint8_t* target = src + (words << 2);
    do
    {
        // Write data if ready
        if (card_romIsDataReady())
        {
            if (src < target)
            {
                data = src[0] | (src[1] << 8) | (src[2] << 16) | (src[3] << 24);
                src += 4;
            }
            card_romSetData(data);
        }
    } while (card_romIsBusy());
}
