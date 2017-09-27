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
//#include <c64/sid.h>
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
    Cpu *cpu_;
    Cia1 *cia1_;
    Cia2 *cia2_;
    Vic *vic_;
    Sid *sid_;
    
    //std::function<bool()> callback_;

  public:
    Memory *mem_;
    Monitor *mon_;
    IO *io_;
    C64();
    ~C64();
    void start();
    //void emscripten_loop();
    //void callback(std::function<bool()> cb){callback_ = cb;};
    Cpu * cpu(){return cpu_;};
    Memory * memory(){return mem_;};
    IO * io(){return io_;};
    Monitor* monitor(){return mon_;};
};

#endif
