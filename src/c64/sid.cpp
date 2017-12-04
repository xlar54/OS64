#include <c64/sid.h>

Sid::Sid()
{

}

Sid::~Sid()
{

}

void Sid::write_register(uint8_t r, uint8_t v)
{
  switch(r)
  {
    case 0x00:
    {
      freqLo = v;
      play();
      break;
    }
    case 0x01:
    {
      freqHi = v;
      play();
      break;
    }
    case 0x07:
    {
      freqLo = v;
      play();
      break;
    }
    case 0x08:
    {
      freqHi = v;
      play();
      break;
    }
    case 0x0e:
    {
      freqLo = v;
      play();
      break;
    }
    case 0x0f:
    {
      freqHi = v;
      play();
      break;
    }
    case 0x18:	// volume
    {
      volume = v;
      play();
      break;
    }
  }
}

void Sid::play()
{
  if(volume == 0)
    speaker_->Nosound();
  else
  {
    frequency = getFrequency(freqHi, freqLo);
    
    if(frequency>0)
      speaker_->Sound(frequency/1000);
  }
}

uint32_t Sid::getFrequency(uint8_t hi, uint8_t lo)
{
  uint16_t v = hi * 256 + lo;
  
  if (v== 0x0112) return 16351;
if (v== 0x0123) return 17324;
if (v== 0x0134) return 18354;
if (v== 0x0146) return 19445;
if (v== 0x015A) return 20601;
if (v== 0x016E) return 21827;
if (v== 0x0184) return 23124;
if (v== 0x019B) return 24499;
if (v== 0x01B3) return 25956;
if (v== 0x01CD) return 27500;
if (v== 0x01E9) return 29135;
if (v== 0x0206) return 30868;
if (v== 0x0225) return 32703.;
if (v== 0x0245) return 34648;
if (v== 0x0268) return 36708;
if (v== 0x028C) return 38891;
if (v== 0x02B3) return 41203;
if (v== 0x02DC) return 43654;
if (v== 0x0308) return 46249;
if (v== 0x0336) return 48999;
if (v== 0x0367) return 51913;
if (v== 0x039B) return 55000;
if (v== 0x03D2) return 58270;
if (v== 0x040C) return 61735;
if (v== 0x0449) return 65406;
if (v== 0x048B) return 69296;
if (v== 0x04D0) return 73416;
if (v== 0x0519) return 77782;
if (v== 0x0567) return 82407;
if (v== 0x05B9) return 87307;
if (v== 0x0610) return 92499;
if (v== 0x066C) return 97999;
if (v== 0x06CE) return 103826;
if (v== 0x0735) return 110000;
if (v== 0x07A3) return 116541;
if (v== 0x0817) return 123471;
if (v== 0x0893) return 130813;
if (v== 0x0915) return 138591;
if (v== 0x099F) return 146832;
if (v== 0x0A3C) return 155563;
if (v== 0x0ACD) return 164814;
if (v== 0x0B72) return 174614;
if (v== 0x0C20) return 184997;
if (v== 0x0CD8) return 195998;
if (v== 0x0D9C) return 207652;
if (v== 0x0E6B) return 220000;
if (v== 0x0F46) return 233082;
if (v== 0x102F) return 246942;
if (v== 0x1125) return 261626;
if (v== 0x122A) return 277183;
if (v== 0x133F) return 293665;
if (v== 0x1464) return 311127;
if (v== 0x159A) return 329628;
if (v== 0x16E3) return 349228;
if (v== 0x183F) return 369994;
if (v== 0x19B1) return 391995;
if (v== 0x1B38) return 415305;
if (v== 0x1CD6) return 440000;
if (v== 0x1E8D) return 466164;
if (v== 0x205E) return 493883;
if (v== 0x224B) return 523251;
if (v== 0x2455) return 554365;
if (v== 0x267E) return 587330;
if (v== 0x28C8) return 622254;
if (v== 0x2B34) return 659255;
if (v== 0x2DC6) return 698456;
if (v== 0x307F) return 739989;
if (v== 0x3461) return 783991;
if (v== 0x366F) return 830609;
if (v== 0x39AC) return 880000;
if (v== 0x3D7E) return 932328;
if (v== 0x40BC) return 987767;
if (v== 0x4495) return 1046502;
if (v== 0x48A9) return 1108731;
if (v== 0x4CFC) return 1174659;
if (v== 0x51A1) return 1244508;
if (v== 0x5669) return 1318510;
if (v== 0x5B8C) return 1396913;
if (v== 0x60FE) return 1479978;
if (v== 0x66C2) return 1567982;
if (v== 0x6CDF) return 1661219;
if (v== 0x7358) return 1760000;
if (v== 0x7A34) return 1864655;
if (v== 0x8178) return 1975533;
if (v== 0x892B) return 2093005;
if (v== 0x9153) return 2217461;
if (v== 0x99F7) return 2349318;
if (v== 0xA31F) return 2489016;
if (v== 0xACD2) return 2637021;
if (v== 0xB719) return 2793826;
if (v== 0xC1FC) return 2959955;
if (v== 0xCD85) return 3135964;
if (v== 0xD9BD) return 3322438;
if (v== 0xE6B0) return 3520000;
if (v== 0xF467) return 3729310;
  
return 0;
}
