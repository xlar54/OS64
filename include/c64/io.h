#ifndef EMUDORE_IO_H
#define EMUDORE_IO_H

#include <common/types.h>
#include <drivers/vga.h>
#include <c64/cpu.h>
//#include <c64/util.h>

using namespace myos::drivers;

/**
 * @brief IO devices
 *
 * This class implements Input/Output devices connected to the 
 * Commodore 64 such as the screen and keyboard.
 *
 * Current backend is SDL2.
 */
class IO
{
  private:
    Cpu *cpu_;
    VideoGraphicsArray *vga;
    uint32_t *frame_;
    size_t cols_;
    size_t rows_;
    unsigned int color_palette[16];
    uint8_t keyboard_matrix_[8];
    bool retval_ = true;
    /* keyboard mappings */
uint16_t backbuffer[64000];
    enum kKeyEvent
    {
      kPress,
      kRelease,
    };


    unsigned int next_key_event_at_;
    static const int kWait = 18000;
    /* vertical refresh sync */
    //std::chrono::high_resolution_clock::time_point prev_frame_was_at_;
    void vsync();
  public:
    IO(VideoGraphicsArray *v);
    ~IO();
    bool emulate();
    void process_events();
    void cpu(Cpu *v){cpu_=v;};
    void init_color_palette();
    void init_keyboard();
    void handle_keydown();
    void handle_keyup();
    void type_character(char c);
    inline uint8_t keyboard_matrix_row(int col){return keyboard_matrix_[col];};
    void screen_update_pixel(int x, int y, int color);
    void screen_draw_rect(int x, int y, int n, int color);
    void screen_draw_border(int y, int color);
    void screen_refresh();
};

// inline member functions accesible from other classes /////////////////////

inline void IO::screen_update_pixel(int x, int y, int color)
{
  //frame_[y * cols_  + x] = color_palette[color & 0xf];
  //vga->PutPixel(x,y,0x03);

  //backbuffer[x*320+y] = 0x03;

  unsigned short offset = (y<<8) + (y<<6) + x; //320*y + x;
  *((uint8_t*)(0xA0000 + offset)) = 0x03;
  //backbuffer[offset] =0x03;
};

#endif