#ifndef EMUDORE_MONITOR_H
#define EMUDORE_MONITOR_H 

#include <lib/stdint.h>
#include <lib/string.h>
#include <c64/io.h>
#include <c64/cpu.h>
#include <c64/memory.h>

/* forward declarations */

class Cpu;
class Vic;
class Cia1;
class Cia2;
class Sid;
class Memory;

class Monitor
{
  private:
    Cpu *cpu_;
    Vic *vic_;
    Cia1 *cia1_;
    Cia2 *cia2_;
    Sid *sid_;
    Memory *mem_;
    char buf[80];
    uint8_t bufPtr;
    void prompt();
    void help();
    void process_cmd();
  public:
    Monitor();
    ~Monitor();
    void cpu(Cpu *c){cpu_ = c;};
    void mem(Memory *m){mem_ = m;};
    void vic(Vic *v){vic_ = v;};
    void cia1(Cia1 *v){cia1_ = v;};
    void cia2(Cia2 *v){cia2_ = v;};
    void Run();
    void OnKeyDown(uint8_t c);
};


#endif