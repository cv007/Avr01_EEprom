#pragma once //eeprom01.h
#include <avr/io.h>

//simple version, 1 byte at a time, no interrupt protection
__attribute((always_inline)) inline
void eememcpy(void* src, void* eemem, uint8_t size){
    volatile uint8_t* s = (volatile uint8_t*)src;
    volatile uint8_t* d = (volatile uint8_t*)eemem;
    while( size-- ){
        *(volatile uint8_t*)0x1400; //ee read, blocks if busy
        //if( *d == *s ){ d++; s++; continue; } //skip write if not needed  (uncomment if wanted)      
        *d++ = *s++;
        _PROTECTED_WRITE_SPM( NVMCTRL.CTRLA, 3 );
    }
}

//version where bytes are written to page buffer until last byte
//or page buffer crossed, which will be faster than the simple
//version, but some more code generated, also no interrupt protection
__attribute((always_inline)) inline
void eememcpy2(void* src, void* eemem, uint8_t size){
    volatile uint8_t* s = (volatile uint8_t*)src;
    volatile uint8_t* d = (volatile uint8_t*)eemem;
    while( size-- ){
        *(volatile uint8_t*)0x1400; //ee read, blocks if busy
        *d++ = *s++;
        if( (size == 0) or ! ((int)d & (EEPROM_PAGE_SIZE-1)) ) { //last byte, or page crossed
            _PROTECTED_WRITE_SPM( NVMCTRL.CTRLA, 3 );
            }
    }
}

//same as eememcpy2 but has interrupt protection, so if an isr also writes to
//eeprom then the page buffer will be protected until the nvm command is run so
//the isr cannot corrupt the page buffer, or use a page buffer that has already 
//been writtem
//the isr can then do its owm eeprom writing and use any of the previous eememcpy
//versions
//may not be a great idea for an isr to have to potentially wait for eeprom, which 
//may take up to 4ms
__attribute((always_inline)) inline
void eememcpy3(void* src, void* eemem, uint8_t size){
    volatile uint8_t* s = (volatile uint8_t*)src;
    volatile uint8_t* d = (volatile uint8_t*)eemem;
    uint8_t sreg = SREG;
    while( size-- ){
        *(volatile uint8_t*)0x1400; //ee read, blocks if busy
        asm("cli");
        *d++ = *s++;
        if( (size == 0) or ! ((int)d & (EEPROM_PAGE_SIZE-1)) )  { //last byte, or page crossed
            _PROTECTED_WRITE_SPM( NVMCTRL.CTRLA, 3 );
            SREG = sreg;
            }
    }
}

//simple version
#define EEWRITE(eemem,val) \
    { typeof(eemem) v__ = (typeof(eemem))val; eememcpy( &v__, &eemem, sizeof(eemem) ); }
//version 2
#define EEWRITE2(eemem,val) \
    { typeof(eemem) v__ = (typeof(eemem))val; eememcpy2( &v__, &eemem, sizeof(eemem) ); }
//version 3
#define EEWRITE3(eemem,val) \
    { typeof(eemem) v__ = (typeof(eemem))val; eememcpy3( &v__, &eemem, sizeof(eemem) ); }

#define EEMEM  __attribute__(( section(".eeprom") ))
#define EEUSER __attribute__(( section(".userrow") ))
