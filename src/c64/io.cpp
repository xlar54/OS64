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

using namespace myos::drivers;

// clas ctor and dtor //////////////////////////////////////////////////////////

IO::IO(VideoGraphicsArray *v)
{
  vga = v;
  
  vga->SetMode(320,200,8);
  vga->foregroundColor = 0xFFFFFF;
  vga->backgroundColor = 0x0000A8;
  vga->Clear();
  
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

void IO::screen_draw_rect(int x, int y, int n, int color)
{
/*  for(int i=0; i < n ; i++)
  {
    vga->PutPixel(x+i,y,0x01);
  }*/

  for(int i=0; i < n ; i++)
  {
    unsigned short offset = (y<<8) + (y<<6) + (x+i);
    *((uint8_t*)(0xA0000 + offset)) = 0x01;
  }
}
 
void IO::screen_draw_border(int y, int color)
{
  screen_draw_rect(0,y,cols_,0x03);
}


void IO::screen_refresh()
{
//  for(int x=0; x<64000;x++)
//    *((uint_t*)(0xA0000 + x)) = backbuffer[x];

//  uint16_t offset = 0;
//  while(offset < 64000){
//    *((uint16_t*)(0xA0000 + offset)) = *(backbuffer + offset++);
//  }
  
  //  for(int x=0; x<320;x++)
//    for(int y=0; y<200; y++)
//      vga->PutPixel(x,y, backbuffer[x*320+y]);
  
  /*SDL_UpdateTexture(texture_, NULL, frame_, cols_ * sizeof(uint32_t));
  SDL_RenderClear(renderer_);
  SDL_RenderCopy(renderer_,texture_, NULL, NULL);
  SDL_RenderPresent(renderer_);
  /* process SDL events once every frame */
  //process_events();
  /* perform vertical refresh sync */
  vsync();
}


void IO::vsync()
{

}




