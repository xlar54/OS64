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

#ifndef EMUDORE_C64_H
#define EMUDORE_C64_H

#include <functional>

#include <lib/stdint.h>
#include <c64/cpu.h>
#include <c64/memory.h>
#include <c64/cia1.h>
#include <c64/cia2.h>
#include <c64/vic.h>
#include <c64/sid.h>
#include <c64/io.h>
#include <c64/monitor.h>

/**
 * @brief Commodore 64
 * 
 * This class glues together all the different
 * components in a Commodore 64 computer
 */
class C64
{
  private:
    bool isRunning = true;
    
  public: 
    C64();
    ~C64();
    Cpu *cpu_;
    Cia1 *cia1_;
    Cia2 *cia2_;  
    Memory *mem_;
    IO *io_;
    Sid *sid_;
    Vic *vic_;
    Monitor *mon_;
    bool reset = false;
    void start();
    void stop();
   
    Cpu * cpu(){return cpu_;};
    Memory * memory(){return mem_;};
    IO * io(){return io_;};
    
    struct cpuState* getCpuState();
};

#endif
