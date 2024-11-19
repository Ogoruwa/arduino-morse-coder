# Arduino Morse Coder - README  

This is a simple project for an **Arduino** that encodes user inputs into Morse code signals, which can be output via an LED, a buzzer, or any other connected component. The project is beginner-friendly and demonstrates the basics of programming with Arduino and working with output devices.

---

## Features  

- Encodes user serial inputs into Morse code.

- Non-blocking transission of Morse code.

- Outputs Morse code via an pin.

- Adjustable timing for dots, dashes, and pauses.

---

## Components Required 

- **Arduino Board** (Uno, Nano, etc.)  

- USB cable for programming the Arduino.  

- If the Arduino does **not** have an inbuilt LED:  
  - **Breadboard**  

  - **LED**  

  - **220-ohm resistor**  

---

## Wiring Diagram (Only if No Inbuilt LED)  
If your Arduino board does not have an inbuilt LED, you can use an external LED:

- Connect the LED's longer leg (anode) to a digital pin (e.g., pin 13).

- Connect the shorter leg (cathode) to a 220-ohm resistor and then to GND.

---

## Installation and Setup  

1. Install the **Arduino IDE** from [arduino.cc](https://www.arduino.cc/).  

2. Download the project files and open the `.ino` file in the Arduino IDE.  

3. (Optional if Arduino has an inbuilt pin) Set the output pins in the `setup` and `morse` functions. They are defined in the `pins` array.

4. (Optional) Change the starting message in the `loop` function. It is defined in the `input` array.

4. Upload the code to the Arduino board.

5. Send more messages serially using the USB cable.

---
