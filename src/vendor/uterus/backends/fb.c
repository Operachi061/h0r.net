/* Copyright (C) 2022-2023 mintsuki and contributors.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <stddef.h>
#include <stdint.h>

#include "../uterus.h"
#include "fb.h"

void *memset(void *, int, size_t);
void *memcpy(void *, const void *, size_t);

static uint8_t bump_alloc_pool[UTERUS_FB_BUMP_ALLOC_POOL_SIZE];
static size_t bump_alloc_ptr = 0;

static void *bump_alloc(size_t s) {
    size_t next_ptr = bump_alloc_ptr + s;
    if (next_ptr > UTERUS_FB_BUMP_ALLOC_POOL_SIZE) {
        return NULL;
    }
    void *ret = &bump_alloc_pool[bump_alloc_ptr];
    bump_alloc_ptr = next_ptr;
    return ret;
}

// Builtin font originally taken from:
// https://github.com/viler-int10h/vga-text-mode-fonts/raw/master/FONTS/PC-OTHER/TOSH-SAT.F16
static const uint8_t builtin_font[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x3c, 0x42, 0x81, 0x81, 0xa5, 0xa5, 0x81,
    0x81, 0xa5, 0x99, 0x81, 0x42, 0x3c, 0x00, 0x00, 0x00, 0x3c, 0x7e, 0xff,
    0xff, 0xdb, 0xdb, 0xff, 0xff, 0xdb, 0xe7, 0xff, 0x7e, 0x3c, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x6c, 0xfe, 0xfe, 0xfe, 0x7c, 0x7c, 0x38, 0x38, 0x10,
    0x10, 0x00, 0x00, 0x00, 0x00, 0x10, 0x10, 0x38, 0x38, 0x7c, 0x7c, 0xfe,
    0x7c, 0x7c, 0x38, 0x38, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18,
    0x3c, 0x3c, 0xdb, 0xff, 0xff, 0xdb, 0x18, 0x18, 0x3c, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x18, 0x3c, 0x7e, 0xff, 0xff, 0xff, 0x66, 0x18, 0x18,
    0x3c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x78,
    0x78, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xe7, 0xc3, 0xc3, 0xe7, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0xcc, 0x84, 0x84, 0xcc, 0x78, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc3, 0x99, 0xbd,
    0xbd, 0x99, 0xc3, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x1e,
    0x0e, 0x1e, 0x32, 0x78, 0xcc, 0xcc, 0xcc, 0x78, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x78, 0xcc, 0xcc, 0xcc, 0x78, 0x30, 0xfc, 0x30, 0x30,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x18, 0x1c, 0x1e, 0x16, 0x12,
    0x10, 0x10, 0x70, 0xf0, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x30, 0x38, 0x2c,
    0x26, 0x32, 0x3a, 0x2e, 0x26, 0x22, 0x62, 0xe2, 0xc6, 0x0e, 0x0c, 0x00,
    0x00, 0x00, 0x00, 0x18, 0x18, 0xdb, 0x3c, 0xe7, 0x3c, 0xdb, 0x18, 0x18,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xc0, 0xe0, 0xf8, 0xfe,
    0xf8, 0xe0, 0xc0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
    0x06, 0x0e, 0x3e, 0xfe, 0x3e, 0x0e, 0x06, 0x02, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x30, 0x78, 0xfc, 0x30, 0x30, 0x30, 0x30, 0x30, 0xfc, 0x78,
    0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0x00, 0xcc, 0xcc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xdb,
    0xdb, 0xdb, 0xdb, 0x7b, 0x1b, 0x1b, 0x1b, 0x1b, 0x1b, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x7c, 0xc6, 0x60, 0x38, 0x6c, 0xc6, 0xc6, 0x6c, 0x38, 0x0c,
    0xc6, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0xfe, 0xfe, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x78,
    0xfc, 0x30, 0x30, 0x30, 0x30, 0x30, 0xfc, 0x78, 0x30, 0xfc, 0x00, 0x00,
    0x00, 0x00, 0x30, 0x78, 0xfc, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
    0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
    0x30, 0x30, 0xfc, 0x78, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x18, 0x0c, 0xfe, 0xfe, 0x0c, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x60, 0xfe, 0xfe, 0x60, 0x30, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0,
    0xc0, 0xc0, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x24, 0x66, 0xff, 0xff, 0x66, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x10, 0x10, 0x38, 0x38, 0x7c, 0x7c, 0xfe, 0xfe,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0xfe, 0x7c, 0x7c,
    0x38, 0x38, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x30, 0x78, 0x78, 0x78, 0x78, 0x30, 0x30, 0x30, 0x00, 0x30,
    0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x6c, 0x6c,
    0x6c, 0xfe, 0x6c, 0x6c, 0x6c, 0xfe, 0x6c, 0x6c, 0x6c, 0x00, 0x00, 0x00,
    0x00, 0x18, 0x18, 0x7c, 0xc6, 0xc0, 0xc0, 0x7c, 0x06, 0x06, 0xc6, 0x7c,
    0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 0xc6, 0xc6, 0x0c, 0x0c, 0x18, 0x38,
    0x30, 0x60, 0x60, 0xc6, 0xc6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x6c,
    0x6c, 0x38, 0x30, 0x76, 0xde, 0xcc, 0xcc, 0xde, 0x76, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x18, 0x18, 0x18, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x30, 0x60, 0x60, 0x60, 0x60,
    0x60, 0x60, 0x60, 0x30, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x30,
    0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x30, 0x60, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x6c, 0x38, 0xfe, 0x38, 0x6c, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x7e,
    0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x30, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x06,
    0x0c, 0x0c, 0x18, 0x38, 0x30, 0x60, 0x60, 0xc0, 0xc0, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x7c, 0xc6, 0xc6, 0xc6, 0xd6, 0xd6, 0xd6, 0xc6, 0xc6, 0xc6,
    0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x38, 0x78, 0x18, 0x18, 0x18,
    0x18, 0x18, 0x18, 0x18, 0x7e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7c, 0xc6,
    0x06, 0x06, 0x0c, 0x18, 0x30, 0x60, 0xc0, 0xc0, 0xfe, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x7c, 0xc6, 0x06, 0x06, 0x3c, 0x06, 0x06, 0x06, 0x06, 0xc6,
    0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x0c, 0x1c, 0x3c, 0x6c, 0xcc,
    0xfe, 0x0c, 0x0c, 0x0c, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0xc0,
    0xc0, 0xc0, 0xfc, 0x06, 0x06, 0x06, 0x06, 0xc6, 0x7c, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x3c, 0x60, 0xc0, 0xc0, 0xfc, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6,
    0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0xc6, 0x06, 0x06, 0x0c, 0x18,
    0x30, 0x30, 0x30, 0x30, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7c, 0xc6,
    0xc6, 0xc6, 0x7c, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0x7c, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x7c, 0xc6, 0xc6, 0xc6, 0xc6, 0x7e, 0x06, 0x06, 0x06, 0x0c,
    0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18,
    0x18, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x18, 0x18, 0x30, 0x00, 0x00,
    0x00, 0x00, 0x06, 0x0c, 0x18, 0x30, 0x60, 0xc0, 0x60, 0x30, 0x18, 0x0c,
    0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x00,
    0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x60,
    0x30, 0x18, 0x0c, 0x06, 0x0c, 0x18, 0x30, 0x60, 0xc0, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x7c, 0xc6, 0xc6, 0x06, 0x0c, 0x18, 0x30, 0x30, 0x00, 0x30,
    0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7c, 0xc6, 0xc6, 0xc6, 0xde, 0xde,
    0xde, 0xde, 0xc0, 0xc0, 0x7e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x6c,
    0xc6, 0xc6, 0xc6, 0xc6, 0xfe, 0xc6, 0xc6, 0xc6, 0xc6, 0x00, 0x00, 0x00,
    0x00, 0x00, 0xfc, 0xc6, 0xc6, 0xc6, 0xfc, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6,
    0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7c, 0xc6, 0xc0, 0xc0, 0xc0, 0xc0,
    0xc0, 0xc0, 0xc0, 0xc6, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0xcc,
    0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0xcc, 0xf8, 0x00, 0x00, 0x00,
    0x00, 0x00, 0xfe, 0xc0, 0xc0, 0xc0, 0xfc, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0,
    0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0xc0, 0xc0, 0xc0, 0xfc, 0xc0,
    0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7c, 0xc6,
    0xc0, 0xc0, 0xc0, 0xde, 0xc6, 0xc6, 0xc6, 0xc6, 0x7e, 0x00, 0x00, 0x00,
    0x00, 0x00, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0xfe, 0xc6, 0xc6, 0xc6, 0xc6,
    0xc6, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x30, 0x30, 0x30, 0x30, 0x30,
    0x30, 0x30, 0x30, 0x30, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x0c,
    0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0xcc, 0xcc, 0x78, 0x00, 0x00, 0x00,
    0x00, 0x00, 0xc6, 0xc6, 0xcc, 0xd8, 0xf0, 0xe0, 0xf0, 0xd8, 0xcc, 0xc6,
    0xc6, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0,
    0xc0, 0xc0, 0xc0, 0xc0, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc6, 0xc6,
    0xee, 0xfe, 0xd6, 0xd6, 0xd6, 0xc6, 0xc6, 0xc6, 0xc6, 0x00, 0x00, 0x00,
    0x00, 0x00, 0xc6, 0xc6, 0xe6, 0xe6, 0xf6, 0xde, 0xce, 0xce, 0xc6, 0xc6,
    0xc6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7c, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6,
    0xc6, 0xc6, 0xc6, 0xc6, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0xc6,
    0xc6, 0xc6, 0xc6, 0xfc, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x7c, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0xf6, 0xda,
    0x6c, 0x06, 0x00, 0x00, 0x00, 0x00, 0xfc, 0xc6, 0xc6, 0xc6, 0xc6, 0xfc,
    0xd8, 0xcc, 0xcc, 0xc6, 0xc6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7c, 0xc6,
    0xc0, 0x60, 0x30, 0x18, 0x0c, 0x06, 0x06, 0xc6, 0x7c, 0x00, 0x00, 0x00,
    0x00, 0x00, 0xff, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
    0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6,
    0xc6, 0xc6, 0xc6, 0xc6, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc6, 0xc6,
    0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0x6c, 0x38, 0x10, 0x00, 0x00, 0x00,
    0x00, 0x00, 0xc6, 0xc6, 0xc6, 0xc6, 0xd6, 0xd6, 0xd6, 0xd6, 0xfe, 0x6c,
    0x6c, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc6, 0xc6, 0xc6, 0x6c, 0x38, 0x38,
    0x38, 0x6c, 0xc6, 0xc6, 0xc6, 0x00, 0x00, 0x00, 0x00, 0x00, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0x78, 0x30, 0x30, 0x30, 0x30, 0x30, 0x00, 0x00, 0x00,
    0x00, 0x00, 0xfe, 0x06, 0x06, 0x0c, 0x18, 0x30, 0x60, 0xc0, 0xc0, 0xc0,
    0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x60, 0x60, 0x60, 0x60, 0x60,
    0x60, 0x60, 0x60, 0x60, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0xc0,
    0x60, 0x60, 0x30, 0x38, 0x18, 0x0c, 0x0c, 0x06, 0x06, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x78, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
    0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x38, 0x6c, 0xc6, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00,
    0x00, 0x00, 0x18, 0x18, 0x18, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7c, 0x06, 0x06,
    0x7e, 0xc6, 0xc6, 0xc6, 0x7e, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0xc0,
    0xc0, 0xdc, 0xe6, 0xc6, 0xc6, 0xc6, 0xc6, 0xe6, 0xdc, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x7c, 0xc6, 0xc0, 0xc0, 0xc0, 0xc0, 0xc6,
    0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x06, 0x06, 0x76, 0xce, 0xc6,
    0xc6, 0xc6, 0xc6, 0xce, 0x76, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x7c, 0xc6, 0xc6, 0xfe, 0xc0, 0xc0, 0xc0, 0x7e, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x1c, 0x36, 0x30, 0x30, 0xfc, 0x30, 0x30, 0x30, 0x30, 0x30,
    0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x76, 0xce, 0xc6,
    0xc6, 0xc6, 0xce, 0x76, 0x06, 0xc6, 0x7c, 0x00, 0x00, 0x00, 0xc0, 0xc0,
    0xc0, 0xdc, 0xe6, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x18, 0x00, 0x00, 0x38, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
    0x3c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x1e, 0x06, 0x06,
    0x06, 0x06, 0x06, 0x06, 0xc6, 0xc6, 0x7c, 0x00, 0x00, 0x00, 0xc0, 0xc0,
    0xc0, 0xc6, 0xcc, 0xd8, 0xf0, 0xf0, 0xd8, 0xcc, 0xc6, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x38, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
    0x3c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xec, 0xfe, 0xd6,
    0xd6, 0xd6, 0xd6, 0xc6, 0xc6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0xdc, 0xe6, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x7c, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6,
    0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xdc, 0xe6, 0xc6,
    0xc6, 0xc6, 0xe6, 0xdc, 0xc0, 0xc0, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x76, 0xce, 0xc6, 0xc6, 0xc6, 0xce, 0x76, 0x06, 0x06, 0x06, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0xdc, 0xe6, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0,
    0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7c, 0xc6, 0xc0,
    0x70, 0x1c, 0x06, 0xc6, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x30,
    0x30, 0xfe, 0x30, 0x30, 0x30, 0x30, 0x30, 0x36, 0x1c, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6,
    0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc6, 0xc6, 0xc6,
    0xc6, 0xc6, 0x6c, 0x38, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0xc6, 0xc6, 0xd6, 0xd6, 0xd6, 0xd6, 0xfe, 0x6c, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0xc6, 0xc6, 0x6c, 0x38, 0x38, 0x6c, 0xc6,
    0xc6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc6, 0xc6, 0xc6,
    0xc6, 0xc6, 0xce, 0x76, 0x06, 0xc6, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0xfe, 0x06, 0x0c, 0x18, 0x30, 0x60, 0xc0, 0xfe, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x1c, 0x30, 0x30, 0x30, 0x30, 0xe0, 0x30, 0x30, 0x30, 0x30,
    0x1c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x30, 0x30, 0x30, 0x30, 0x00,
    0x30, 0x30, 0x30, 0x30, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x30,
    0x30, 0x30, 0x30, 0x1c, 0x30, 0x30, 0x30, 0x30, 0xe0, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x76, 0xdc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x10, 0x38, 0x38, 0x6c,
    0x6c, 0xc6, 0xc6, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3c, 0x66,
    0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0x66, 0x3c, 0x18, 0xcc, 0x78, 0x00,
    0x00, 0x00, 0x6c, 0x6c, 0x00, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6,
    0x7c, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x18, 0x30, 0x00, 0x7c, 0xc6, 0xc6,
    0xfe, 0xc0, 0xc0, 0xc0, 0x7e, 0x00, 0x00, 0x00, 0x00, 0x10, 0x38, 0x6c,
    0x00, 0x7c, 0x06, 0x06, 0x7e, 0xc6, 0xc6, 0xc6, 0x7e, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x6c, 0x6c, 0x00, 0x7c, 0x06, 0x06, 0x7e, 0xc6, 0xc6, 0xc6,
    0x7e, 0x00, 0x00, 0x00, 0x00, 0x60, 0x30, 0x18, 0x00, 0x7c, 0x06, 0x06,
    0x7e, 0xc6, 0xc6, 0xc6, 0x7e, 0x00, 0x00, 0x00, 0x00, 0x38, 0x6c, 0x38,
    0x00, 0x7c, 0x06, 0x06, 0x7e, 0xc6, 0xc6, 0xc6, 0x7e, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x7c, 0xc6, 0xc0, 0xc0, 0xc0, 0xc0, 0xc6,
    0x7c, 0x18, 0x0c, 0x38, 0x00, 0x10, 0x38, 0x6c, 0x00, 0x7c, 0xc6, 0xc6,
    0xfe, 0xc0, 0xc0, 0xc0, 0x7e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x6c, 0x6c,
    0x00, 0x7c, 0xc6, 0xc6, 0xfe, 0xc0, 0xc0, 0xc0, 0x7e, 0x00, 0x00, 0x00,
    0x00, 0x60, 0x30, 0x18, 0x00, 0x7c, 0xc6, 0xc6, 0xfe, 0xc0, 0xc0, 0xc0,
    0x7e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x6c, 0x6c, 0x00, 0x38, 0x18, 0x18,
    0x18, 0x18, 0x18, 0x18, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x10, 0x38, 0x6c,
    0x00, 0x38, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3c, 0x00, 0x00, 0x00,
    0x00, 0x60, 0x30, 0x18, 0x00, 0x38, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
    0x3c, 0x00, 0x00, 0x00, 0xc6, 0xc6, 0x10, 0x38, 0x6c, 0xc6, 0xc6, 0xc6,
    0xfe, 0xc6, 0xc6, 0xc6, 0xc6, 0x00, 0x00, 0x00, 0x38, 0x6c, 0x38, 0x00,
    0x38, 0x6c, 0xc6, 0xc6, 0xc6, 0xfe, 0xc6, 0xc6, 0xc6, 0x00, 0x00, 0x00,
    0x18, 0x30, 0x60, 0x00, 0xfe, 0xc0, 0xc0, 0xfc, 0xc0, 0xc0, 0xc0, 0xc0,
    0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xec, 0x36, 0x36,
    0x76, 0xde, 0xd8, 0xd8, 0x6e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x3c,
    0x6c, 0xcc, 0xcc, 0xfe, 0xcc, 0xcc, 0xcc, 0xcc, 0xce, 0x00, 0x00, 0x00,
    0x00, 0x10, 0x38, 0x6c, 0x00, 0x7c, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6,
    0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x6c, 0x6c, 0x00, 0x7c, 0xc6, 0xc6,
    0xc6, 0xc6, 0xc6, 0xc6, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x60, 0x30, 0x18,
    0x00, 0x7c, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0x7c, 0x00, 0x00, 0x00,
    0x00, 0x10, 0x38, 0x6c, 0x00, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6,
    0x7c, 0x00, 0x00, 0x00, 0x00, 0x60, 0x30, 0x18, 0x00, 0xc6, 0xc6, 0xc6,
    0xc6, 0xc6, 0xc6, 0xc6, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x6c, 0x6c,
    0x00, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0xce, 0x76, 0x06, 0xc6, 0x7c, 0x00,
    0x6c, 0x6c, 0x00, 0x7c, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6,
    0x7c, 0x00, 0x00, 0x00, 0x6c, 0x6c, 0x00, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6,
    0xc6, 0xc6, 0xc6, 0xc6, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30,
    0x30, 0x78, 0xcc, 0xc0, 0xc0, 0xcc, 0x78, 0x30, 0x30, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x38, 0x6c, 0x60, 0x60, 0x60, 0xf8, 0x60, 0x60, 0x60, 0xe6,
    0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0xcc, 0xcc, 0xcc, 0x78, 0x30, 0xfc,
    0x30, 0xfc, 0x30, 0x30, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0xcc,
    0xcc, 0xf8, 0xc4, 0xcc, 0xde, 0xcc, 0xcc, 0xcc, 0xc6, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x0e, 0x1b, 0x18, 0x18, 0x18, 0x7e, 0x18, 0x18, 0x18, 0x18,
    0x18, 0xd8, 0x70, 0x00, 0x00, 0x0c, 0x18, 0x30, 0x00, 0x7c, 0x06, 0x06,
    0x7e, 0xc6, 0xc6, 0xc6, 0x7e, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x18, 0x30,
    0x00, 0x38, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3c, 0x00, 0x00, 0x00,
    0x00, 0x0c, 0x18, 0x30, 0x00, 0x7c, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6,
    0x7c, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x18, 0x30, 0x00, 0xc6, 0xc6, 0xc6,
    0xc6, 0xc6, 0xc6, 0xc6, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x76, 0xdc, 0x00,
    0x00, 0xdc, 0xe6, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0x00, 0x00, 0x00,
    0x76, 0xdc, 0x00, 0xc6, 0xc6, 0xe6, 0xf6, 0xfe, 0xde, 0xce, 0xc6, 0xc6,
    0xc6, 0x00, 0x00, 0x00, 0x00, 0x78, 0xd8, 0xd8, 0x6c, 0x00, 0xfc, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x6c, 0x6c,
    0x38, 0x00, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x18, 0x18, 0x00, 0x18, 0x18, 0x30, 0x60, 0xc0, 0xc6, 0xc6,
    0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xfe, 0xc0, 0xc0, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xfe, 0x06, 0x06, 0x06, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0xc0, 0xc2, 0xc6, 0xcc, 0xd8, 0x30, 0x60, 0xdc, 0x86, 0x0c,
    0x18, 0x3e, 0x00, 0x00, 0x00, 0x00, 0xc0, 0xc2, 0xc6, 0xcc, 0xd8, 0x30,
    0x66, 0xce, 0x9e, 0x3e, 0x06, 0x06, 0x00, 0x00, 0x00, 0x00, 0x30, 0x30,
    0x00, 0x30, 0x30, 0x30, 0x78, 0x78, 0x78, 0x78, 0x30, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x36, 0x6c, 0xd8, 0x6c, 0x36, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xd8, 0x6c, 0x36,
    0x6c, 0xd8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x22, 0x88, 0x22, 0x88,
    0x22, 0x88, 0x22, 0x88, 0x22, 0x88, 0x22, 0x88, 0x22, 0x88, 0x22, 0x88,
    0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa,
    0x55, 0xaa, 0x55, 0xaa, 0xdd, 0x77, 0xdd, 0x77, 0xdd, 0x77, 0xdd, 0x77,
    0xdd, 0x77, 0xdd, 0x77, 0xdd, 0x77, 0xdd, 0x77, 0x18, 0x18, 0x18, 0x18,
    0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
    0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0xf8, 0xf8, 0x18, 0x18, 0x18,
    0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0xf8, 0xf8, 0x18,
    0x18, 0xf8, 0xf8, 0x18, 0x18, 0x18, 0x18, 0x18, 0x36, 0x36, 0x36, 0x36,
    0x36, 0x36, 0x36, 0xf6, 0xf6, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0xfe, 0x36, 0x36, 0x36,
    0x36, 0x36, 0x36, 0x36, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0xf8, 0x18,
    0x18, 0xf8, 0xf8, 0x18, 0x18, 0x18, 0x18, 0x18, 0x36, 0x36, 0x36, 0x36,
    0x36, 0xf6, 0xf6, 0x06, 0x06, 0xf6, 0xf6, 0x36, 0x36, 0x36, 0x36, 0x36,
    0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36,
    0x36, 0x36, 0x36, 0x36, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0xfe, 0x06,
    0x06, 0xf6, 0xf6, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36,
    0x36, 0xf6, 0xf6, 0x06, 0x06, 0xfe, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0xfe, 0xfe, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0xf8, 0xf8, 0x18,
    0x18, 0xf8, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0xf8, 0xf8, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
    0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x1f, 0x1f, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0xff,
    0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0xff, 0xff, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
    0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x1f, 0x1f, 0x18, 0x18, 0x18,
    0x18, 0x18, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff,
    0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x18, 0x18,
    0x18, 0x18, 0x18, 0xff, 0xff, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
    0x18, 0x18, 0x18, 0x18, 0x18, 0x1f, 0x1f, 0x18, 0x18, 0x1f, 0x1f, 0x18,
    0x18, 0x18, 0x18, 0x18, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x37,
    0x37, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36,
    0x36, 0x37, 0x37, 0x30, 0x30, 0x3f, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x3f, 0x30, 0x30, 0x37, 0x37, 0x36,
    0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0xf7, 0xf7, 0x00,
    0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0xff, 0xff, 0x00, 0x00, 0xf7, 0xf7, 0x36, 0x36, 0x36, 0x36, 0x36,
    0x36, 0x36, 0x36, 0x36, 0x36, 0x37, 0x37, 0x30, 0x30, 0x37, 0x37, 0x36,
    0x36, 0x36, 0x36, 0x36, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00,
    0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x36, 0x36, 0x36, 0x36,
    0x36, 0xf7, 0xf7, 0x00, 0x00, 0xf7, 0xf7, 0x36, 0x36, 0x36, 0x36, 0x36,
    0x18, 0x18, 0x18, 0x18, 0x18, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0xff,
    0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x18, 0x18, 0x18, 0x18, 0x18,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x36, 0x36, 0x36,
    0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x3f,
    0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x18, 0x18,
    0x18, 0x1f, 0x1f, 0x18, 0x18, 0x1f, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0x1f, 0x18, 0x18, 0x1f, 0x1f, 0x18,
    0x18, 0x18, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f,
    0x3f, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36,
    0x36, 0x36, 0x36, 0xff, 0xff, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36,
    0x18, 0x18, 0x18, 0x18, 0x18, 0xff, 0xff, 0x18, 0x18, 0xff, 0xff, 0x18,
    0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0xf8,
    0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x1f, 0x1f, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0xf0, 0xf0, 0xf0,
    0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0,
    0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f,
    0x0f, 0x0f, 0x0f, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x76, 0xd6, 0xdc, 0xc8, 0xc8, 0xdc, 0xd6, 0x76, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x78, 0xcc, 0xcc, 0xcc, 0xd8, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xd8, 0xc0, 0xc0, 0x00, 0x00, 0x00, 0xfe, 0xc6, 0xc6, 0xc0, 0xc0, 0xc0,
    0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x7e, 0xfe, 0x24, 0x24, 0x24, 0x24, 0x66, 0xc6, 0x00, 0x00, 0x00,
    0x00, 0x00, 0xfe, 0xfe, 0xc2, 0x60, 0x30, 0x18, 0x30, 0x60, 0xc2, 0xfe,
    0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7e, 0xc8, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x76, 0x6c, 0x60, 0xc0, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x66, 0xfc, 0x98, 0x18, 0x18, 0x18, 0x18,
    0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x30, 0x30, 0x78, 0xcc, 0xcc,
    0xcc, 0x78, 0x30, 0x30, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x6c,
    0xc6, 0xc6, 0xc6, 0xfe, 0xc6, 0xc6, 0xc6, 0x6c, 0x38, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x38, 0x6c, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0x6c, 0x6c, 0x6c,
    0xee, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0xcc, 0x60, 0x30, 0x78, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x76, 0xbb, 0x99, 0x99, 0xdd, 0x6e, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x02, 0x06, 0x3c, 0x6c, 0xce, 0xd6, 0xd6, 0xe6, 0x6c, 0x78,
    0xc0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x30, 0x60, 0xc0, 0xc0, 0xfe,
    0xc0, 0xc0, 0x60, 0x30, 0x1e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x6c,
    0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xfe, 0x00, 0x00, 0xfe, 0x00, 0x00, 0xfe, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x30, 0xfc,
    0x30, 0x30, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x60, 0x30, 0x18, 0x0c, 0x18, 0x30, 0x60, 0x00, 0xfc, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x18, 0x30, 0x60, 0xc0, 0x60, 0x30, 0x18, 0x00,
    0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x36, 0x36, 0x30, 0x30, 0x30,
    0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x18, 0x18, 0x18, 0x18,
    0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0xd8, 0xd8, 0x70, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x30, 0x30, 0x00, 0xfc, 0x00, 0x30, 0x30, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x76, 0xdc, 0x00,
    0x76, 0xdc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0xcc, 0xcc,
    0xcc, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x30, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x0c, 0x0c,
    0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0xcc, 0x6c, 0x3c, 0x1c, 0x0c, 0x00, 0x00,
    0x00, 0xd8, 0xec, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x6c, 0x0c, 0x18, 0x30, 0x60, 0x7c,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00};


static void plot_char(struct uterus_context *_ctx, struct uterus_fb_char *c,
                      size_t x, size_t y) {
    struct uterus_fb_context *ctx = (void *)_ctx;

    if (x >= _ctx->cols || y >= _ctx->rows) {
        return;
    }
    x = ctx->offset_x + x * ctx->glyph_width;
    y = ctx->offset_y + y * ctx->glyph_height;

    uint8_t *glyph = ctx->font_bits +
                     c->c * (((ctx->font_width / 8) + 1) * ctx->font_height);
    for (size_t gy = 0; gy < ctx->glyph_height; gy++) {
        volatile uint32_t *fb_line =
            ctx->framebuffer + x + (y + gy) * (ctx->pitch / 4);

        for (size_t fx = 0; fx < ctx->font_width; fx++) {
            bool draw = (glyph[gy * ((ctx->font_width / 8) + 1) + fx / 8] >>
                         (7 - fx % 8)) &
                        1;
                uint32_t bg = c->bg == 0xffffffff ? ctx->default_bg : c->bg;
                uint32_t fg = c->fg == 0xffffffff ? ctx->default_bg : c->fg;
                fb_line[fx] = draw ? fg : bg;
        }
    }
}

static inline bool compare_char(struct uterus_fb_char *a,
                                struct uterus_fb_char *b) {
    return !(a->c != b->c || a->bg != b->bg || a->fg != b->fg);
}

static void push_to_queue(struct uterus_context *_ctx,
                          struct uterus_fb_char *c, size_t x, size_t y) {
    struct uterus_fb_context *ctx = (void *)_ctx;

    if (x >= _ctx->cols || y >= _ctx->rows) {
        return;
    }

    size_t i = y * _ctx->cols + x;

    struct uterus_fb_queue_item *q = ctx->map[i];

    if (q == NULL) {
        if (compare_char(&ctx->grid[i], c)) {
            return;
        }
        q = &ctx->queue[ctx->queue_i++];
        q->x = x;
        q->y = y;
        ctx->map[i] = q;
    }

    q->c = *c;
}
static void uterus_fb_scroll(struct uterus_context *_ctx) {
    struct uterus_fb_context *ctx = (void *)_ctx;

    for (size_t i = (_ctx->scroll_top_margin + 1) * _ctx->cols;
         i < _ctx->scroll_bottom_margin * _ctx->cols; i++) {
        struct uterus_fb_char *c;
        struct uterus_fb_queue_item *q = ctx->map[i];
        if (q != NULL) {
            c = &q->c;
        } else {
            c = &ctx->grid[i];
        }
        push_to_queue(_ctx, c, (i - _ctx->cols) % _ctx->cols,
                      (i - _ctx->cols) / _ctx->cols);
    }

    // Clear the last line of the screen.
    struct uterus_fb_char empty;
    empty.c = ' ';
    empty.fg = ctx->text_fg;
    empty.bg = ctx->text_bg;
    for (size_t i = 0; i < _ctx->cols; i++) {
        push_to_queue(_ctx, &empty, i, _ctx->scroll_bottom_margin - 1);
    }
}

static void uterus_fb_clear(struct uterus_context *_ctx, bool move) {
    struct uterus_fb_context *ctx = (void *)_ctx;

    struct uterus_fb_char empty;
    empty.c = ' ';
    empty.fg = ctx->text_fg;
    empty.bg = ctx->text_bg;
    for (size_t i = 0; i < _ctx->rows * _ctx->cols; i++) {
        push_to_queue(_ctx, &empty, i % _ctx->cols, i / _ctx->cols);
    }

    if (move) {
        ctx->cursor_x = 0;
        ctx->cursor_y = 0;
    }
}

static void uterus_fb_set_cursor_pos(struct uterus_context *_ctx, size_t x,
                                       size_t y) {
    struct uterus_fb_context *ctx = (void *)_ctx;

    if (x >= _ctx->cols) {
        if ((int)x < 0) {
            x = 0;
        } else {
            x = _ctx->cols - 1;
        }
    }
    if (y >= _ctx->rows) {
        if ((int)y < 0) {
            y = 0;
        } else {
            y = _ctx->rows - 1;
        }
    }
    ctx->cursor_x = x;
    ctx->cursor_y = y;
}

static void uterus_fb_get_cursor_pos(struct uterus_context *_ctx, size_t *x,
                                       size_t *y) {
    struct uterus_fb_context *ctx = (void *)_ctx;

    *x = ctx->cursor_x >= _ctx->cols ? _ctx->cols - 1 : ctx->cursor_x;
    *y = ctx->cursor_y >= _ctx->rows ? _ctx->rows - 1 : ctx->cursor_y;
}

static void uterus_fb_set_text_fg(struct uterus_context *_ctx, size_t fg) {
    struct uterus_fb_context *ctx = (void *)_ctx;

    ctx->text_fg = ctx->ansi_colours[fg];
}

static void uterus_fb_set_text_bg(struct uterus_context *_ctx, size_t bg) {
    struct uterus_fb_context *ctx = (void *)_ctx;

    ctx->text_bg = ctx->ansi_colours[bg];
}

static void uterus_fb_set_text_fg_rgb(struct uterus_context *_ctx,
                                        uint32_t fg) {
    struct uterus_fb_context *ctx = (void *)_ctx;

    ctx->text_fg = fg;
}

static void uterus_fb_set_text_bg_rgb(struct uterus_context *_ctx,
                                        uint32_t bg) {
    struct uterus_fb_context *ctx = (void *)_ctx;

    ctx->text_bg = bg;
}

static void uterus_fb_set_text_fg_default(struct uterus_context *_ctx) {
    struct uterus_fb_context *ctx = (void *)_ctx;

    ctx->text_fg = ctx->default_fg;
}

static void uterus_fb_set_text_bg_default(struct uterus_context *_ctx) {
    struct uterus_fb_context *ctx = (void *)_ctx;

    ctx->text_bg = 0xffffffff;
}

static void draw_cursor(struct uterus_context *_ctx) {
    struct uterus_fb_context *ctx = (void *)_ctx;

    if (ctx->cursor_x >= _ctx->cols || ctx->cursor_y >= _ctx->rows) {
        return;
    }

    size_t i = ctx->cursor_x + ctx->cursor_y * _ctx->cols;

    struct uterus_fb_char c;

    c = ctx->grid[i];
    uint32_t tmp = c.fg;
    c.fg = c.bg;
    c.bg = tmp;
    plot_char(_ctx, &c, ctx->cursor_x, ctx->cursor_y);
}

static void uterus_fb_double_buffer_flush(struct uterus_context *_ctx) {
    struct uterus_fb_context *ctx = (void *)_ctx;

    if (_ctx->cursor_enabled) {
        draw_cursor(_ctx);
    }

    for (size_t i = 0; i < ctx->queue_i; i++) {
        struct uterus_fb_queue_item *q = &ctx->queue[i];
        size_t offset = q->y * _ctx->cols + q->x;
        if (ctx->map[offset] == NULL) {
            continue;
        }
        plot_char(_ctx, &q->c, q->x, q->y);
        ctx->grid[offset] = q->c;
        ctx->map[offset] = NULL;
    }

    if ((ctx->old_cursor_x != ctx->cursor_x ||
         ctx->old_cursor_y != ctx->cursor_y) ||
        _ctx->cursor_enabled == false) {
        if (ctx->old_cursor_x < _ctx->cols && ctx->old_cursor_y < _ctx->rows) {
            plot_char(
                _ctx,
                &ctx->grid[ctx->old_cursor_x + ctx->old_cursor_y * _ctx->cols],
                ctx->old_cursor_x, ctx->old_cursor_y);
        }
    }

    ctx->old_cursor_x = ctx->cursor_x;
    ctx->old_cursor_y = ctx->cursor_y;

    ctx->queue_i = 0;
}

static void uterus_fb_raw_putchar(struct uterus_context *_ctx, uint8_t c) {
    struct uterus_fb_context *ctx = (void *)_ctx;

    if (ctx->cursor_x >= _ctx->cols &&
        (ctx->cursor_y < _ctx->scroll_bottom_margin - 1 ||
         _ctx->scroll_enabled)) {
        ctx->cursor_x = 0;
        ctx->cursor_y++;
        if (ctx->cursor_y == _ctx->scroll_bottom_margin) {
            ctx->cursor_y--;
            uterus_fb_scroll(_ctx);
        }
        if (ctx->cursor_y >= _ctx->cols) {
            ctx->cursor_y = _ctx->cols - 1;
        }
    }

    struct uterus_fb_char ch;
    ch.c = c;
    ch.fg = ctx->text_fg;
    ch.bg = ctx->text_bg;
    push_to_queue(_ctx, &ch, ctx->cursor_x++, ctx->cursor_y);
}

static void uterus_fb_full_refresh(struct uterus_context *_ctx) {
    struct uterus_fb_context *ctx = (void *)_ctx;

    for (size_t y = 0; y < ctx->height; y++) {
        for (size_t x = 0; x < ctx->width; x++) {
            ctx->framebuffer[y * (ctx->pitch / sizeof(uint32_t)) + x] =
                ctx->default_bg;
        }
    }

    for (size_t i = 0; i < (size_t)_ctx->rows * _ctx->cols; i++) {
        size_t x = i % _ctx->cols;
        size_t y = i / _ctx->cols;

        plot_char(_ctx, &ctx->grid[i], x, y);
    }

    if (_ctx->cursor_enabled) {
        draw_cursor(_ctx);
    }
}

struct uterus_context *uterus_fb_init(
    uint32_t *framebuffer, size_t width, size_t height, size_t pitch,
    void *font, size_t font_width,
    size_t font_height) {

    struct uterus_fb_context *ctx = NULL;
    ctx = bump_alloc(sizeof(struct uterus_fb_context));
    if (ctx == NULL) {
        goto fail;
    }

    struct uterus_context *_ctx = (void *)ctx;

    memset(ctx, 0, sizeof(struct uterus_fb_context));

    ctx->ansi_colours[0] = 0x00000000; // black
    ctx->ansi_colours[1] = 0x00ff5555; // red
    ctx->ansi_colours[2] = 0x0055ff55; // green
    ctx->ansi_colours[3] = 0x00ffff55; // brown
    ctx->ansi_colours[4] = 0x005555ff; // blue
    ctx->ansi_colours[5] = 0x00ff55ff; // magenta
    ctx->ansi_colours[6] = 0x0055ffff; // cyan
    ctx->ansi_colours[7] = 0x00ffffff; // grey

    ctx->default_bg = 0x00000000; // background (black)
    ctx->default_fg = 0x00ffffff; // foreground (white)
    
    ctx->text_fg = ctx->default_fg;
    ctx->text_bg = 0xffffffff;

    ctx->framebuffer = (void *)framebuffer;
    ctx->width = width;
    ctx->height = height;
    ctx->pitch = pitch;

#define FONT_BYTES ((font_width * font_height * UTERUS_FB_FONT_GLYPHS) / 8)

    if (font != NULL) {
        ctx->font_width = font_width;
        ctx->font_height = font_height;
        ctx->font_bits = bump_alloc(FONT_BYTES);
        if (ctx->font_bits == NULL) {
            goto fail;
        }
        memcpy(ctx->font_bits, font, FONT_BYTES);
    } else {
        ctx->font_width = font_width = 8;
        ctx->font_height = font_height = 16;
        ctx->font_bits = bump_alloc(FONT_BYTES);
        if (ctx->font_bits == NULL) {
            goto fail;
        }
        memcpy(ctx->font_bits, builtin_font, FONT_BYTES);
    }

#undef FONT_BYTES

    // spacing is 1 pixel always
    ctx->font_width += 1;

    ctx->glyph_width = ctx->font_width;
    ctx->glyph_height = font_height;

    _ctx->cols = ctx->width / ctx->glyph_width;
    _ctx->rows = ctx->height / ctx->glyph_height;

    ctx->offset_x = (ctx->width % ctx->glyph_width) / 2;
    ctx->offset_y = (ctx->height % ctx->glyph_height) / 2;

    ctx->grid_size = _ctx->rows * _ctx->cols * sizeof(struct uterus_fb_char);
    ctx->grid = bump_alloc(ctx->grid_size);
    if (ctx->grid == NULL) {
        goto fail;
    }
    for (size_t i = 0; i < _ctx->rows * _ctx->cols; i++) {
        ctx->grid[i].c = ' ';
        ctx->grid[i].fg = ctx->text_fg;
        ctx->grid[i].bg = ctx->text_bg;
    }

    ctx->queue_size =
        _ctx->rows * _ctx->cols * sizeof(struct uterus_fb_queue_item);
    ctx->queue = bump_alloc(ctx->queue_size);
    if (ctx->queue == NULL) {
        goto fail;
    }
    ctx->queue_i = 0;
    memset(ctx->queue, 0, ctx->queue_size);

    ctx->map_size =
        _ctx->rows * _ctx->cols * sizeof(struct uterus_fb_queue_item *);
    ctx->map = bump_alloc(ctx->map_size);
    if (ctx->map == NULL) {
        goto fail;
    }
    memset(ctx->map, 0, ctx->map_size);

    _ctx->raw_putchar = uterus_fb_raw_putchar;
    _ctx->clear = uterus_fb_clear;
    _ctx->set_cursor_pos = uterus_fb_set_cursor_pos;
    _ctx->get_cursor_pos = uterus_fb_get_cursor_pos;
    _ctx->set_text_fg = uterus_fb_set_text_fg;
    _ctx->set_text_bg = uterus_fb_set_text_bg;
    _ctx->set_text_fg_rgb = uterus_fb_set_text_fg_rgb;
    _ctx->set_text_bg_rgb = uterus_fb_set_text_bg_rgb;
    _ctx->set_text_fg_default = uterus_fb_set_text_fg_default;
    _ctx->set_text_bg_default = uterus_fb_set_text_bg_default;
    _ctx->scroll = uterus_fb_scroll;
    _ctx->double_buffer_flush = uterus_fb_double_buffer_flush;
    _ctx->full_refresh = uterus_fb_full_refresh;

    uterus_context_reinit(_ctx);
    uterus_fb_full_refresh(_ctx);

    return _ctx;

fail:
    return NULL;
}
