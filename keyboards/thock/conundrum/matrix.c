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

#include "matrix.h"
#include "ec_touch.h"

// Signal thresholds, with a gap between them so keys don't flicker
#define EC_PRESS_THRESHOLD 125
#define EC_RELEASE_THRESHOLD 75

// Filled in by the ec_touch library as the PTC finishes each measurement
static uint16_t touch_signals[MATRIX_ROWS * MATRIX_COLS];

void matrix_init_custom(void) {
    ec_touch_init(touch_signals);
    ec_touch_start();
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
