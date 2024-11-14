@
@ Copyright (C) 2024 lifehackerhansol
@
@ SPDX-License-Identifier: zlib
@

    .syntax unified
    .arm

@ void waitByLoop(uint32_t count)
@ because we can't use swi 0x3 in DLDI

    .global waitByLoop
waitByLoop:
    subs r0, r0, #1
    bgt waitByLoop
    bx lr
