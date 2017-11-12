#ifndef EMUDORE_IO_H
#define EMUDORE_IO_H

#include <lib/stdint.h>
#include <lib/vga.h>
#include <c64/cpu.h>
#include <c64/memory.h>
#include <c64/monitor.h>

#include <drivers/ata.h>
#include <filesystem/fat.h>

using namespace myos::hardwarecommunication;
using namespace myos::filesystem;

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
    bool retval_;
    /* keyboard mappings */
    enum kKeyEvent
    {
      kPress,
      kRelease,
    };
    uint8_t shift;
    uint8_t mode;

    static const uint8_t kbd[8][8];
  
    unsigned int next_key_event_at_;
    static const int kWait = 18000;
    /* vertical refresh sync */
    //std::chrono::high_resolution_clock::time_point prev_frame_was_at_;
    void vsync();
        
    Fat32 *fat32_;
    void file_load();
    void file_save();
    void file_open();
    void file_close();
    void file_get();

    uint8_t *vgaMem;		// pointer to the offset of VGA memory
    uint8_t vscreen[320*200];	// pointer to the offset of virtual screen.
        
  public:
    IO();
    ~IO();
    Monitor *mon_;
    bool emulate();
    void process_events();
    void cpu(Cpu *v){cpu_=v;};
    void memory(Memory *m) {mem_ = m;};
    void monitor(Monitor *m) {mon_ = m;};
    void fat32(Fat32 *m) { fat32_ = m; };
    
    
    void init_color_palette();
    void init_keyboard();
    void OnKeyDown(uint8_t c);
    void OnKeyUp(uint8_t c);

    void type_character(char c);
    inline uint8_t keyboard_matrix_row(int col){return keyboard_matrix_[col];};
    inline void screen_draw_rect(int x, int y, int n, uint8_t color) {
      for(int i=0; i < n ; i++)
	*(vscreen + 320 * y + x + i) = color;
    };
    inline void screen_draw_border(int y, int color) { vga_draw_rect(0, y, cols_, color); };
    inline void screen_refresh() {
      //memcpy(vgaMem,vscreen,size_t(320*200));
      for(int x=0;x<64000;x+=4)
	*(uint32_t*)(vgaMem+x) = *(uint32_t*)(vscreen+x);
    }
    inline void screen_update_pixel(int x, int y, int color) { 
      *(vscreen + 320 * y + x) = (uint8_t) color;
    };
};

#endif