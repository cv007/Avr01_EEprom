
#### The linker script location is typically in the _avr/lib/ldscripts_ folder, so you can backup the original _avrxmega3.xn_ file if you wish before modification.

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

#### The header file is named as to not conflict with the existing header.h file, so can put this header01.h file in the same location.

#### The eeprom function is inline, and the EEWRITE macro attempts to be an all-in-one macro to make the single function work. The eememcpy function can also be used directly if wanted.
