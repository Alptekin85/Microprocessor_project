// low level routines for programming flash memory.  Interrupts should be disabled when doing any of this.
#include <stdint.h>
#include <stm32f031x6.h>
#include "nvm.h"

int  writeSector(uint32_t Address,void * values, uint16_t size)
{              
    uint16_t *AddressPtr;
    uint16_t *valuePtr;
    AddressPtr = (uint16_t *)Address;
    valuePtr=(uint16_t *)values;
    size = size / 2;  // incoming value is expressed in bytes, not 16 bit words
    while(size) {        
        // unlock the flash 
        // Key 1 : 0x45670123
        // Key 2 : 0xCDEF89AB
        FLASH->KEYR = 0x45670123;
        FLASH->KEYR = 0xCDEF89AB;
        FLASH->CR &= ~(1 << 1); // ensure PER is low
        FLASH->CR |= (1 << 0);  // set the PG bit        
        *(AddressPtr) = *(valuePtr);
        while(FLASH->SR & (1 << 0)); // wait while busy
        if (FLASH->SR & (1 << 2))
            return -1; // flash not erased to begin with
        if (FLASH->SR & (1 << 4))
            return -2; // write protect error
        AddressPtr++;
        valuePtr++;
        size--;
    }    
    return 0;    
}
void eraseSector(uint32_t SectorStartAddress)
{
    FLASH->KEYR = 0x45670123;
    FLASH->KEYR = 0xCDEF89AB;
    FLASH->CR &= ~(1 << 0);  // Ensure PG bit is low
    FLASH->CR |= (1 << 1);; // set the PER bit
    FLASH->AR = SectorStartAddress;
    FLASH->CR |= (1 << 6);; // set the start bit 
    while(FLASH->SR & (1 << 0)); // wait while busy
}
void readSector(uint32_t SectorStartAddress, void * values, uint16_t size)
{
    uint16_t *AddressPtr;
    uint16_t *valuePtr;
    AddressPtr = (uint16_t *)SectorStartAddress;
    valuePtr=(uint16_t *)values;
    size = size/2; // incoming value is expressed in bytes, not 16 bit words
    while(size)
    {
        *((uint16_t *)valuePtr)=*((uint16_t *)AddressPtr);
        valuePtr++;
        AddressPtr++;
        size--;
    }
}
