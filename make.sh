#!/bin/bash

cfiles="c/utils.c c/ble_utils.c c/recmem_utils.c"
objs="main.o utils.o ble_utils.o recmem_utils.o asm/library.S"


avr-gcc -g -Os -mmcu=atmega328p -c main.c ${cfiles}

avr-gcc -g -mmcu=atmega328p -o main.elf ${objs}

avr-objcopy -j .text -j .data -O ihex main.elf main.hex

avr-objdump -h -S main.elf > main.lst

avrdude -c usbasp -p m328p -P /dev/ttyUSB0 -b 57600 -U flash:w:main.hex -u -v

# Cleanup
rm *.elf *.o *.hex 2>/dev/null
