// Copyright 2021 NK Labs, LLC

// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to permit
// persons to whom the Software is furnished to do so, subject to the
// following conditions:

// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
// CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT
// OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
// THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "version.h"

const NK_FLASH uint16_t firmware_major = NK_VERSION_MAJOR;
const NK_FLASH uint16_t firmware_minor = NK_VERSION_MINOR;
const NK_FLASH uint16_t build_year = NK_YEAR;
const NK_FLASH uint8_t build_month = NK_MONTH;
const NK_FLASH uint8_t build_day = NK_DAY;
const NK_FLASH uint8_t build_hour = NK_HOUR;
const NK_FLASH uint8_t build_minute = NK_MINUTE;
const NK_FLASH char git_hash[] = NK_GIT_REV;
