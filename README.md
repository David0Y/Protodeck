# Protodeck (spi-tests branch)  

SPI communication verification for **Protodeck** after completing perfboard soldering and bit-banging validation.   
These tests confirm that the hardware SPI interface on the nRF24L01+PA+LNA transceivers communicates reliably with the perfboard SPI rails across standard and elevated (over-)clock speeds.  
# Test Scope & Available Tests  

This branch contains two source files for different verification checks:  

1.  **Main SPI Test (main.cpp):** Verifies basic SPI register/transceiver communication for both modules (MOD I and MOD II).
2.  **SPI Speed Frequency Test (frequencytest_main.cpp):** Sweeps through increasing SPI clock speeds (1-12 MHz) to verify signal stability across frequencies on Mod I (PB0) and Mod II (PA4).
 
# Hardware Setup & Connection
1. Plug the USB-C cable into the exposed STM32F411CE BlackPill port to connect to your PC.
2. Connect the serial debugger pins to the exposed 4 upper header pins sticking out of the enclosure ceiling.
# Selecting a Test in platformio.ini during Flashing  
Similar to the bit-banging workflow, select the test you want to run by uncommenting only its corresponding line in platformio.ini:  
1. Save any file changes manually before building or flashing.
2. Open platformio.ini and remove the leading ; on only one build_src_filter line for the active test,never leave both uncommented at once:
  ```ini
 build_src_filter = +<*> -<main.cpp> ; for frequencytest_main.cpp
;build_src_filter = -<*> +<spi_peripheraltesting/src/main.cpp> ; for main.cpp
 ````
3. Open the CLI terminal and run the clean and upload commands rather than relying on the VS Code bottom status bar:
 ```ini
pio run -t clean
pio run -t upload
````

# Serial Monitor Verification 
Open your serial monitor (e.g., Tera Term) to inspect the test output. Both modules must report [PASS] / PASS for the SPI tests to be considered successful:  
   
   **Main SPI Test (main.cpp):**  
* **Pass:** Prints MOD I: PASS and MOD II: PASS
* **Fail:** Prints MOD I: FAIL and MOD II: FAIL

**Speed Frequency Test (frequencytest_main.cpp):**
* **Pass:** Prints Testing @ X MHz -> Mod I (PB0): [PASS] | Mod II (PA4): [PASS] at each tested frequency (1-12 MHz).
* **Fail:** Prints Testing @ X MHz -> Mod I (PB0): [FAIL] | Mod II (PA4): [FAIL] at each tested frequency (1-12 MHz).

