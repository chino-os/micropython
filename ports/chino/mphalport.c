/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2022-2023 Damien P. George
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#define WIN32_LEAN_AND_MEAN
#include "py/mphal.h"
#include "py/mpthread.h"
#include "shared/readline/readline.h"
#include <Windows.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>

mp_uint_t mp_hal_ticks_us(void) { return GetTickCount() * 1000; }

mp_uint_t mp_hal_ticks_cpu(void) {
    LARGE_INTEGER value;
    QueryPerformanceCounter(&value);
#ifdef _WIN64
    return value.QuadPart;
#else
    return value.LowPart;
#endif
}

uint64_t mp_hal_time_ns(void) { return (uint64_t)mp_hal_ticks_us() * 1000ULL; }

void mp_hal_delay_ms(mp_uint_t ms) {
#if MICROPY_ENABLE_SCHEDULER
    mp_uint_t start = mp_hal_ticks_ms();
    while (mp_hal_ticks_ms() - start < ms) {
        SleepEx(1, TRUE);
    }
#else
    msec_sleep((double)ms);
#endif
}

void mp_hal_delay_us(mp_uint_t us) { SleepEx(0, TRUE); }

int mp_hal_stdin_rx_chr(void) {
    MP_THREAD_GIL_EXIT();
    int c = getchar();
    MP_THREAD_GIL_ENTER();
    return c;
}

mp_uint_t mp_hal_stdout_tx_strn(const char *str, size_t len) {
    MP_THREAD_GIL_EXIT();
    printf("%.*s", (int)len, str);
    MP_THREAD_GIL_ENTER();
    return 0;
}

uintptr_t mp_hal_stdio_poll(uintptr_t poll_flags) { return 0; }
