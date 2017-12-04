#ifndef EMUDORE_IO_H
#define EMUDORE_IO_H

#include <lib/stdint.h>
#include <lib/vga.h>
#include <c64/cpu.h>
#include <c64/memory.h>

#include <drivers/keyscancodes.h>
#include <drivers/ata.h>
#include <filesystem/fat.h>
#include <drivers/serial.h>
#include <drivers/rtc.h>

//#define _NO_BORDER_

#ifndef _NO_BORDER_
#define VIRT_WIDTH 403
#define VIRT_HEIGHT 284
#else
#define VIRT_WIDTH 320
#define VIRT_HEIGHT 200
#endif

// current milliseconds since epoch
extern uint32_t current_milli;

using namespace myos::hardwarecommunication;
using namespace myos::filesystem;
using namespace myos::drivers;

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
    uint8_t joy1, joy2;

    static const uint8_t kbd[8][8];
  
    unsigned int next_key_event_at_;
    static const int kWait = 18000;
    /* vertical refresh sync */
    void vsync();
        
    Fat32 *fat32_;
    void file_load();
    void file_save();
    void file_open();
    void file_close();
    void file_get();
    
    SerialDriver *serial_;
    RTCDriver *rtc_;

    uint16_t *vscreen_; 		// pointer to the offset of virtual screen.
    uint16_t *pscreen_;
    double scaleWidth;
    double scaleHeight;
    
    uint8_t *vgaMem_;
    uint32_t screen_width_;
    uint32_t screen_height_;
    uint32_t screen_pitch_;
    uint8_t screen_bpp_;
    uint8_t pixel_width_;
    
    uint32_t prev_frame_milli;

public:
    IO();
    ~IO();
    void init_display(uint32_t* vgaMemAddress, uint32_t width, uint32_t height, uint32_t pitch, uint8_t bpp);

    bool emulate();
    void process_events();
    void cpu(Cpu *v){cpu_=v;};
    void memory(Memory *m) {mem_ = m;};
    void fat32(Fat32 *m) { fat32_ = m; };
    void serial(SerialDriver *m) { serial_ = m; };
    void rtc(RTCDriver *m) { rtc_ = m; };
    
    bool step = false;
    uint16_t color_palette[16];
    
    void init_color_palette();
    void init_keyboard();
    void OnKeyDown(uint8_t c);
    void OnKeyUp(uint8_t c);
    uint8_t getJoystick(uint8_t num);
    void SendSerial(uint8_t c);
    
    void type_character(char c);
    inline uint8_t keyboard_matrix_row(int col){return keyboard_matrix_[col];};
    
    inline void put_pixel(int x,int y, int color) {
      uint8_t *pixel = vgaMem_ + y*screen_pitch_ + x*pixel_width_;     
      *pixel = color_palette[color] & 255;              
      *(pixel + 1) = (color_palette[color] >> 8) & 255;
    };
    
    inline void screen_update_pixel(int x, int y, int color) { 
      *(vscreen_ + y * VIRT_WIDTH  + x) = color;
    };
    
    inline void screen_draw_rect(int x, int y, int n, int color) {
      for(int i=1; i <= n ; i++)
	*(vscreen_ + y * VIRT_WIDTH + x + i) = color;
    };
    
    inline void screen_draw_border(int y, int color) {
      for(int i=0; i < cols_ ; i++)
	*(vscreen_ + y * VIRT_WIDTH  + i) = color;
    };
    
    inline void screen_refresh() {

      for(uint16_t cy = 0; cy < screen_height_; cy++)
	for(uint16_t cx = 0; cx < screen_pitch_; cx++)
	{
	  if(cy % 2 == 0) // Scanlines
	  {
	    uint32_t nearestMatch =  (((uint16_t)(cy / scaleHeight) * VIRT_WIDTH) + ((uint16_t)(cx / scaleWidth)));
	    pscreen_[cy * screen_pitch_ + cx] = vscreen_[nearestMatch];
	  }
	}
	
	for(uint32_t x=0;x<screen_pitch_*screen_height_;x+=pixel_width_)
	{
	  uint16_t color = color_palette[*(pscreen_+x)];
	  *(vgaMem_+x) = color & 255;
	  *(vgaMem_+x+1) = (color >> 8) & 255;
	}
      
      sync();
            
      // no scaling
      /*
      for(uint16_t y=0;y<284;y++)
	for(uint16_t x=0;x<403;x++)
	  put_pixel(x,y,vscreen_[y*403+x]);*/
    };
    
    inline void sync()
    {
      uint32_t current = current_milli;
      uint32_t t = current - prev_frame_milli;
      uint32_t ttw = 19 - t; //(Vic::kRefreshRate*1000) - t;
      
      delay(current_milli - current+ttw);
      prev_frame_milli = current_milli;
    }

    inline void delay(uint32_t milliseconds)
    {
      uint32_t startmilli = current_milli;
      while(current_milli < startmilli + milliseconds)  {};
    }
    
    
};

#endif