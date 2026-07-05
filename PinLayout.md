| Arduino Pin | Function            | Target Component           | Wire Color   | Description / Note                                                    |
| :---        | :---                | :---                       | :---         | :---                                                                  |
| **D2**      | Digital Input       | Mill Button                | Orange       | Trigger Button for the Mill (Switches to GND. Use `INPUT_PULLUP`!)    |
| **D4**      | Digital Output      | Stepper Driver 1 (IN1)     | TODO         | Stepper control pin 1 (Motor 1)                                       |
| **D5**      | Digital Output      | Stepper Driver 1 (IN2)     | TODO         | Stepper control pin 2 (Motor 1)                                       |
| **D6**      | Digital Output      | Stepper Driver 1 (IN3)     | TODO         | Stepper control pin 3 (Motor 1)                                       |
| **D7**      | Digital Output      | Stepper Driver 1 (IN4)     | TODO         | Stepper control pin 4 (Motor 1)                                       |
| **D8**      | Digital Output      | Stepper Driver 2 (IN1)     | Green        | Stepper control pin 1 (Motor 2)                                       |
| **D9**      | Digital Output      | Stepper Driver 2 (IN2)     | Blue         | Stepper control pin 2 (Motor 2)                                       |
| **D10**     | Digital Output      | Stepper Driver 2 (IN3)     | Green-White  | Stepper control pin 3 (Motor 2)                                       |
| **D11**     | Digital Output      | Stepper Driver 2 (IN4)     | Blue-White   | Stepper control pin 4 (Motor 2)                                       |
| **D12**     | Digital Output      | Mill Relay (IN)            | Brown        | Relay control for the mill motor                                      |
| **A1**      | Digital Input       | HX711 Module (DOUT)        | TODO         | Scale data line / DOUT                                                |
| **A2**      | Digital Output      | HX711 Module (PD_SCK)      | TODO         | Scale clock line / SCK                                                |
| **A4**      | Hardware I2C (SDA)  | SSD1306 OLED Display (SDA) | TODO         | Display data line (I2C)                                               |
| **A5**      | Hardware I2C (SCL)  | SSD1306 OLED Display (SCL) | TODO         | Display clock line (I2C)                                              |
| **5V**      | Power Input/Output  | External 5V Power Rail     | Red          | Main system voltage (5V)                                              |
| **GND**     | Ground              | External Ground            | Black        | Common ground reference for all components                            |
