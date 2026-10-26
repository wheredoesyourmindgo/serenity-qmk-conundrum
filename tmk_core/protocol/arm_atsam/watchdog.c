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

#include "samd51.h"
#include "watchdog.h"

void watchdog_init(void) {
#ifdef ARM_ATSAM_WATCHDOG
    // The watchdog runs from the 1.024kHz ultra low power oscillator: it resets the chip after 1
    // second without a feed, and warns after 256 cycles (250ms)
    WDT->CTRLA.reg = 0;
    while (WDT->SYNCBUSY.reg) {
    }
    WDT->CONFIG.reg  = WDT_CONFIG_PER_CYC1024;
    WDT->EWCTRL.reg  = WDT_EWCTRL_EWOFFSET_CYC256;
    WDT->INTFLAG.reg = WDT_INTFLAG_EW;
    WDT->INTENSET.reg = WDT_INTENSET_EW;

    // The early warning outranks every other interrupt, so it can catch one that never ends
    for (int irq = 0; irq < PERIPH_COUNT_IRQn; irq++) {
        if (NVIC_GetPriority(irq) == 0) {
            NVIC_SetPriority(irq, 1);
        }
    }
    NVIC_SetPriority(WDT_IRQn, 0);
    NVIC_EnableIRQ(WDT_IRQn);

    WDT->CTRLA.reg = WDT_CTRLA_ENABLE;
    while (WDT->SYNCBUSY.reg) {
    }
#endif
}

void watchdog_feed(void) {
#ifdef ARM_ATSAM_WATCHDOG
    if (!WDT->SYNCBUSY.bit.CLEAR) { // A clear is still syncing from the last feed
        WDT->CLEAR.reg = WDT_CLEAR_CLEAR_KEY;
    }
#endif
}

#ifdef ARM_ATSAM_WATCHDOG
// Frozen for 250ms. Waiting out the full timeout leaves the host repeating any key that was
// down, so reset now. Called from WDT_Handler (below) with the stacked exception frame.
__attribute__((noreturn, used)) void watchdog_warning(uint32_t *frame) {
#    ifdef ARM_ATSAM_BREADCRUMBS
    BREADCRUMBS->warn_pc   = frame[6];
    BREADCRUMBS->warn_lr   = frame[5];
    BREADCRUMBS->warn_xpsr = frame[7]; // Low bits: the exception it interrupted, 0 for the main loop
    for (int i = 0; i < 5; i++) {
        BREADCRUMBS->pending[i] = NVIC->ISPR[i];
        BREADCRUMBS->active[i]  = NVIC->IABR[i];
    }
#    else
    (void)frame;
#    endif
    NVIC_SystemReset();
    while (1) {
    }
}

__attribute__((naked)) void WDT_Handler(void) {
    __asm volatile(
        "tst lr, #4\n"
        "ite eq\n"
        "mrseq r0, msp\n"
        "mrsne r0, psp\n"
        "b watchdog_warning\n");
}
#endif

#ifdef ARM_ATSAM_BREADCRUMBS
#    define BREADCRUMBS_MAGIC 0x42524345 // "BRCE"

breadcrumbs_t breadcrumbs_previous;
uint32_t      breadcrumbs_reset_cause;

void breadcrumbs_boot(void) {
    breadcrumbs_reset_cause = RSTC->RCAUSE.reg;

    if (BREADCRUMBS->magic == BREADCRUMBS_MAGIC) {
        breadcrumbs_previous = *(breadcrumbs_t *)BREADCRUMBS;
    }

    // Backup RAM only loses power with the chip, so count resets until then
    uint32_t resets = breadcrumbs_previous.resets;
    if (breadcrumbs_reset_cause & (RSTC_RCAUSE_WDT | RSTC_RCAUSE_SYST)) {
        resets++;
    } else {
        resets = 0;
    }

    *(breadcrumbs_t *)BREADCRUMBS = (breadcrumbs_t){.resets = resets, .magic = BREADCRUMBS_MAGIC};
}

// Called from Dummy_Handler (startup.c) with the stacked exception frame: r0-r3, r12, lr, pc, xpsr
__attribute__((noreturn, used)) void breadcrumbs_fault(uint32_t *frame) {
    BREADCRUMBS->fault = __get_IPSR();
    BREADCRUMBS->lr    = frame[5];
    BREADCRUMBS->pc    = frame[6];
    NVIC_SystemReset();
    while (1) {
    }
}
#endif
