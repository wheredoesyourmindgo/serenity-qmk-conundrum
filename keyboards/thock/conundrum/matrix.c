/*
Copyright 2020 Thock Co

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "samd51.h"
#include "matrix.h"
#include "ec_touch.h"

// Signal thresholds, with a gap between them so keys don't flicker
#define EC_PRESS_THRESHOLD 125
#define EC_RELEASE_THRESHOLD 75

// Filled in by the ec_touch library as the PTC finishes each measurement
static uint16_t touch_signals[MATRIX_ROWS * MATRIX_COLS];

// The ec_touch library's PTC settings (QTouch qtm_acq_node_group_config_t); byte 5 is the PTC
// interrupt's priority, which the library applies each time it starts a measurement
extern uint8_t ptc_qtlib_acq_gen1[];
#define PTC_INTERRUPT_PRIORITY 5

void matrix_init_custom(void) {
    ec_touch_init(touch_signals);
    ec_touch_start();

    // The PTC interrupt starts each key's conversion, then waits for the ADC to report it busy.
    // The library runs it at priority 2, so USB and the timer (priority 0) could interrupt that
    // wait; when the conversion finished before the wait resumed, busy never showed and the
    // keyboard froze. Put every other interrupt below it. (Raising the PTC to priority 0 in the
    // library's settings instead stops its measurements from ever starting.)
    uint8_t ptc_priority = ptc_qtlib_acq_gen1[PTC_INTERRUPT_PRIORITY];
    for (int irq = 0; irq < PERIPH_COUNT_IRQn; irq++) {
        if (irq != ADC0_1_IRQn && NVIC_GetPriority(irq) <= ptc_priority) {
            NVIC_SetPriority(irq, ptc_priority + 1);
        }
    }
}

bool matrix_scan_custom(matrix_row_t current_matrix[]) {
    if (!ec_touch_ready()) {
        return false;
    }

    bool changed = false;

    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        matrix_row_t row_state = current_matrix[row];

        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            uint16_t     signal = touch_signals[row * MATRIX_COLS + col];
            matrix_row_t mask   = (matrix_row_t)1 << col;

            if ((row_state & mask) && signal < EC_RELEASE_THRESHOLD) {
                row_state &= ~mask;
            } else if (!(row_state & mask) && signal > EC_PRESS_THRESHOLD) {
                row_state |= mask;
            }
        }

        if (current_matrix[row] != row_state) {
            current_matrix[row] = row_state;
            changed             = true;
        }
    }

    return changed;
}
