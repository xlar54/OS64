#ifndef EMUDORE_MONITOR_H
#define EMUDORE_MONITOR_H 

#include <lib/stdint.h>
#include <lib/string.h>
#include <c64/io.h>
#include <c64/cpu.h>
#include <c64/memory.h>
#include <drivers/ata.h>
#include <filesystem/fat.h>

using namespace myos::hardwarecommunication;
using namespace myos::filesystem;

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
    Fat32 *fat32_;
  public:
    Monitor();
    ~Monitor();
    void cpu(Cpu *c){cpu_ = c;};
    void mem(Memory *m){mem_ = m;};
    void vic(Vic *v){vic_ = v;};
    void cia1(Cia1 *v){cia1_ = v;};
    void cia2(Cia2 *v){cia2_ = v;};
    void fat32(Fat32 *m) { fat32_ = m; };
    void Run();
    void OnKeyDown(uint8_t c);
};


#endif