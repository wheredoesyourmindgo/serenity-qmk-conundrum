# Conundrum

A 4x12 ortholinear keyboard by [Thock.co](https://thock.co) with rubber dome EC switches, sensed capacitively by the SAMD51's PTC.

* Hardware: ATSAMD51G18A, Adafruit UF2 bootloader
* Key sensing: prebuilt `lib/ec_touch` library (Thock's code plus Microchip QTouch, no source available)

Make example for this keyboard (after setting up your build environment):

    make thock/conundrum:default

This produces `thock_conundrum_default.uf2`. Enter the bootloader and copy the file to the drive that appears.
