# Protodeck (nRF-testing branch)

Backup firmware branch for Protodeck, running a simpler, more rudimentary baseline version of the dual nRF24L01+PA+LNA transceiver firmware found in the main branch. This branch serves as a known-good fallback and baseline reference for verifying basic RF packet transmission and hardware behavior without the extended features of the primary firmware.

# Purpose & Scope

Fallback & Baseline Reference: Provides a stripped-down, straightforward implementation of the nRF24L01+PA+LNA test routines to isolate core transceiver functionality from higher-level firmware logic.

Hardware Sanity Check: Useful for quickly confirming that the STM32F411CE BlackPill, perfboard SPI rails, and dual transceivers communicate properly when troubleshooting changes on the main branch.

# Hardware Setup

1. Connect the ST-Link debugger to the 4 exposed debugger header pins on the enclosure ceiling.
2. Plug the USB-C cable into the exposed STM32F411CE BlackPill port to power the board and establish serial communication.

# Build & Flash (nrf-testing/)

1. Navigate to the firmware project folder (nrf-testing/) and open platformio.ini.
2. Save any local source edits manually before compiling.
3. Open the CLI terminal (avoiding the VS Code bottom status bar) and run:
```bash
pio run -t clean
pio run -t upload
````
Running pio run -t clean first ensures stale build artifacts are cleared before flashing the BlackPill.

# Serial Monitor Verification
Open your serial monitor (e.g., Tera Term) on the BlackPill COM port with the baud rate set to 9600.
