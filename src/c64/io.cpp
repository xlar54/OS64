/*
 * emudore, Commodore 64 emulator
 * Copyright (c) 2016, Mario Ballano <mballano@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <c64/io.h>
#include <c64/vic.h>
#include <lib/vga.h>

// current milliseconds since epoch
extern uint32_t current_milli;

// The 64 has only 64 keys. This matrix represents the keyboard
// to the CIA1 chip. PC keys must supply the same code for a keypress
// to be detected.  But actual key mappings can be changed
// via the memory.cpp file and altering the codes generated

/*
+----+----------------------+-------------------------------------------------------------------------------------------------------+
|    |                      |                                Peek from $dc01 (code in paranthesis):                                 |
|row:| $dc00:               +------------+------------+------------+------------+------------+------------+------------+------------+
|    |                      |   BIT 7    |   BIT 6    |   BIT 5    |   BIT 4    |   BIT 3    |   BIT 2    |   BIT 1    |   BIT 0    |
+----+----------------------+------------+------------+------------+------------+------------+------------+------------+------------+
|1.  | #%11111110 (254/$fe) | DOWN  ($  )|   F5  ($  )|   F3  ($  )|   F1  ($  )|   F7  ($  )| RIGHT ($  )| RETURN($  )|DELETE ($  )|
|2.  | #%11111101 (253/$fd) |LEFT-SH($  )|   e   ($05)|   s   ($13)|   z   ($1a)|   4   ($34)|   a   ($01)|   w   ($17)|   3   ($33)|
|3.  | #%11111011 (251/$fb) |   x   ($18)|   t   ($14)|   f   ($06)|   c   ($03)|   6   ($36)|   d   ($04)|   r   ($12)|   5   ($35)|
|4.  | #%11110111 (247/$f7) |   v   ($16)|   u   ($15)|   h   ($08)|   b   ($02)|   8   ($38)|   g   ($07)|   y   ($19)|   7   ($37)|
|5.  | #%11101111 (239/$ef) |   n   ($0e)|   o   ($0f)|   k   ($0b)|   m   ($0d)|   0   ($30)|   j   ($0a)|   i   ($09)|   9   ($39)|
|6.  | #%11011111 (223/$df) |   ,   ($2c)|   @   ($00)|   :   ($3a)|   .   ($2e)|   -   ($2d)|   l   ($0c)|   p   ($10)|   +   ($2b)|
|7.  | #%10111111 (191/$bf) |   /   ($2f)|   ^   ($1e)|   =   ($3d)|RGHT-SH($  )|  HOME ($  )|   ;   ($3b)|   *   ($2a)|   £   ($1c)|
|8.  | #%01111111 (127/$7f) | STOP  ($  )|   q   ($11)|COMMODR($  )| SPACE ($20)|   2   ($32)|CONTROL($  )|  <-   ($1f)|   1   ($31)|
+----+----------------------+------------+------------+------------+------------+------------+------------+------------+------------+
*/

const uint8_t IO::kbd[8][8] = {
  {0x0E, 0x0A, 0xFB, 0xF7 , 0xF1 , 0xF3 , 0xF5 , 0xFE},
  {'3' , 'W' , 'A' , '4'  , 'Z'  , 'S'  , 'E'  , 0x2A},
  {'5' , 'R' , 'D' , '6'  , 'C'  , 'F'  , 'T'  , 'X'},
  {'7' , 'Y' , 'G' , '8'  , 'B'  , 'H'  , 'U'  , 'V'},
  {'9' , 'I' , 'J' , '0'  , 'M'  , 'K'  , 'O'  , 'N'},
  {'+' , 'P' , 'L' , '-'  , '.'  , ';'  , '['  , ','},
  {0x5C, ']' , '\'', 0xFC , 0x2A , '='  , 0x2B , '/'},
  {'1' , '`' , 0x1D, '2'  , ' '  , 0xFF , 'Q'  , 0x0F}
};


// clas ctor and dtor //////////////////////////////////////////////////////////

IO::IO()
{  
  init_color_palette();
  init_keyboard();
  shift = 0;
  mode = 0;
  retval_ = true;
  
  cols_ = Vic::kVisibleScreenWidth;
  rows_ = Vic::kVisibleScreenHeight;
}

IO::~IO()
{
  delete [] frame_;
}

// init io devices  ////////////////////////////////////////////////////////////

void IO::init_keyboard()
{
  /* init keyboard matrix state */
  for(size_t i=0 ; i < sizeof(keyboard_matrix_) ; i++)
  {
    keyboard_matrix_[i] = 0xff;
  }
  
  joy1 = 0;
  joy2 = 0;
}

