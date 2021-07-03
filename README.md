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
