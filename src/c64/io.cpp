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
  shift = 0;
  mode = 0;
  retval_ = true;
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
  vga_set_color(0, 0, 0, 0);// black
  vga_set_color(1,63,63,63);// white
  vga_set_color(2,63, 0, 0);// red
  vga_set_color(3, 0,63,63);// cyan
  vga_set_color(4,63, 0,63);// violet
  vga_set_color(5, 0,63, 0);// green
  vga_set_color(6, 0, 0,63);// blue
  vga_set_color(7,63,63, 0);// yellow
  vga_set_color(8,63,36, 0);// orange
  vga_set_color(9,50,20, 0);// brown
  vga_set_color(10,63,40,50);// Lightred
  vga_set_color(11,20,20,20);// Gray 1 (dark)
  vga_set_color(12,30,30,30);// Gray 2 (med)
  vga_set_color(13,20,63,20);// Lightgreen
  vga_set_color(14,30,30,63);// Lightblue
  vga_set_color(15,40,40,40);// Gray 2 (light)

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
{
	// PC keycode to petscii translation.  We are just injecting to the keyboard buffer for now.
	switch(c)
	{
	  case '1' : { if (shift == 1) c = 0x21; break; } // (
	  case '2' : { if (shift == 1) c = 0x40; break; } // (
	  case '3' : { if (shift == 1) c = 0x23; break; } // (
	  case '4' : { if (shift == 1) c = 0x24; break; } // (
	  case '5' : { if (shift == 1) c = 0x25; break; } // (
	  case '6' : { if (shift == 1) c = 0x20; break; } // (
	  case '7' : { if (shift == 1) c = 0x26; break; } // (
	  case '8' : { if (shift == 1) c = 0x2A; break; } // (
	  case '9' : { if (shift == 1) c = 0x28; break; } // (
	  case '0' : { if (shift == 1) c = 0x29; break; } // )
	  case ',' : { if (shift == 1) c = 0x3C; break; } // )
	  case '.' : { if (shift == 1) c = 0x3E; break; } // )
	  case ';' : { if (shift == 1) c = 0x3A; break; } // )
	  case '/' : { if (shift == 1) c = 0x3F; else c= 0x2F; break; } // )
	  case '\'': { if (shift == 1) c = 0x22; break; } // Double Quote
	  case 0x0E: { c = 0x14; break; } // Backspace
	  case 0x0A: { c = 0x0D; break; } // Return
	  case '=':  { if (shift == 1) c = 0x2B; else c= 0x3D; break; } // )
	  //case 0x3B: { c = 0x85; break; } // F1
	  /*case 0x3C: { c = 0x89; break; } // F2
	  case 0x3D: { c = 0x86; break; } // F3
	  case 0x3E: { c = 0x8A; break; } // F4
	  case 0x3F: { c = 0x87; break; } // F5
	  case 0x40: { c = 0x86; break; } // F6
	  case 0x41: { c = 0x8B; break; } // F7
	  case 0x42: { c = 0x8C; break; } // F8*/

	  case 0x13: { if (shift == 1) c = 0x93; else c= 0x13; break; } // home / clr home
	  
	  case 0x2A: { c = 0x00; shift=1; break; }
	  	  

	  
	}
	if(c != 0x00)
	{
	  mem_->write_byte(631,c);
	  mem_->write_byte(198,1);
	}
}
    
void IO::OnKeyUp(uint8_t c)
{
      if(c == 0xaa)
	shift = 0;
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