void IO::init_display(uint32_t* vgaMemAddress, uint32_t width, uint32_t height, uint32_t pitch, uint8_t bpp)
{
  vgaMem_ = (uint8_t*)vgaMemAddress;
  screen_width_ = width;
  screen_height_ = height;
  screen_pitch_ = pitch;
  screen_bpp_ = bpp;
  pixel_width_ = bpp/8;
  
  vscreen_ = new uint8_t[VIRT_WIDTH  * VIRT_HEIGHT];
  pscreen_ = new uint8_t[screen_pitch_ * screen_height_];

  scaleWidth =  (double)screen_pitch_ / (double)VIRT_WIDTH;
  scaleHeight = (double)screen_height_ / (double)VIRT_HEIGHT;
}

void IO::init_color_palette()
{
 color_palette[0] = ((0x00>>3)<<11) | ((0x00>>2)<<5) | (0x00>>3);
 color_palette[1] = ((0xff>>3)<<11) | ((0xff>>2)<<5) | (0xff>>3);
 color_palette[2] = ((0x68>>3)<<11) | ((0x37>>2)<<5) | (0x2b>>3);
 color_palette[3] = ((0x70>>3)<<11) | ((0xa4>>2)<<5) | (0xb2>>3);
 color_palette[4] = ((0x6f>>3)<<11) | ((0x3d>>2)<<5) | (0x86>>3);
 color_palette[5] = ((0x58>>3)<<11) | ((0x8d>>2)<<5) | (0x43>>3);
 color_palette[6] = ((0x35>>3)<<11) | ((0x28>>2)<<5) | (0x79>>3);
 color_palette[7] = ((0xb8>>3)<<11) | ((0xc7>>2)<<5) | (0x6f>>3);
 color_palette[8] = ((0x6f>>3)<<11) | ((0x4f>>2)<<5) | (0x25>>3);
 color_palette[9] = ((0x43>>3)<<11) | ((0x39>>2)<<5) | (0x00>>3);
 color_palette[10] = ((0x9a>>3)<<11) | ((0x67>>2)<<5) | (0x59>>3);
 color_palette[11] = ((0x44>>3)<<11) | ((0x44>>2)<<5) | (0x44>>3);
 color_palette[12] = ((0x6c>>3)<<11) | ((0x6c>>2)<<5) | (0x6c>>3);
 color_palette[13] = ((0x9a>>3)<<11) | ((0xd2>>2)<<5) | (0x84>>3);
 color_palette[14] = ((0x6c>>3)<<11) | ((0x5e>>2)<<5) | (0xb5>>3);
 color_palette[15] = ((0x95>>3)<<11) | ((0x95>>2)<<5) | (0x95>>3);
}

// emulation /////////////////////////////////////////////////////////////////// 

bool IO::emulate()
{
  // FAT32 hook
  uint8_t fat32cmd;
  fat32cmd =  mem_->read_byte(0x02);
  
  switch (fat32cmd)  
  {
    case 0x04:
    {
      file_load();
      break;
    }
    case 0x05:
    {
      file_save();
      break;
    }
    default:
      break;
  }

  static uint16_t waiter = 0;
  
  if(waiter == 100)
  {
    struct datetime curDateTime;
    rtc_->GetRTC(&curDateTime);
    
    mem_->write_byte(0x0334, (uint8_t)curDateTime.month);
    mem_->write_byte(0x0335, (uint8_t)curDateTime.day);
    mem_->write_byte(0x0336, (uint8_t)(curDateTime.year>>8));
    mem_->write_byte(0x0337, (uint8_t)(curDateTime.year & 0xFF));
    mem_->write_byte(0x0338, (uint8_t)curDateTime.hour);
    mem_->write_byte(0x0339, (uint8_t)curDateTime.minute);
    mem_->write_byte(0x033a, (uint8_t)curDateTime.second);
    
    waiter = 0;
  }
  
  waiter++;
  
  return retval_; 
}

void IO::process_events()
{

}

uint8_t IO::getJoystick(uint8_t num)
{
  if(num == 1)
  {
    return joy1;
  }
  
  if(num == 2)
  {
    return joy2;
  }
  
  return 0;
}

void IO::SendSerial(uint8_t c)
{
  serial_->Send(c);
}

// keyboard handling /////////////////////////////////////////////////////////// 

