
#include <drivers/keyboard.h>

using namespace myos::drivers;
using namespace myos::hardwarecommunication;


KeyboardEventHandler::KeyboardEventHandler()
{
}

void KeyboardEventHandler::OnKeyDown(uint8_t)
{
}

void KeyboardEventHandler::OnKeyUp(uint8_t)
{
}

KeyboardDriver::KeyboardDriver(InterruptManager* manager, KeyboardEventHandler *handler)
: InterruptHandler(manager, 0x21),
  dataport(0x60),
  commandport(0x64)
{
    this->handler = handler;
}

KeyboardDriver::~KeyboardDriver()
{
}

void KeyboardDriver::Activate()
{
    while(commandport.Read() & 0x1)
        dataport.Read();
    
    commandport.Write(0xae); // activate interrupts
    commandport.Write(0x20); // command 0x20 = read controller command byte
    
    uint8_t status = (dataport.Read() | 1) & ~0x10;
    commandport.Write(0x60); // command 0x60 = set controller command byte
    dataport.Write(status);
    dataport.Write(0xf4);
}

uint32_t KeyboardDriver::HandleInterrupt(uint32_t esp)
{
    uint8_t key = dataport.Read();

    if(handler == 0)
        return esp;
    
    // Extended keys
    if(key == 0xE0)
    {
      key = dataport.Read();
      
      switch(key)
      {
	case 0x4B: handler->OnKeyDown(0xFA); break;  // Cursor Left 
	case 0x4D: handler->OnKeyDown(0xFB); break;  // Cursor Right 
	case 0x47: handler->OnKeyDown(0xFC); break;  // Cursor Home
	case 0x48: handler->OnKeyDown(0xFD); break;  // Cursor Up  
	case 0x50: handler->OnKeyDown(0xFE); break;  // Cursor Down
	
	case 0xCB: handler->OnKeyUp(0xFA); break;  // Cursor Left 
	case 0xCD: handler->OnKeyUp(0xFB); break;  // Cursor Right 
	case 0xC7: handler->OnKeyUp(0xFC); break;  // Cursor Home
	case 0xC8: handler->OnKeyUp(0xFD); break;  // Cursor Up  
	case 0xD0: handler->OnKeyUp(0xFE); break;  // Cursor Down
      }
    }
    else
    { 
      uint8_t pressed = 1;
      
      if(key >= 0x80)
      {
	pressed = 0;
	key = key-128;
      }
      
      uint8_t newKey = 0;
      
      switch(key)
      {
	case 0x01: newKey=0x01; break; // ESC
	case 0x02: newKey='1'; break;
	case 0x03: newKey='2'; break;
	case 0x04: newKey='3'; break;
	case 0x05: newKey='4'; break;
	case 0x06: newKey='5'; break;
	case 0x07: newKey='6'; break;
	case 0x08: newKey='7'; break;
	case 0x09: newKey='8'; break;
	case 0x0A: newKey='9'; break;
	case 0x0B: newKey='0'; break;
	case 0x0C: newKey='-'; break;
	case 0x0D: newKey='='; break;
	case 0x0E: newKey=0x0E; break; // Backspace
	case 0x0F: newKey=0x0F; break; // Tab
	case 0x10: newKey='Q'; break;
	case 0x11: newKey='W'; break;
	case 0x12: newKey='E'; break;
	case 0x13: newKey='R'; break;
	case 0x14: newKey='T'; break;
	case 0x15: newKey='Y'; break;
	case 0x16: newKey='U'; break;
	case 0x17: newKey='I'; break;
	case 0x18: newKey='O'; break;
	case 0x19: newKey='P'; break;
	case 0x1A: newKey='['; break;
	case 0x1B: newKey=']'; break;
	case 0x1C: newKey='\n'; break;
	case 0x1D: newKey= 0x1D; break; // Left Ctrl
	case 0x1E: newKey='A'; break;
	case 0x1F: newKey='S'; break;
	case 0x20: newKey='D'; break;
	case 0x21: newKey='F'; break;
	case 0x22: newKey='G'; break;
	case 0x23: newKey='H'; break;
	case 0x24: newKey='J'; break;
	case 0x25: newKey='K'; break;
	case 0x26: newKey='L'; break;
	case 0x27: newKey=';'; break;
	case 0x28: newKey='\''; break;
	case 0x29: newKey='`'; break;
	case 0x2A: newKey=0x2A; break; //LEFTSHIFT
	case 0x2B: newKey=0x2B; break;
	case 0x2C: newKey='Z'; break;
	case 0x2D: newKey='X'; break;
	case 0x2E: newKey='C'; break;
	case 0x2F: newKey='V'; break;
	case 0x30: newKey='B'; break;
	case 0x31: newKey='N'; break;
	case 0x32: newKey='M'; break;
	case 0x33: newKey=','; break;
	case 0x34: newKey='.'; break;
	case 0x35: newKey='/'; break;
	case 0x36: newKey=0x2A; break; // RIGHTSHIFT
	case 0x37: newKey='*'; break;
	case 0x38: newKey=0xFF; break; //LEFT ALT
	case 0x39: newKey=' '; break;
	//case 0x3A: newKey=0x3A; break; // CAPSLOCK
	case 0x3B: newKey=0xF1; break; //F1
	case 0x3C: newKey=0xF2; break; //F2
	case 0x3D: newKey=0xF3; break; //F3
	case 0x3E: newKey=0xF4; break; //F4
	case 0x3F: newKey=0xF5; break; //F5
	case 0x40: newKey=0xF6; break; //F6
	case 0x41: newKey=0xF7; break; //F7
	case 0x42: newKey=0xF8; break; //F8
	//case 0x43: newKey=0x49; break; //F9
	//case 0x44: newKey=0x44; break; //F10
	//case 0x45: newKey=0x45; break; //NUMLOCK
	//case 0x46: newKey=0x46; break; // SCROLL LOCK
	case 0x47: newKey=0xFC; break; // HOME
	case 0x48: newKey=0xFD; break; // 8 - up
	//case 0x49: newKey='9'; break;
	//case 0x4A: newKey='-'; break;
	case 0x4B: newKey=0xFA; break; // 4 - left
	//case 0x4C: newKey='5'; break;
	case 0x4D: newKey=0xFB; break; // 6 - right
	//case 0x4E: newKey='+'; break;
	//case 0x4F: newKey='1'; break;
	case 0x50: newKey=0xFE; break; // 2 - down
	//case 0x51: newKey='3'; break;
	//case 0x52: newKey='0'; break;
	//case 0x53: newKey='.'; break;*/
	//case 0x57: newKey=0x57; break;	// F11
	//case 0x58: newKey=F12; break;
      }
      //printf("\nscancode=%02X",newKey);
      if(newKey !=0)
      {
	if(pressed == 1)
	  handler->OnKeyDown(newKey);
	else
	  handler->OnKeyUp(newKey);
      }
    }

    return esp;
}
