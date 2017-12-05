
#include <drivers/keyscancodes.h>
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
      
      return esp;

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
	case 0x43: newKey=0x02; break; //F9
	case 0x44: newKey=0x03; break; //F10
	case 0x45: newKey=0x91; break; //NUMLOCK
	case 0x46: newKey=0x90; break; // SCROLL LOCK
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
	//case 0x53: newKey='.'; break;
	//case 0x57: newKey=0x57; break; // F11
	case 0x58: newKey=0x04; break; // F12
      }
      
      if(newKey !=0)
      {
	//printf("\nscancode=%02X",newKey);
	if(pressed == 1)
	  handler->OnKeyDown(newKey);
	else
	  handler->OnKeyUp(newKey);
      }
    }

    return esp;
}


uint32_t KeyboardDriver::HandleInterruptNew(uint32_t esp)
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
	case 0x4B: handler->OnKeyDown(KEY_SCANCODE_LEFT); break;  // Cursor Left 
	case 0x4D: handler->OnKeyDown(KEY_SCANCODE_RIGHT); break;  // Cursor Right 
	case 0x47: handler->OnKeyDown(KEY_SCANCODE_HOME); break;  // Cursor Home
	case 0x48: handler->OnKeyDown(KEY_SCANCODE_UP); break;  // Cursor Up  
	case 0x50: handler->OnKeyDown(KEY_SCANCODE_DOWN); break;  // Cursor Down
	
	case 0xCB: handler->OnKeyUp(KEY_SCANCODE_LEFT); break;  // Cursor Left 
	case 0xCD: handler->OnKeyUp(KEY_SCANCODE_RIGHT); break;  // Cursor Right 
	case 0xC7: handler->OnKeyUp(KEY_SCANCODE_HOME); break;  // Cursor Home
	case 0xC8: handler->OnKeyUp(KEY_SCANCODE_UP); break;  // Cursor Up  
	case 0xD0: handler->OnKeyUp(KEY_SCANCODE_DOWN); break;  // Cursor Down
      }
      
      return esp;

    }
    else
    { 
      uint8_t pressed = 1;
      
      if(key >= 0x80)
      {
	pressed = 0;
	key = key-128;
      }
      
      uint8_t newKey = KEY_SCANCODE_UNKNOWN;
      
      switch(key)
      {
	case 0x01: newKey=KEY_SCANCODE_ESCAPE; break;
	case 0x02: newKey=KEY_SCANCODE_1; break;
	case 0x03: newKey=KEY_SCANCODE_2; break;
	case 0x04: newKey=KEY_SCANCODE_3; break;
	case 0x05: newKey=KEY_SCANCODE_4; break;
	case 0x06: newKey=KEY_SCANCODE_5; break;
	case 0x07: newKey=KEY_SCANCODE_6; break;
	case 0x08: newKey=KEY_SCANCODE_7; break;
	case 0x09: newKey=KEY_SCANCODE_8; break;
	case 0x0A: newKey=KEY_SCANCODE_9; break;
	case 0x0B: newKey=KEY_SCANCODE_0; break;
	case 0x0C: newKey=KEY_SCANCODE_MINUS; break;
	case 0x0D: newKey=KEY_SCANCODE_EQUALS; break;
	case 0x0E: newKey=KEY_SCANCODE_BACKSPACE; break;
	case 0x0F: newKey=KEY_SCANCODE_TAB; break;
	case 0x10: newKey=KEY_SCANCODE_Q; break;
	case 0x11: newKey=KEY_SCANCODE_W; break;
	case 0x12: newKey=KEY_SCANCODE_E; break;
	case 0x13: newKey=KEY_SCANCODE_R; break;
	case 0x14: newKey=KEY_SCANCODE_T; break;
	case 0x15: newKey=KEY_SCANCODE_Y; break;
	case 0x16: newKey=KEY_SCANCODE_U; break;
	case 0x17: newKey=KEY_SCANCODE_I; break;
	case 0x18: newKey=KEY_SCANCODE_O; break;
	case 0x19: newKey=KEY_SCANCODE_P; break;
	case 0x1A: newKey=KEY_SCANCODE_LEFTBRACKET; break;
	case 0x1B: newKey=KEY_SCANCODE_RIGHTBRACKET; break;
	case 0x1C: newKey=KEY_SCANCODE_RETURN; break;
	case 0x1D: newKey=KEY_SCANCODE_LCTRL; break;
	case 0x1E: newKey=KEY_SCANCODE_A; break;
	case 0x1F: newKey=KEY_SCANCODE_S; break;
	case 0x20: newKey=KEY_SCANCODE_D; break;
	case 0x21: newKey=KEY_SCANCODE_F; break;
	case 0x22: newKey=KEY_SCANCODE_G; break;
	case 0x23: newKey=KEY_SCANCODE_H; break;
	case 0x24: newKey=KEY_SCANCODE_J; break;
	case 0x25: newKey=KEY_SCANCODE_K; break;
	case 0x26: newKey=KEY_SCANCODE_L; break;
	case 0x27: newKey=KEY_SCANCODE_SEMICOLON; break;
	case 0x28: newKey=KEY_SCANCODE_APOSTROPHE; break;
	case 0x29: newKey=KEY_SCANCODE_GRAVE; break;
	case 0x2A: newKey=KEY_SCANCODE_LSHIFT; break;
	case 0x2B: newKey=KEY_SCANCODE_BACKSLASH; break;
	case 0x2C: newKey=KEY_SCANCODE_Z; break;
	case 0x2D: newKey=KEY_SCANCODE_X; break;
	case 0x2E: newKey=KEY_SCANCODE_C; break;
	case 0x2F: newKey=KEY_SCANCODE_V; break;
	case 0x30: newKey=KEY_SCANCODE_B; break;
	case 0x31: newKey=KEY_SCANCODE_N; break;
	case 0x32: newKey=KEY_SCANCODE_M; break;
	case 0x33: newKey=KEY_SCANCODE_COMMA; break;
	case 0x34: newKey=KEY_SCANCODE_PERIOD; break;
	case 0x35: newKey=KEY_SCANCODE_SLASH; break;
	case 0x36: newKey=KEY_SCANCODE_RSHIFT; break;
	case 0x37: newKey=KEY_SCANCODE_KP_MULTIPLY; break;
	case 0x38: newKey=KEY_SCANCODE_LALT; break;
	case 0x39: newKey=KEY_SCANCODE_SPACE; break;
	case 0x3A: newKey=KEY_SCANCODE_CAPSLOCK; break;
	case 0x3B: newKey=KEY_SCANCODE_F1; break;
	case 0x3C: newKey=KEY_SCANCODE_F2; break;
	case 0x3D: newKey=KEY_SCANCODE_F3; break;
	case 0x3E: newKey=KEY_SCANCODE_F4; break;
	case 0x3F: newKey=KEY_SCANCODE_F5; break;
	case 0x40: newKey=KEY_SCANCODE_F6; break;
	case 0x41: newKey=KEY_SCANCODE_F7; break;
	case 0x42: newKey=KEY_SCANCODE_F8; break;
	case 0x43: newKey=KEY_SCANCODE_F9; break;
	case 0x44: newKey=KEY_SCANCODE_F10; break;
	case 0x45: newKey=KEY_SCANCODE_NUMLOCK; break;
	case 0x46: newKey=KEY_SCANCODE_SCROLLLOCK; break;
	case 0x47: newKey=KEY_SCANCODE_HOME; break;
	case 0x48: newKey=KEY_SCANCODE_KP_8; break;
	case 0x49: newKey=KEY_SCANCODE_KP_9; break;
	case 0x4A: newKey=KEY_SCANCODE_KP_MINUS; break;
	case 0x4B: newKey=KEY_SCANCODE_KP_4; break;
	case 0x4C: newKey=KEY_SCANCODE_KP_5; break;
	case 0x4D: newKey=KEY_SCANCODE_KP_6; break;
	case 0x4E: newKey=KEY_SCANCODE_KP_PLUS; break;
	case 0x4F: newKey=KEY_SCANCODE_KP_1; break;
	case 0x50: newKey=KEY_SCANCODE_KP_2; break;
	case 0x51: newKey=KEY_SCANCODE_KP_3; break;
	case 0x52: newKey=KEY_SCANCODE_KP_0; break;
	case 0x53: newKey=KEY_SCANCODE_KP_PERIOD; break;
	case 0x57: newKey=KEY_SCANCODE_F11; break;
	case 0x58: newKey=KEY_SCANCODE_F12; break;
      }
      
      if(newKey !=0)
      {
	//printf("\nscancode=%02X",newKey);
	if(pressed == 1)
	  handler->OnKeyDown(newKey);
	else
	  handler->OnKeyUp(newKey);
      }
    }

    return esp;
}