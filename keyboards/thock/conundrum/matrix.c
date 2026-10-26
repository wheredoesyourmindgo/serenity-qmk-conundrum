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
#include "watchdog.h"

// Signal thresholds, with a gap between them so keys don't flicker
#define EC_PRESS_THRESHOLD 125
#define EC_RELEASE_THRESHOLD 75

// Filled in by the ec_touch library as the PTC finishes each measurement
static uint16_t touch_signals[MATRIX_ROWS * MATRIX_COLS];

// The ec_touch library's PTC settings (QTouch qtm_acq_node_group_config_t); byte 5 is the PTC
// interrupt's priority, which the library applies each time it starts a measurement
extern uint8_t ptc_qtlib_acq_gen1[];
#define PTC_INTERRUPT_PRIORITY 5

#ifdef EC_TOUCH_DEBUG
#    include "print.h"
#    include "timer.h"

// State inside the ec_touch library (Thock's touch.c and Microchip's QTouch), found by disassembly
extern uint32_t touch_counter;          // Bumped after each completed measurement of all keys
extern uint8_t  measurement_done_touch; // Set when a measurement finished without error
extern uint8_t  module_error_code;      // Last error passed to the QTouch error callback
extern uint8_t  qtm_control[];          // [0]: QTouch binding layer flags
uint8_t         qtm_lib_get_state(void);
uint16_t        get_sensor_node_reference(uint16_t node);
uint16_t        get_sensor_node_signal(uint16_t node);
uint16_t        get_sensor_cc_val(uint16_t node);
uint8_t         get_sensor_state(uint16_t node);

#    define EC_DEBUG_INTERVAL 2000 // ms between status lines
#    define EC_DEBUG_STALL 200     // ms without a measurement before reporting a stall
#    define EC_KEY_COUNT (MATRIX_ROWS * MATRIX_COLS)
#    define EC_DUMP_PER_LINE 4 // Keys per dump line; the console buffer holds 512 bytes per main loop pass

static uint32_t debug_timer;
static uint32_t last_measurement;
static uint32_t measurements;
static uint32_t last_counter;
static uint8_t  last_error;
static bool     stalled;
static bool     first_measurement = true;
static int16_t  dump_next = -1; // Next key to dump, or -1 when not dumping

static void ec_debug_status(const char *reason) {
    uint16_t max_delta = 0, max_key = 0;
    uint16_t ref_min = UINT16_MAX, ref_max = 0, cc_min = UINT16_MAX, cc_max = 0;
    for (uint16_t i = 0; i < EC_KEY_COUNT; i++) {
        if (touch_signals[i] > max_delta) {
            max_delta = touch_signals[i];
            max_key   = i;
        }
        uint16_t ref = get_sensor_node_reference(i), cc = get_sensor_cc_val(i);
        if (ref < ref_min) ref_min = ref;
        if (ref > ref_max) ref_max = ref;
        if (cc < cc_min) cc_min = cc;
        if (cc > cc_max) cc_max = cc;
    }
    uprintf("ec %s t=%lu meas=%lu count=%lu err=%u lib=%u done=%u flags=%02X max=%u@%u,%u ref=%u-%u cc=%u-%u\n", reason, (unsigned long)(timer_read32() / 1000), (unsigned long)measurements, (unsigned long)touch_counter, module_error_code, qtm_lib_get_state(), measurement_done_touch, qtm_control[0], max_delta, max_key / MATRIX_COLS, max_key % MATRIX_COLS, ref_min, ref_max, cc_min, cc_max);
#    ifdef ARM_ATSAM_BREADCRUMBS
    // Why this boot happened (RSTC RCAUSE: 0x01 power on, 0x20 watchdog, 0x40 software or fault)
    // and where the previous run was (enum breadcrumb in watchdog.h)
    uprintf("ec boot cause=%02lX last=%lu fault=%lu pc=%08lX lr=%08lX resets=%lu\n", (unsigned long)breadcrumbs_reset_cause, (unsigned long)breadcrumbs_previous.crumb, (unsigned long)breadcrumbs_previous.fault, (unsigned long)breadcrumbs_previous.pc, (unsigned long)breadcrumbs_previous.lr, (unsigned long)BREADCRUMBS->resets);
    // Where the previous run froze, and the NVIC pending and active interrupts at the time
    uprintf("ec freeze pc=%08lX lr=%08lX xpsr=%08lX pending=%08lX,%08lX,%08lX,%08lX,%08lX active=%08lX,%08lX,%08lX,%08lX,%08lX\n", (unsigned long)breadcrumbs_previous.warn_pc, (unsigned long)breadcrumbs_previous.warn_lr, (unsigned long)breadcrumbs_previous.warn_xpsr, (unsigned long)breadcrumbs_previous.pending[0], (unsigned long)breadcrumbs_previous.pending[1], (unsigned long)breadcrumbs_previous.pending[2], (unsigned long)breadcrumbs_previous.pending[3], (unsigned long)breadcrumbs_previous.pending[4], (unsigned long)breadcrumbs_previous.active[0], (unsigned long)breadcrumbs_previous.active[1], (unsigned long)breadcrumbs_previous.active[2], (unsigned long)breadcrumbs_previous.active[3], (unsigned long)breadcrumbs_previous.active[4]);
#    endif
    measurements = 0;
}

// Per key: row,col state reference signal cc delta, a few keys per call
static void ec_debug_dump_step(void) {
    if (dump_next < 0) {
        return;
    }
    uprintf("ec key");
    for (uint8_t n = 0; n < EC_DUMP_PER_LINE && dump_next < EC_KEY_COUNT; n++, dump_next++) {
        uprintf(" %u,%u:s%02X r%u g%u c%u d%u", dump_next / MATRIX_COLS, dump_next % MATRIX_COLS, get_sensor_state(dump_next), get_sensor_node_reference(dump_next), get_sensor_node_signal(dump_next), get_sensor_cc_val(dump_next), touch_signals[dump_next]);
    }
    uprintf("\n");
    if (dump_next >= EC_KEY_COUNT) {
        dump_next = -1;
    }
}

static void ec_debug_task(void) {
    uint32_t now = timer_read32();

    if (touch_counter != last_counter) {
        measurements += touch_counter - last_counter;
        last_counter     = touch_counter;
        last_measurement = now;
        if (first_measurement) {
            first_measurement = false;
            ec_debug_status("first");
        }
        if (stalled) {
            stalled = false;
            ec_debug_status("resumed");
        }
    } else if (!stalled && timer_elapsed32(last_measurement) > EC_DEBUG_STALL) {
        stalled = true;
        ec_debug_status("STALL");
        dump_next = 0;
    }

    if (module_error_code != last_error) {
        last_error = module_error_code;
        ec_debug_status("ERROR");
        dump_next = 0;
    }

    if (timer_elapsed32(debug_timer) > EC_DEBUG_INTERVAL) {
        debug_timer = now;
        ec_debug_status(stalled ? "stalled" : "ok");
    }

    ec_debug_dump_step();
}
#endif // EC_TOUCH_DEBUG

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
#ifdef EC_TOUCH_DEBUG
    debug_timer = last_measurement = timer_read32();
#endif
}

bool matrix_scan_custom(matrix_row_t current_matrix[]) {
    BREADCRUMB(BC_EC_TOUCH);
    bool ready = ec_touch_ready();
    BREADCRUMB(BC_MATRIX_SCAN);
#ifdef EC_TOUCH_DEBUG
    ec_debug_task();
#endif
    if (!ready) {
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
