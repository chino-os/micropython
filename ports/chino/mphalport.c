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

// Send string of given length to stdout, converting \n to \r\n.
// void mp_hal_stdout_tx_strn_cooked(const char *str, size_t len) {
//    //printf("%.*s", (int)len, str);
//    WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), str, len, NULL, NULL);
//}

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

uint64_t mp_hal_time_ns(void) { return (uint64_t)mp_hal_ticks_us * 1000ULL; }

void msec_sleep(double msec) {
    if (msec < 0.0) {
        msec = 0.0;
    }
    SleepEx((DWORD)msec, TRUE);
}

#ifdef _MSC_VER
int usleep(useconds_t usec) {
    msec_sleep((double)usec / 1000.0);
    return 0;
}
#endif

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

typedef struct item_t {
    WORD vkey;
    const char *seq;
} item_t;

// map virtual key codes to key sequences known by MicroPython's readline implementation
static item_t keyCodeMap[] = {
    {VK_UP, "[A"},   {VK_DOWN, "[B"}, {VK_RIGHT, "[C"},   {VK_LEFT, "[D"},
    {VK_HOME, "[H"}, {VK_END, "[F"},  {VK_DELETE, "[3~"}, {0, ""} // sentinel
};

// likewise, but with Ctrl key down
static item_t ctrlKeyCodeMap[] = {
    {VK_LEFT, "b"}, {VK_RIGHT, "f"}, {VK_DELETE, "d"}, {VK_BACK, "\x7F"}, {0, ""} // sentinel
};

static const char *cur_esc_seq = NULL;

static int esc_seq_process_vk(WORD vk, bool ctrl_key_down) {
    for (item_t *p = (ctrl_key_down ? ctrlKeyCodeMap : keyCodeMap); p->vkey != 0; ++p) {
        if (p->vkey == vk) {
            cur_esc_seq = p->seq;
            return 27; // ESC, start of escape sequence
        }
    }
    return 0; // nothing found
}

static int esc_seq_chr() {
    if (cur_esc_seq) {
        const char c = *cur_esc_seq++;
        if (c) {
            return c;
        }
        cur_esc_seq = NULL;
    }
    return 0;
}

int mp_hal_stdin_rx_chr(void) {
    // currently processing escape seq?
    const int ret = esc_seq_chr();
    if (ret) {
        return ret;
    }

    // poll until key which we handle is pressed
    BOOL status;
    DWORD num_read;
    INPUT_RECORD rec;
    for (;;) {
        MP_THREAD_GIL_EXIT();
        status = ReadConsoleInputA(GetStdHandle(STD_INPUT_HANDLE), &rec, 1, &num_read);
        MP_THREAD_GIL_ENTER();
        if (!status || !num_read) {
            return CHAR_CTRL_C; // EOF, ctrl-D
        }
        if (rec.EventType != KEY_EVENT || !rec.Event.KeyEvent.bKeyDown) { // only want key down events
            continue;
        }
        const bool ctrl_key_down = (rec.Event.KeyEvent.dwControlKeyState & LEFT_CTRL_PRESSED) ||
                                   (rec.Event.KeyEvent.dwControlKeyState & RIGHT_CTRL_PRESSED);
        const int ret = esc_seq_process_vk(rec.Event.KeyEvent.wVirtualKeyCode, ctrl_key_down);
        if (ret) {
            return ret;
        }
        const char c = rec.Event.KeyEvent.uChar.AsciiChar;
        if (c) { // plain ascii char, return it
            return c;
        }
    }
}

mp_uint_t mp_hal_stdout_tx_strn(const char *str, size_t len) {
    MP_THREAD_GIL_EXIT();
    WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), str, len, NULL, NULL);
    MP_THREAD_GIL_ENTER();
    return 0;
}

uintptr_t mp_hal_stdio_poll(uintptr_t poll_flags) { return 0; }