void IO::OnKeyDown(uint8_t c)
{
  if(c == 0x90)
  {
    step = !step;
    return;
  }
  
  if(c == 0x2A) // shift 
    joy2 |= (1 << 4);	// fire
  
  if(c == 0xFB)	// cursor right 
    joy2 |= (1 << 3);	// joy2 right

  
  if(c == 0xFE)	// cursor down 
    joy2 |= (1 << 1);	// joy2 right
  
  if(c == 0xFD) // cursor up (simulate shift-cursor down)
  {
    joy2 |= (1 << 0);	// joy2 up
    keyboard_matrix_[0] &= ~(1 << 7);
    keyboard_matrix_[1] &= ~(1 << 7); // SHIFT
    return;
  }
  
  if(c == 0xFA) // cursor left (simulate shift-cursor right)
  {
    joy2 |= (1 << 2);;
    keyboard_matrix_[0] &= ~(1 << 2);
    keyboard_matrix_[1] &= ~(1 << 7); // SHIFT
    return;
  }
  
  if(c == 0xF2) // F2
  {
    keyboard_matrix_[0] &= ~(1 << 4); // F1
    keyboard_matrix_[1] &= ~(1 << 7); // SHIFT
    return;
  }
  
  if(c == 0xF4) // F4
  {
    keyboard_matrix_[0] &= ~(1 << 5); // F3
    keyboard_matrix_[1] &= ~(1 << 7); // SHIFT
    return;
  }
  
  if(c == 0xF6) // F6
  {
    keyboard_matrix_[0] &= ~(1 << 6); // F5
    keyboard_matrix_[1] &= ~(1 << 7); // SHIFT
    return;
  }
  
  if(c == 0xF8) // F8
  {
    keyboard_matrix_[0] &= ~(1 << 3); // F7
    keyboard_matrix_[1] &= ~(1 << 7); // SHIFT
    return;
  }

  if(c == 0x02) // RESTORE keyboard (F9)
  {
    cpu_->nmi();
    return;
  }
   
  for(int row=0;row<8;row++)
    for(int col=0;col<8;col++)
      if(kbd[row][col] == c)
      {
	keyboard_matrix_[row] &= ~(1 << col);
      }  
}
    
void IO::OnKeyUp(uint8_t c)
{
  joy2 = 0;
  
  if(c == 0xFD) // cursor up
  {
    keyboard_matrix_[0] |= (1 << 7);
    keyboard_matrix_[1] |= (1 << 7); // SHIFT
    return;
  }
  
  if(c == 0xFA) // cursor left
  {
    keyboard_matrix_[0] |= (1 << 2);
    keyboard_matrix_[1] |= (1 << 7); // SHIFT
    return;
  }
  
  if(c == 0xF2) // F2
  {
    keyboard_matrix_[0] |= (1 << 4); // F1
    keyboard_matrix_[1] |= (1 << 7); // SHIFT
    return;
  }
   
  if(c == 0xF4) // F4
  {
    keyboard_matrix_[0] |= (1 << 5); // F3
    keyboard_matrix_[1] |= (1 << 7); // SHIFT
    return;
  }
  
  if(c == 0xF6) // F6
  {
    keyboard_matrix_[0] |= (1 << 6); // F5
    keyboard_matrix_[1] |= (1 << 7); // SHIFT
    return;
  }
  
  if(c == 0xF8) // F8
  {
    keyboard_matrix_[0] |= (1 << 3); // F7
    keyboard_matrix_[1] |= (1 << 7); // SHIFT
    return;
  }
  
  for(int row=0;row<8;row++)
    for(int col=0;col<8;col++)
      if(kbd[row][col] == c)
      {
	keyboard_matrix_[row] |= (1 << col);
      }
}


void IO::type_character(char c)
{

}

void IO::vsync()
{
  // This normally is called at the end of 
  // the screen_refresh() function
  uint32_t current = current_milli;
  uint32_t t = current - prev_frame_milli;
  uint32_t ttw = (Vic::kRefreshRate*1000) - t;
  
  while(current_milli < current+ ttw) {};
  prev_frame_milli = current_milli;
  
}

