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
#include <vga.h>

// clas ctor and dtor //////////////////////////////////////////////////////////

IO::IO()
{
  write_regs(g_320x200x256);
  
  cols_ = Vic::kVisibleScreenWidth;
  rows_ = Vic::kVisibleScreenHeight;

  /**
   * unfortunately, we need to keep a copy of the rendered frame 
   * in our own memory, there does not seem to be a way around 
   * that would allow manipulating pixels straight on the GPU 
   * memory due to how the image is internally stored, etc..
   *
   * The rendered frame gets uploaded to the GPU on every 
   * screen refresh.
   */
  frame_  = new uint32_t[cols_ * rows_]();
  init_color_palette();
  init_keyboard();
  next_key_event_at_ = 0;
  //prev_frame_was_at_ = std::chrono::high_resolution_clock::now();
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
}

void IO::init_color_palette()
{
  outb(0x03c8, 0); outb(0x03c9, 0x00); outb(0x03c9, 0x00); outb(0x03c9, 0x00);  // black
  outb(0x03c8, 1); outb(0x03c9, 0xff); outb(0x03c9, 0xff); outb(0x03c9, 0xff);  // white
  outb(0x03c8, 2); outb(0x03c9, 0xff); outb(0x03c9, 0x00); outb(0x03c9, 0x00);  // red
  outb(0x03c8, 3); outb(0x03c9, 0x00); outb(0x03c9, 0xff); outb(0x03c9, 0xff);  // cyan
  outb(0x03c8, 4); outb(0x03c9, 0xff); outb(0x03c9, 0x00); outb(0x03c9, 0xff);  // violet
  outb(0x03c8, 5); outb(0x03c9, 0x00); outb(0x03c9, 0xff); outb(0x03c9, 0x00);  // green
  outb(0x03c8, 6); outb(0x03c9, 0x00); outb(0x03c9, 0x00); outb(0x03c9, 0xff);  // blue
  outb(0x03c8, 7); outb(0x03c9, 0xff); outb(0x03c9, 0xff); outb(0x03c9, 0x00);  // yellow
  outb(0x03c8, 8); outb(0x03c9, 0xff); outb(0x03c9, 0x67); outb(0x03c9, 0x00);  // orange
  outb(0x03c8, 9); outb(0x03c9, 0xa7); outb(0x03c9, 0x47); outb(0x03c9, 0x00);  // brown
  outb(0x03c8, 10); outb(0x03c9, 0xff); outb(0x03c9, 0x82); outb(0x03c9, 0xA7); // Lightred
  outb(0x03c8, 11); outb(0x03c9, 0x50); outb(0x03c9, 0x50); outb(0x03c9, 0x50); // Gray 1 (dark)
  outb(0x03c8, 12); outb(0x03c9, 0xa8); outb(0x03c9, 0xa8); outb(0x03c9, 0xa8); // Gray 2 (med)
  outb(0x03c8, 13); outb(0x03c9, 0x97); outb(0x03c9, 0xff); outb(0x03c9, 0x97); // Lightgreen
  outb(0x03c8, 14); outb(0x03c9, 0x97); outb(0x03c9, 0x97); outb(0x03c9, 0xff); // Lightblue
  outb(0x03c8, 15); outb(0x03c9, 0x70); outb(0x03c9, 0x70); outb(0x03c9, 0x70); // Gray 2 (light)
}

// emulation /////////////////////////////////////////////////////////////////// 

bool IO::emulate()
{
  return retval_;
}

void IO::process_events()
{

}

// keyboard handling /////////////////////////////////////////////////////////// 

void IO::OnKeyDown(uint8_t c)
{/*
    //char* foo = " ";
    //foo[0] = c;
    //printf(foo);

    // PC keycode to petscii translation.  We are just injecting to the keyboard buffer for now.
    switch(c)
    {
      case '9' : { if (leftShift == 1) c = 0x28; break; } // (
      case '0' : { if (leftShift == 1) c = 0x29; break; } // )
      case '\'': { if (leftShift == 1) c = 0x22; break; } // Double Quote
      case 0x08: { c = 0x14; break; } // Backspace
      case 0x0A: { c = 0x0D; break; } // Return
      case 0x2A: { leftShift = 1; return; }
      
      // doesnt work and its making me nuts.
      // Scroll lock key to switch back to text mode
      case 0x46: { setTextModeVGA(0); printf("hello"); return; } 
      
    }
    
    c64ptr->mem_->write_byte(631,c);
    c64ptr->mem_->write_byte(198,1);
    
*/
}
    
void IO::OnKeyUp(uint8_t c)
{
  if(c == 0xaa)
  {
    leftShift = 0;
  }
}

void IO::handle_keydown()
{

}

void IO::handle_keyup()
{

}

void IO::type_character(char c)
{

}

// screen handling /////////////////////////////////////////////////////////////

void IO::screen_draw_rect(int x, int y, int n, uint8_t color)
{

  for(int i=0; i < n ; i++)
  {
    unsigned short offset = (y<<8) + (y<<6) + (x+i);
    *((uint8_t*)(0xA0000 + offset)) = color;
  }
}
 
void IO::screen_draw_border(int y, int color)
{
  screen_draw_rect(0,y,cols_, color); //0x03);
}


void IO::screen_refresh()
{
  /* process SDL events once every frame */
  //process_events();
  /* perform vertical refresh sync */
  vsync();
}


void IO::vsync()
{
    // wait until done with vertical retrace */
    //while  ((inb(0x03da) & 0x08)) {};
    // wait until done refreshing */
    //while (!(inb(0x03da) & 0x08)) {};
}


