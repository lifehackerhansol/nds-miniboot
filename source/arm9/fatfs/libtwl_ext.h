/*
    Copyright (C) 2023 lifehackerhansol

    SPDX-License-Identifier: Zlib
*/

#pragma once

#include <stdint.h>

// Wrapper for reading from a cartridge.
void cardExt_ReadData(uint64_t command, uint32_t flags, void* buffer, uint32_t length);

// Wrapper for writing to a cartridge.
void cardExt_WriteData(uint64_t command, uint32_t flags, const void* buffer, uint32_t length);

// Wrapper for reading 4 bytes from a cartridge. Usually used for simple commands.
uint32_t cardExt_ReadData4Byte(uint64_t command, uint32_t flags);

// Wrapper for sending a command without an expected return value.
void cardExt_SendCommand(uint64_t command, uint32_t flags);
