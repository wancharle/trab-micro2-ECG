all: 
	
	msp430-gcc -Wall -Os -mmcu=msp430g2553 -c -o lcd16.o lcd16.c
	msp430-gcc -Wall -Os -mmcu=msp430g2553 -c -o main.o main.c
	msp430-gcc -Wall -Os -mmcu=msp430g2553 -o trab_micro.elf main.o lcd16.o 
	msp430-objcopy -O ihex trab_micro.elf trab_micro.hex
	

clean:

	rm lcd16.o main.o
