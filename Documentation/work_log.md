# Work Log

This file will serve as detailed documentation for progress made to the circuit itself, in addition to any changes made to non-software aspects of the project. For software progress, see commits, pulls etc.

## 20/06/19 - 01/01/20

Inspired by [Ben Eater's videos](https://www.youtube.com/playlist?list=PLowKtXNTBypGqImE405J2565dvjafglHU), I decided to build a similar computer for my final ECE high-school project. I knew this project would take a very long time, so I started working on it the summer before school year.

I watched Eater's videos and decided on a few changes I'd make to the computer (I can't just copy his work for my own project, right?):
* Use the MCM6810P 128-byte RAM chip instead of the 74189 64-bit chip

* Since the project guidelines demand that I use some sort of a ready microprocessor, the computer will be interfaced with an Arduino using a physical driver circuit (still not sure that's the right term) consisting of various MUXs and gates.

  * The basic feature will be giving an 8-bit input to the input register and reading the output of the computer
  * Other features I might include if I have time are:
  
    * Automatic programming to RAM
    * Assembler
    * Reading programs from SD card

The specs of the final project will be (as of 02/01/20):

* Adjustable clock frequency of about 1 Hz to 1.35 kHz with a manual pulse (monostable) mode

* 8-bit processing, memory and output

* 128 byte RAM

* 8-bit word input

* Quad 7-seg display for output

* I/O interface with Arduino

I then began building the circuit on breadboards. After building a few parts of the computer and going on a short haitus, I realized I made a grave mistake while building my circuit without drawing schematics first. I've then drawn my schematics and made sure to draw one before building each part.

After a bare-bones version of the computer was built, testing began (don't worry, I tested each part individually). It could run basic programs successfuly, but the main problem was that sometimes the counter in the control unit would skip a beat, throwing the program execution off-course. My main theory was that the clock signals weren't clean enough, but I couldn't test that since I didn't have an oscilloscope at the time.

Around that time, the teachers told me I had to build the project on either a wire-wrap board or a PCB. Perhaps it was for the better, since breadboard connections aren't exactly reliable. Since I don't know how to work on PCBs, I decided to start building my computer on several wire-wrap boards.

A more detailed work log in Hebrew can be found [here](https://docs.google.com/document/d/1f-CUX1oLNrWuMgMoHW-Djt5JsNIpseODwH8WzywEzs0/edit?usp=sharing).

## 02/01/20 (10:00)

I've decided that in addition to the work log I wrote on Google Docs in Hebrew until now (and which was required by the project guidelines), I'll create a public GitHub repository where I'll document my progress in English (starting now at least). This is my first serious repository so it's going to take a while until I learn the ropes.

My progress until now:
* Schematics for the entire project

* Arduino programs for EEPROM programming and interfacing with the computer

* A complete breadboard circuit (minus the Arduino driver)

* One wire-wrap board consisting of these modules:

  * Clock
  * Memory Address Register (MAR)
  * RAM
  * Instruction Register (IR)
  * Input register
  * Board port (with connections to Arduino drivers for the input)
  
* Second wire-wrap board with Program Counter (PC). Will consist of:

  * Registers A and B
  * ALU
  * Flags register
  * Output
  
* Plans for 2 more boards for the Control Unit and Arduino Driver
