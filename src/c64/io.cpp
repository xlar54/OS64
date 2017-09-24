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
  setColorVGA(0, 0, 0, 0);// black
  setColorVGA(1,63,63,63);// white
  setColorVGA(2,63, 0, 0);// red
  setColorVGA(3, 0,63,63);// cyan
  setColorVGA(4,63, 0,63);// violet
  setColorVGA(5, 0,63, 0);// green
  setColorVGA(6, 0, 0,63);// blue
  setColorVGA(7,63,63, 0);// yellow
  setColorVGA(8,63,36, 0);// orange
  setColorVGA(9,50,20, 0);// brown
  setColorVGA(10,63,40,50);// Lightred
  setColorVGA(11,20,20,20);// Gray 1 (dark)
  setColorVGA(12,30,30,30);// Gray 2 (med)
  setColorVGA(13,20,63,20);// Lightgreen
  setColorVGA(14,20,20,63);// Lightblue
  setColorVGA(15,40,40,40);// Gray 2 (light)

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

//void IO::screen_update_pixel(int x, int y, int color)
//{
//  vga_put_pixel(x, y,color);
//};

//void IO::screen_draw_rect(int x, int y, int n, uint8_t color)
//{
//  vga_draw_rect(x, y, n, color);
//}

//void IO::screen_draw_border(int y, int color)
//{
//  vga_draw_rect(0, y, cols_, color);
//}


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




