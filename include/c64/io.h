#ifndef EMUDORE_IO_H
#define EMUDORE_IO_H

#include <common/types.h>
#include <lib/vga.h>
#include <c64/cpu.h>
#include <c64/memory.h>
#include <c64/monitor.h>

class Monitor;

/**
 * @brief IO devices
 *
 * This class implements Input/Output devices connected to the 
 * Commodore 64 such as the screen and keyboard.
 */
class IO
{
  private:
    Cpu *cpu_;
    Memory *mem_;
    uint32_t *frame_;
    size_t cols_;
    size_t rows_;
    unsigned int color_palette[16];
    uint8_t keyboard_matrix_[8];
    bool retval_ = true;
    /* keyboard mappings */
    enum kKeyEvent
    {
      kPress,
      kRelease,
    };
    uint8_t shift = 0;
    uint8_t mode = 0;

    unsigned int next_key_event_at_;
    static const int kWait = 18000;
    /* vertical refresh sync */
    //std::chrono::high_resolution_clock::time_point prev_frame_was_at_;
    void vsync();
  public:
    IO();
    ~IO();
    Monitor *mon_;
    bool emulate();
    void process_events();
    void cpu(Cpu *v){cpu_=v;};
    void memory(Memory *m) {mem_ = m;};
    void monitor(Monitor *m) {mon_ = m;};
    void init_color_palette();
    void init_keyboard();
    void OnKeyDown(uint8_t c);
    void OnKeyUp(uint8_t c);
    void handle_keydown();
    void handle_keyup();
    void type_character(char c);
    inline uint8_t keyboard_matrix_row(int col){return keyboard_matrix_[col];};
    inline void screen_update_pixel(int x, int y, int color) { vga_put_pixel(x, y,color); };
    inline void screen_draw_rect(int x, int y, int n, uint8_t color) { vga_draw_rect(x, y, n, color); };
    inline void screen_draw_border(int y, int color) { vga_draw_rect(0, y, cols_, color); };
    void screen_refresh();
};

// inline member functions accesible from other classes /////////////////////

//inline void IO::screen_update_pixel(int x, int y, int color)
//{
//  unsigned short offset = (y<<8) + (y<<6) + x;
//  *((uint8_t*)(0xA0000 + offset)) = color;
//  
//};

#endif