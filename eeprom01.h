#pragma once //eeprom01.h
#include <avr/io.h>

__attribute((always_inline)) inline
void eememcpy(uint16_t src, uint16_t eemem, uint8_t size){
    volatile uint8_t* s = (volatile uint8_t*)src;
    volatile uint8_t* d = (volatile uint8_t*)eemem;
    while( size-- ){
        *(volatile uint8_t*)0x1400; //ee read, blocks if busy
        *d++ = *s++;
        _PROTECTED_WRITE_SPM( NVMCTRL.CTRLA, 3 );
    }
}

#define EEWRITE(eemem,val) \
    { typeof(eemem) v__ = (typeof(eemem))val; eememcpy( (uint16_t)&v__, (uint16_t)&eemem, sizeof(eemem) ); }

#define EEMEM  __attribute__(( section(".eeprom") ))
#define EEUSER __attribute__(( section(".userrow") ))
