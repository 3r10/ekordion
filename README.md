# EKordion

## Introduction

Creation of a digital button accordion. *EK* could mean :

- The author's initials.
- *Electronic Keyboard*.

## Hardware

### PCBs *(designed with Kicad)*

+ Right hand PCB
  - 35 Cherry MX switches
  - 5 shift registers (74hc165)
  - Daisy-chain : *not implemented*
  - 6 pin connector to be connected to a micro-controller *(Arduino, ESP32, ...)*
  - Pull-down resistors : pushing buttons means high level
+ Left hand PCB :
  - 24 Cherry MX switches
  - 3 shift registers (74hc165)
  - Daisy-chain : *available*
  - 7 pin output connector to be connected to a micro-controller *(Arduino, ESP32, ...)*
  or another PCB
  - Jumper for CLK INH pull-down
  - 6 pin input connector to be connected to another PCB
  - Jumper for serial input pull-up
  - Pull-up resistors : pushing buttons means low level.

### 3D prints and laser cut *(designed with OpenSCAD)*

+ Right hand
  - 35 button caps to fit the Cherry MX switches
+ Left hand 
  - 24 button caps to fit the CHerry MX switches with offset to allow 
  the required layout.
  - laser cutted grid

## Software

### ESP32 *(IDF environment)*

Polyphonic sound synthesizer.

+ Keyboard inputs via GPIOs
+ Sound output via I2S protocol and external DAC.
+ Control of synthesis parameters via classic bluetooth
+ Synthesis engine :
  - Three instruments : bass, chords, lead
  - Wavetable synthesis
  - 10 polyphonic sounds *(may be more)*
  - Vibrato
  - Small reverberation

### Android app *(Android Studio)*

+ Control of synthesis parameters via classic bluetooth