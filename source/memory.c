#include <stdio.h>

#include "registers.h"
#include "gpu.h"
#include "interrupts.h"
#include "debug.h"

#include "memory.h"

unsigned char cart[0x8000];
unsigned char sram[0x2000];
unsigned char io[0x100];
unsigned char vram[0x2000];
unsigned char oam[0x100];
unsigned char wram[0x2000];
unsigned char hram[0x80];

unsigned char readByte(unsigned short address) {
	// Set read breakpoints here
	//if(address == 0x0300) {
	//	realtimeDebugEnable = 1;
	//}
	
	if(address <= 0x7fff)
		return cart[address];
	
	else if(address >= 0xa000 && address <= 0xbfff)
		return sram[address - 0xa000];
	
	else if(address >= 0x8000 && address <= 0x9fff)
		return vram[address - 0x8000];
	
	else if(address >= 0xc000 && address <= 0xdfff)
		return wram[address - 0xc000];
	
	else if(address >= 0xe000 && address <= 0xfdff)
		return wram[address - 0xe000];
	
	else if(address >= 0xfe00 && address <= 0xfeff)
		return oam[address - 0xfe00];
	
	else if(address >= 0xff80 && address <= 0xfffe)
		return hram[address - 0xff80];
	
	else if(address == 0xff40) return gpu.control;
	else if(address == 0xff42) return gpu.scrollY;
	else if(address == 0xff43) return gpu.scrollX;
	else if(address == 0xff44) return gpu.scanline; // read only
	
	else if(address == 0xff0f) return interrupt.flags;
	else if(address == 0xffff) return interrupt.enable;
	
	return 0;
}

unsigned short readShort(unsigned short address) {
	return readByte(address) | (readByte(address + 1) << 8);
}

unsigned short readShortFromStack(void) {
	unsigned short value = readShort(registers.sp);
	registers.sp += 2;
	
	#ifdef DEBUG_STACK
		printf("Stack read 0x%04x\n", value);
	#endif
	
	return value;
}

void writeByte(unsigned short address, unsigned char value) {
	// Set write breakpoints here
	//if(address == 0xffa6) {
		//realtimeDebugEnable = 1;
	//}
	
	// Block writes to ff80
	if(tetrisPatch && address == 0xff80) return;
	
	if(address >= 0xa000 && address <= 0xbfff)
		sram[address - 0xa000] = value;
	
	else if(address >= 0x8000 && address <= 0x9fff) {
		vram[address - 0x8000] = value;
		updateTile(address, value);
	}
	
	if(address >= 0xc000 && address <= 0xdfff)
		wram[address - 0xc000] = value;
	
	else if(address >= 0xe000 && address <= 0xfdff)
		wram[address - 0xe000] = value;
	
	else if(address >= 0xfe00 && address <= 0xfeff)
		oam[address - 0xfe00] = value;
		
	else if(address >= 0xff80 && address <= 0xfffe)
		hram[address - 0xff80] = value;
	
	else if(address == 0xff40) gpu.control = value;
	else if(address == 0xff42) gpu.scrollY = value;
	else if(address == 0xff43) gpu.scrollX = value;
	else if(address == 0xff47) gpu.bgPalette = value; // write only
	
	else if(address == 0xff0f) interrupt.flags = value;
	else if(address == 0xffff) interrupt.enable = value;
}

void writeShort(unsigned short address, unsigned short value) {
	writeByte(address, (unsigned char)(value & 0x00ff));
	writeByte(address + 1, (unsigned char)((value & 0xff00) >> 8));
}

void writeShortToStack(unsigned short value) {
	registers.sp -= 2;
	writeShort(registers.sp, value);
	
	#ifdef DEBUG_STACK
		printf("Stack write 0x%04x\n", value);
	#endif
}