void IO::file_load()
{
  mem_->write_byte(0x02,0);	// clear command byte
  
  int fstatus = 0;
  
  uint8_t filenameLength = mem_->read_byte(0xB7);
  uint8_t secondaryAddr = mem_->read_byte(0xB9);
  uint8_t deviceNum = mem_->read_byte(0xBA);
  uint8_t filenamePtrLo = mem_->read_byte(0xBB);
  uint8_t filenamePtrHi = mem_->read_byte(0xBC);
	  
  uint16_t filenamePtr = (filenamePtrHi << 8) + (filenamePtrLo & 0xFF);
  uint16_t startAddress = (mem_->read_byte(0x2C) << 8) + (mem_->read_byte(0x2B) & 0xFF);
  uint16_t endAddress = (mem_->read_byte(0x2E) << 8) + (mem_->read_byte(0x2D) & 0xFF);
  uint16_t m = 0;
  uint8_t filenameBuffer[13]="        .PRG";
  uint16_t size;
  
  for(int z=0;z<filenameLength; z++)
    filenameBuffer[z] = mem_->read_byte(filenamePtr+z);
  
  // if $ is filename, load directory
  if (filenameBuffer[0] == '$' && filenameLength == 1)
  {
    int ctr=0;  
    uint8_t dir[4000];
    size = fat32_->GetCBMDir(dir);
    
    while (ctr <= size)
    {
      mem_->write_byte(startAddress + ctr, dir[ctr]);
      ctr++;
    }
    
    // tell basic where program ends (after 3 zeros)
    // regular kernel routines copy AE/AF to 2D/2E when done
    mem_->write_byte(0xAE, (startAddress + size) & 0xFF); // poke low byte to 45  
    mem_->write_byte(0xAF, (startAddress + size) >> 8); // poke hi byte to 46
    return;
  }
    
  
  fstatus = fat32_->OpenFile(1, (uint8_t*)filenameBuffer, FILEACCESSMODE_READ);
  
  if(fstatus == FILE_STATUS_NOTFOUND)
  {
    mem_->write_byte(0x90,0x42);	// ST = $0x42 (66 dec)
    return;
  }
  
  if(fstatus == FILE_STATUS_OK)
  {
    uint8_t b;
    size = 0;
    
    if(secondaryAddr == 1)
    {
      uint8_t lo = 0;
      uint8_t hi = 0;
      fstatus = fat32_->ReadNextFileByte(1, &lo);
      fstatus = fat32_->ReadNextFileByte(1, &hi);
      startAddress = (hi << 8) + (lo & 0xFF);	
    }

    fstatus = fat32_->ReadNextFileByte(1, &b);
    
    while(fstatus != FILE_STATUS_EOF)
    {     
      mem_->write_byte(startAddress+size, b);
      size++;
      
      fstatus = fat32_->ReadNextFileByte(1, &b);
    }
    fat32_->CloseFile(1);
    
    if (secondaryAddr == 0)
    {
      // tell basic where program ends (after 3 zeros)
      mem_->write_byte(0xAE, (startAddress + size) & 0xFF); // poke low byte to 45  
      mem_->write_byte(0xAF, (startAddress + size) >> 8); // poke hi byte to 46
    }
    
    mem_->write_byte(0x90,0x40);		// ST = $0x40 (64 dec)
  }
}

void IO::file_save()
{
  mem_->write_byte(0x02,0);	// clear command byte
  
  int fstatus = 0;
  
  uint8_t filenameLength = mem_->read_byte(0xB7);
  uint8_t secondaryAddr = mem_->read_byte(0xB9);
  uint8_t deviceNum = mem_->read_byte(0xBA);
  uint8_t filenamePtrLo = mem_->read_byte(0xBB);
  uint8_t filenamePtrHi = mem_->read_byte(0xBC);
	  
  uint16_t filenamePtr = (filenamePtrHi << 8) + (filenamePtrLo & 0xFF);
  uint16_t startAddress = (mem_->read_byte(0x2C) << 8) + (mem_->read_byte(0x2B) & 0xFF);
  uint16_t endAddress = (mem_->read_byte(0x2E) << 8) + (mem_->read_byte(0x2D) & 0xFF);
  
  uint16_t m = 0;
  uint8_t filenameBuffer[13]="        .PRG";
  
  for(int z=0;z<filenameLength; z++)
    filenameBuffer[z] = mem_->read_byte(filenamePtr+z);
  
  fstatus = fat32_->OpenFile(1, (uint8_t*)filenameBuffer, FILEACCESSMODE_CREATE);
	  
  if(fstatus == FILE_STATUS_OK)
  {
    uint8_t b;

    while(startAddress <= endAddress)
    {
	b = mem_->read_byte(startAddress++);
	fstatus = fat32_->WriteNextFileByte(1, b);
    }
    fat32_->CloseFile(1);
  }
}

void IO::file_open()
{
  mem_->write_byte(0x02,0);	// clear command byte
  
  int fstatus = 0;
  
  uint8_t filenameLength = mem_->read_byte(0xB7);
  uint8_t secondaryAddr = mem_->read_byte(0xB9);
  uint8_t deviceNum = mem_->read_byte(0xBA);
  uint8_t filenamePtrLo = mem_->read_byte(0xBB);
  uint8_t filenamePtrHi = mem_->read_byte(0xBC);
	  
  uint16_t filenamePtr = (filenamePtrHi << 8) + (filenamePtrLo & 0xFF);
  uint8_t filenameBuffer[13]="        .PRG";
  uint16_t size;
  
  for(int z=0;z<filenameLength; z++)
    filenameBuffer[z] = mem_->read_byte(filenamePtr+z);
  
  fstatus = fat32_->OpenFile(1, (uint8_t*)filenameBuffer, FILEACCESSMODE_READ);
  
  if(fstatus == FILE_STATUS_NOTFOUND)
  {
    mem_->write_byte(0x90,0x42);	// ST = $0x42 (66 dec)
    return;
  }
}

void IO::file_get()
{
}

void IO::file_close()
{

}
