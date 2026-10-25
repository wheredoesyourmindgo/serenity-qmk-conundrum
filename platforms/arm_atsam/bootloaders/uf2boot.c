/* Copyright 2020 Thock Co
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "bootloader.h"

#include "samd51.h"

// Adafruit uf2-samdx1 stays in bootloader mode after a reset when this
// "double tap" magic value is found in the last word of RAM
extern uint32_t _eram;
#define BOOTLOADER_MAGIC 0xf01669ef
#define MAGIC_ADDR (uint32_t *)((intptr_t)(&_eram) - 4)

void bootloader_jump(void) {
    *MAGIC_ADDR = BOOTLOADER_MAGIC; // Set magic number into RAM
    NVIC_SystemReset();             // Perform system reset
    while (1)
        ; // Won't get here
}

__attribute__((weak)) void mcu_reset(void) {
    NVIC_SystemReset();
}
