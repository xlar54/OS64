
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

//void printf(char*);
//void printfHex(uint8_t);

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
    
    // Treat left and right shift as same
    if (key == 0xAA | key == 0xB6)
    {
      handler->OnKeyUp(0xAA);
    }
    
    // Extended keys
    if(key == 0xE0)
    {
      uint8_t extdkey = dataport.Read();
      
      switch(extdkey)
      {
	case 0x4D: handler->OnKeyDown(0x1D); break; // Cursor Right 
	case 0x4B: handler->OnKeyDown(0x9D); break;  // Cursor Left 
	case 0x47: handler->OnKeyDown(0x13); break;  // Cursor Home
	case 0x48: handler->OnKeyDown(0x91); break;  // Cursor Up  
	case 0x50: handler->OnKeyDown(0x11); break;  // Cursor Down
      }
      
    }
    
    if(key < 0x80)
    {
        switch(key)
        {
	    case 0x01: handler->OnKeyDown(0x01); break; // ESC
            case 0x02: handler->OnKeyDown('1'); break;
            case 0x03: handler->OnKeyDown('2'); break;
            case 0x04: handler->OnKeyDown('3'); break;
            case 0x05: handler->OnKeyDown('4'); break;
            case 0x06: handler->OnKeyDown('5'); break;
            case 0x07: handler->OnKeyDown('6'); break;
            case 0x08: handler->OnKeyDown('7'); break;
            case 0x09: handler->OnKeyDown('8'); break;
            case 0x0A: handler->OnKeyDown('9'); break;
            case 0x0B: handler->OnKeyDown('0'); break;
	    case 0x0C: handler->OnKeyDown('-'); break;
	    case 0x0D: handler->OnKeyDown('='); break;
	    case 0x0E: handler->OnKeyDown(0x0E); break; // Backspace
	    case 0x0F: handler->OnKeyDown(0x0F); break; // Tab
            case 0x10: handler->OnKeyDown('Q'); break;
            case 0x11: handler->OnKeyDown('W'); break;
            case 0x12: handler->OnKeyDown('E'); break;
            case 0x13: handler->OnKeyDown('R'); break;
            case 0x14: handler->OnKeyDown('T'); break;
            case 0x15: handler->OnKeyDown('Y'); break;
            case 0x16: handler->OnKeyDown('U'); break;
            case 0x17: handler->OnKeyDown('I'); break;
            case 0x18: handler->OnKeyDown('O'); break;
            case 0x19: handler->OnKeyDown('P'); break;
	    case 0x1A: handler->OnKeyDown('['); break;
	    case 0x1B: handler->OnKeyDown(']'); break;
	    case 0x1C: handler->OnKeyDown('\n'); break;
            
	    case 0x1E: handler->OnKeyDown('A'); break;
            case 0x1F: handler->OnKeyDown('S'); break;
            case 0x20: handler->OnKeyDown('D'); break;
            case 0x21: handler->OnKeyDown('F'); break;
            case 0x22: handler->OnKeyDown('G'); break;
            case 0x23: handler->OnKeyDown('H'); break;
            case 0x24: handler->OnKeyDown('J'); break;
            case 0x25: handler->OnKeyDown('K'); break;
            case 0x26: handler->OnKeyDown('L'); break;
	    case 0x27: handler->OnKeyDown(';'); break;
            case 0x28: handler->OnKeyDown('\''); break;
	    case 0x29: handler->OnKeyDown('`'); break;
	    case 0x2A: handler->OnKeyDown(0x2A); break; //LEFTSHIFT
	    case 0x2B: handler->OnKeyDown('\\'); break;
	    case 0x2C: handler->OnKeyDown('Z'); break;
            case 0x2D: handler->OnKeyDown('X'); break;
            case 0x2E: handler->OnKeyDown('C'); break;
            case 0x2F: handler->OnKeyDown('V'); break;
            case 0x30: handler->OnKeyDown('B'); break;
            case 0x31: handler->OnKeyDown('N'); break;
            case 0x32: handler->OnKeyDown('M'); break;
            case 0x33: handler->OnKeyDown(','); break;
            case 0x34: handler->OnKeyDown('.'); break;
            case 0x35: handler->OnKeyDown('/'); break;
	    case 0x36: handler->OnKeyDown(0x2A); break; // RIGHTSHIFT (just sending the same as left for now)
	    case 0x37: handler->OnKeyDown('*'); break;
	    //case 0x38: handler->OnKeyDown(LEFTALT); break;
            case 0x39: handler->OnKeyDown(' '); break;
	    //case 0x3A: handler->OnKeyDown(CAPSLOCK); break;
	    case 0x3B: handler->OnKeyDown(0x3B); break; //F1
	    case 0x3C: handler->OnKeyDown(0x3C); break; //F2
	    case 0x3D: handler->OnKeyDown(0x3D); break; //F3
	    case 0x3E: handler->OnKeyDown(0x3E); break; //F4
	    case 0x3F: handler->OnKeyDown(0x3F); break; //F5
	    case 0x40: handler->OnKeyDown(0x40); break; //F6
	    case 0x41: handler->OnKeyDown(0x41); break; //F7
	    case 0x42: handler->OnKeyDown(0x42); break; //F8
	    case 0x43: handler->OnKeyDown(0x43); break; //F9
	    case 0x44: handler->OnKeyDown(0x44); break; //F10
	    //case 0x45: handler->OnKeyDown(NUMLOCK); break;
	    case 0x46: handler->OnKeyDown(0x46); break; // SCROLL LOCK
	    case 0x47: handler->OnKeyDown(0x47); break; // HOME
	    case 0x48: handler->OnKeyDown(0x91); break; // 8 - up
	    case 0x49: handler->OnKeyDown('9'); break;
	    case 0x4A: handler->OnKeyDown('-'); break;
	    case 0x4B: handler->OnKeyDown(0x9D); break; // 4 - left
	    case 0x4C: handler->OnKeyDown('5'); break;
	    case 0x4D: handler->OnKeyDown(0x1D); break; // 6 - right
	    case 0x4E: handler->OnKeyDown('+'); break;
	    case 0x4F: handler->OnKeyDown('1'); break;
	    case 0x50: handler->OnKeyDown(0x11); break; // 2 - down
	    case 0x51: handler->OnKeyDown('3'); break;
	    case 0x52: handler->OnKeyDown('0'); break;
	    case 0x53: handler->OnKeyDown('.'); break;
	    //case 0x57: handler->OnKeyDown(F11); break;
	    //case 0x58: handler->OnKeyDown(F12); break;

            /*default:
            {
                printf("KEYBOARD 0x");
                printfHex(key);
                break;
            }*/
        }
    }
    

    
    return esp;
}
