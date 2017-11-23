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
#include <c64/c64.h>

C64::C64()
{
  /* create chips */
  io_   = new IO();
  cpu_  = new Cpu();
  mem_  = new Memory();
  cia1_ = new Cia1();
  cia2_ = new Cia2();
  vic_  = new Vic();
  sid_  = new Sid();
  
  mon_ = new Monitor();
  mon_->cpu(cpu_);
  mon_->mem(mem_);

  /* init cpu */
  cpu_->memory(mem_);
  cpu_->reset();
  /* init vic-ii */
  vic_->memory(mem_);
  vic_->cpu(cpu_);
  vic_->io(io_);
  /* init cia1 */
  cia1_->cpu(cpu_);
  cia1_->io(io_);
  /* init cia2 */
  cia2_->cpu(cpu_);
  /* init io */
  io_->cpu(cpu_);
  io_->memory(mem_);
  io_->monitor(mon_);
  /* DMA */
  mem_->vic(vic_);
  mem_->cia1(cia1_);
  mem_->cia2(cia2_);
  mem_->sid(sid_);
 /* r2 support */
  
}

C64::~C64()
{
  delete cpu_;
  delete mem_;
  delete cia1_;
  delete cia2_;
  delete vic_;
  delete sid_;
  delete io_;

}

void C64::start()
{
  /* main emulator loop */
  while(true)
  {
    /* CIA1 */
    if(!cia1_->emulate())
      break;
    /* CIA2 */
    if(!cia2_->emulate())
      break;
    /* CPU */
    if(!cpu_->emulate(io_->step))
      break;
    /* VIC-II */
    if(!vic_->emulate())
      break;
    /* IO */
    if(!io_->emulate())
      break;
  }
}




 

