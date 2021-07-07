# AVR0/1 EEprom
----------
#### This eeprom01.h header along with some linker script changes allows easier use of eeprom in the avr0/1 series of mcu's.

#### The linker script location is typically in the _avr/lib/ldscripts_ folder, and you can backup the original _avrxmega3.xn_ file if you wish before modification.

#### The following changes are needed in the linker script, which adds the VMA address to these eeprom/userrow sections, and also keeps their LMA address 0 based for the hex file-
````
    .eeprom 0x1400 : {

        KEEP(*(.eeprom*))
        PROVIDE( __eeprom_end = . ) ;

    } AT>eeprom

    .userrow 0x1300 : {

        KEEP(*(.userrow*))
        PROVIDE( __userrow_end = . ) ;

    } AT>userrow

````

#### The header file is named as to not conflict with the existing header.h file, so you can put this header01.h file in the same location.

#### The eeprom function is designed to handle all possibilities from a single byte on up, and the EEWRITE macro attempts to be an all-in-one macro to make the single function work. The eememcpy function can also be used directly if wanted.


#### Example-

````
#include <avr/io.h>
#include <avr/eeprom01.h>

EEMEM struct { PORT_t* port; uint8_t pinbm;  } eedata = { &PORTC, 1<<2 };
EEUSER uint32_t bootcount;

int main(){

    EEWRITE( bootcount, bootcount+1 );
    
    eedata.port->DIRSET = eedata.pinbm;
    eedata.port->OUTSET = eedata.pinbm;

    EEWRITE( eedata.port, &PORTA );
    EEWRITE( eedata.pinbm, 1<<5 );

    eedata.port->DIRSET = eedata.pinbm;
    eedata.port->OUTSET = eedata.pinbm;

    while(1){}

}

````
#### You do have to be aware that this write method (1 byte at a time) is used for simplicity, as you then do not have to deal with crossing eeprom pages. The code could be modified to check the page boundaries which would decrease the write time, but would use more flash in each use. If using the eememcpy directly, along with a bigger chunk of bytes, then it may make sense to modify or simply add an alternative eememcpy function, something like-

````
__attribute((always_inline)) inline
void eememcpy2(uint16_t src, uint16_t eemem, uint8_t size){
    volatile uint8_t* s = (volatile uint8_t*)src;
    volatile uint8_t* d = (volatile uint8_t*)eemem;
    while( size-- ){
        *(volatile uint8_t*)0x1400; //ee read, blocks if busy
        *d++ = *s++;
        if( n == 0 or not ((int)d bitand EEPROM_PAGE_SIZE-1) ) { //last byte, or page crossed
            _PROTECTED_WRITE_SPM( NVMCTRL.CTRLA, 3 );
            }
    }
}
````

#### There is also a potential problem if you start to do eeprom writing in both main code and isr code. An interrupt could happen in our process of writing to the page buffer and before the nvm command runs, so would have to protect these two so they are not split. First the global interrupt status is saved, and in the write loop interrupts have to be off bewteen the page buffer write(s) and the nvm command. Using the previous example which writes when a page is crossed or the last byte, then the cli will occur before the first page write (could be multiple bytes written to page), and interrupts are restored after an nvm command is started. If there are more bytes to write, then the process repeats, or if was the last byte then we leave with interrupts restored. Any isr that also wants to write to eeprom, will use these same functions and will also block on eeprom busy (and can decide to wait in the isr for eeprom or go to plan b).

#### The main code will be protecting its own page buffer, and the isr code can use the simpler eememcpy if it wants as it will not need to protect from an interrupt (ignoring there is a high priority interrupt available in avr0/1, and assume no one would want to deal with eeprom in that one).

````
__attribute((always_inline)) inline
void eememcpy3(uint16_t src, uint16_t eemem, uint8_t size){
    volatile uint8_t* s = (volatile uint8_t*)src;
    volatile uint8_t* d = (volatile uint8_t*)eemem;
    uint8_t sreg = SREG;
    while( size-- ){
        *(volatile uint8_t*)0x1400; //ee read, blocks if busy
        asm("cli");
        *d++ = *s++;
        if( n == 0 or not ((int)d bitand EEPROM_PAGE_SIZE-1) ) { //last byte, or page crossed
            _PROTECTED_WRITE_SPM( NVMCTRL.CTRLA, 3 );
            SREG = sreg;
            }
    }
}
````
