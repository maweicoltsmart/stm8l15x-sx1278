#include "stm8l15x_flash.h"

void __eeprom_write_8(unsigned short addr_eep,unsigned char data)
{
  FLASH_WaitForLastOperation(FLASH_MemType_Data);  
  FLASH_Unlock(FLASH_MemType_Data);
  
  FLASH_ProgramByte(addr_eep, data); 
  
  FLASH_WaitForLastOperation(FLASH_MemType_Data); 
  FLASH_Lock(FLASH_MemType_Data);  
}
void __eeprom_write_16(unsigned short addr_eep,unsigned short data)
{
  FLASH_WaitForLastOperation(FLASH_MemType_Data);  
  FLASH_Unlock(FLASH_MemType_Data);
  
  FLASH_ProgramByte(addr_eep,   data/256); 
  FLASH_WaitForLastOperation(FLASH_MemType_Data); 
  
  FLASH_ProgramByte(addr_eep+1, data%256);
  FLASH_WaitForLastOperation(FLASH_MemType_Data); 
  
  FLASH_Lock(FLASH_MemType_Data);  
}
void __eeprom_write_32(unsigned short addr_eep,unsigned long data)
{
  FLASH_WaitForLastOperation(FLASH_MemType_Data);  
  FLASH_Unlock(FLASH_MemType_Data);
  
  FLASH_ProgramByte(addr_eep,   (unsigned char)(data>>24)); 
  FLASH_WaitForLastOperation(FLASH_MemType_Data); 
  
  FLASH_ProgramByte(addr_eep+1, (unsigned char)(data>>16));
  FLASH_WaitForLastOperation(FLASH_MemType_Data); 
  
  FLASH_ProgramByte(addr_eep+2,   (unsigned char)(data>>8)); 
  FLASH_WaitForLastOperation(FLASH_MemType_Data); 
  
  FLASH_ProgramByte(addr_eep+3, (unsigned char)(data>>0));
  FLASH_WaitForLastOperation(FLASH_MemType_Data);
  
  FLASH_Lock(FLASH_MemType_Data);  
}

void __eeprom_write_many(unsigned short addr_eep,unsigned short size,unsigned short dummy,unsigned short addr_ram)
{
  
  FLASH_WaitForLastOperation(FLASH_MemType_Data);  
  FLASH_Unlock(FLASH_MemType_Data);
  
  for(unsigned short i=0;i<size;i++)
  {
     FLASH_ProgramByte(addr_eep+i,   *((unsigned char *)(addr_ram)+i)); 
     FLASH_WaitForLastOperation(FLASH_MemType_Data); 
  }
  
  
  FLASH_Lock(FLASH_MemType_Data);  
  
}