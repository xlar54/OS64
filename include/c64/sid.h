#ifndef EMUDORE_SID_H
#define EMUDORE_SID_H


#include <lib/stdint.h>
#include <c64/io.h>
#include <c64/cpu.h>
#include <c64/memory.h>
#include <drivers/speaker.h>

class Sid
{
  private:

  myos::drivers::SpeakerDriver *speaker_;
  uint8_t volume;
  uint8_t freqLo;
  uint8_t freqHi;
  uint32_t frequency;
    
  public:
    Sid();
    ~Sid();
    void speaker(myos::drivers::SpeakerDriver *m) { speaker_ = m; };
    
    void write_register(uint8_t r, uint8_t v);
    uint32_t getFrequency(uint8_t hi, uint8_t lo);
    void play();
};

#endif