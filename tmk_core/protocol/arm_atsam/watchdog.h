/*
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

// Freeze recovery and diagnosis, both off unless a keyboard builds with them:
//
// ARM_ATSAM_WATCHDOG: the main loop feeds the watchdog, so if the firmware freezes for 250ms the
// chip resets itself, as if the keyboard were unplugged and plugged back in.
//
// ARM_ATSAM_BREADCRUMBS: the firmware notes where it is in backup RAM, which survives a reset,
// and a fault or an interrupt with no handler records itself and resets. A freeze records where
// it was stuck and which interrupts were pending or running. The next boot can then report why
// it reset and where it was.

#pragma once

#include <stdint.h>

enum breadcrumb {
    BC_NONE,
    BC_MAIN_LOOP,
    BC_SUSPENDED,
    BC_KEYBOARD_TASK,
    BC_SEND_KEYBOARD_WAIT, // Waiting for the host to take the previous keyboard report
    BC_SEND_NKRO_WAIT,
    BC_CONSOLE_WAIT, // Waiting for the host to take the previous console report
    BC_HOUSEKEEPING,
    BC_MATRIX_SCAN,
    BC_EC_TOUCH, // Inside the keyboard's touch sensing library
};

void watchdog_init(void);
void watchdog_feed(void);

#ifdef ARM_ATSAM_BREADCRUMBS
typedef struct {
    uint32_t magic;
    uint32_t crumb;  // enum breadcrumb
    uint32_t fault;  // Exception number of a fault or unhandled interrupt (IPSR), 0 if none
    uint32_t pc;     // Where the fault happened
    uint32_t lr;     // Its caller
    uint32_t resets; // Resets by the watchdog or a fault since power on

    // Captured by the watchdog's early warning when frozen
    uint32_t warn_pc;    // Where it was stuck
    uint32_t warn_lr;    // Its caller
    uint32_t warn_xpsr;  // Low bits: the exception it was in, 0 for the main loop
    uint32_t pending[5]; // NVIC interrupts waiting (ISPR)
    uint32_t active[5];  // NVIC interrupts running (IABR)
} breadcrumbs_t;

#    define BREADCRUMBS ((volatile breadcrumbs_t *)0x47000000) // Backup RAM

extern breadcrumbs_t breadcrumbs_previous; // As left by the previous run
extern uint32_t      breadcrumbs_reset_cause;  // RSTC->RCAUSE at this boot

void breadcrumbs_boot(void);

static inline uint32_t breadcrumb_swap(uint32_t where) {
    uint32_t previous   = BREADCRUMBS->crumb;
    BREADCRUMBS->crumb = where;
    return previous;
}
#    define BREADCRUMB(where) (BREADCRUMBS->crumb = (where))
#else
static inline uint32_t breadcrumb_swap(uint32_t where) {
    (void)where;
    return BC_NONE;
}
#    define BREADCRUMB(where) ((void)(where))
#endif
