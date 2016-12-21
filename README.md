# pi-avrdude-programmer


####avrdude with a Linux SPI programmer for Raspberry Pi

*edited by: P. Dockhorn 2016; original by: Kevin Cuzner Jun 2013, kevin@kevincuzner.com*

## compile avrdude

run:

- `apt-get update && apt-get install gcc git-core make  bison autoconf flex avr-libc gcc-avr`
- `git clone https://github.com/D0gi/pi-avrdude-programmer.git`
- `cd avrdude`
- `./bootstrap`
- `./configure`
- `make`
- `sudo make install`

optional:

- run `sudo raspi-config` 
- choose: `7 Advanced Options` >> `A6 SPI Enable/Disable automatic loading of SPI kernel module` and confirm
- reboot


## Pinout

- example for attiny261:

|Pi-GPIO|Pi-Pin|Type|attiny261-Pin|
|:-----:|:----:|:--:|:-----------:|
|GPIO 10| 19   |MOSI| 1           |
|GPIO 9 | 21   |MISO| 2           |
|GPIO 11| 23   |CLK | 3           |
|GPIO 25| 22   |RST | 10          |
| ---   |17(1) |3V3 |5(15)        |
| ---   |25(20)|GND |6(16)        |

- in addition a **pull-up-resistor** (~10kR) should be drawn from **3V3** to **RST** and optionally a capacitor (>100nF) between 3V3 and GND
- test connection: `sudo avrdude -c linuxspi -p attiny261 -P /dev/spidev0.0 -U flash:r:"/dev/null":r`

## use the makefiele for programming
*edited by: P. Dockhorn 2016; original by: Peter Fleury*

- the makefiele is stored in `prog/`, open it and edit:
```MCU = attiny261 ```
```F_CPU = 8000000 ```
```TARGET = cfilebla ```
to your needs

- use the folowing commands to ptogram your MyC:


 `make all` - Make software.

 `make clean` - Clean out built project files.

 `make coff` - Convert ELF to AVR COFF.

 `make extcoff` - Convert ELF to AVR Extended COFF.

 `make program` - Download the hex file to the device, using avrdude.
                Please customize the avrdude settings below first!

 `make debug` - Start either simulavr or avarice as specified for debugging, 
              with avr-gdb or avr-insight as the front end for debugging.

 `make filename.s` - Just compile filename.c into the assembler code only.

 `make filename.i` - Create a preprocessed source file for use in submitting
                   bug reports to the GCC project.
