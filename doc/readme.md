# ESP32 Project Hardware Notes

## Development and Production Hardware

- **Development Board:**  
  The source code in this project was developed and tested using an ESP32 Dev Kit with a custom shield. Details about the shield's features and pin assignments can be found in [ESP32_Shield_Desc.txt](ESP32_Shield_Desc.txt).

- **Production Board:**  
  The final production hardware is represented in the schematic found in [Project_Prod_Schematic.pdf](Project_Prod_Schematic.pdf).

- **GPIO Pin Consistency:**  
  The GPIO pins used in the source code are the same as those used in the production board. This ensures that the firmware is fully compatible with both the development shield and the final production PCB.  
  All sensor and actuator connections (see `src/main.cpp`) are mapped to the same ESP32 GPIOs as shown in the schematic.

- **Reference Files:**  
  - [ESP32_Shield_Desc.txt](ESP32_Shield_Desc.txt): Description of the development shield and pin mapping.
  - [ESP32_pinout.png](ESP32_pinout.png): ESP32 pinout diagram.
  - [Project_Prod_Schematic.pdf](Project_Prod_Schematic.pdf): Schematic of the production board.

> **Note:** When adapting or deploying this code, always verify that your hardware connections match the GPIO assignments defined in the source code and schematic.