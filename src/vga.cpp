#include <vga.h>


inline void outb(uint16_t port, uint8_t value) 
{
  //("a" puts value in eax, "dN" puts port in edx or uses 1-byte constant.)
  asm volatile ("outb %0, %1" :: "a" (value), "dN" (port));
}

inline uint8_t inb(uint16_t port)
{
    uint8_t result;
    asm volatile("inb %1, %0" : "=a" (result) : "Nd" (port));
    return result;
}

uint8_t readRegVGA(unsigned short reg, uint8_t idx)
{
   outb(reg, idx);

   return inb(reg + 0x01);
}

void writeRegVGA(uint16_t reg, uint8_t idx, uint8_t val)
{
   outb(reg, idx);
   outb(reg + 1, val);
}

void write_regs(unsigned char *regs)
{
	unsigned i;

/* write MISCELLANEOUS reg */
	outb(VGA_MISC_WRITE, *regs);
	regs++;
/* write SEQUENCER regs */
	for(i = 0; i < VGA_NUM_SEQ_REGS; i++)
	{
		outb(VGA_SEQ_INDEX, i);
		outb(VGA_SEQ_DATA, *regs);
		regs++;
	}
/* unlock CRTC registers */
	outb(VGA_CRTC_INDEX, 0x03);
	outb(VGA_CRTC_DATA, inb(VGA_CRTC_DATA) | 0x80);
	outb(VGA_CRTC_INDEX, 0x11);
	outb(VGA_CRTC_DATA, inb(VGA_CRTC_DATA) & ~0x80);
/* make sure they remain unlocked */
	regs[0x03] |= 0x80;
	regs[0x11] &= ~0x80;
/* write CRTC regs */
	for(i = 0; i < VGA_NUM_CRTC_REGS; i++)
	{
		outb(VGA_CRTC_INDEX, i);
		outb(VGA_CRTC_DATA, *regs);
		regs++;
	}
/* write GRAPHICS CONTROLLER regs */
	for(i = 0; i < VGA_NUM_GC_REGS; i++)
	{
		outb(VGA_GC_INDEX, i);
		outb(VGA_GC_DATA, *regs);
		regs++;
	}
/* write ATTRIBUTE CONTROLLER regs */
	for(i = 0; i < VGA_NUM_AC_REGS; i++)
	{
		(void)inb(VGA_INSTAT_READ);
		outb(VGA_AC_INDEX, i);
		outb(VGA_AC_WRITE, *regs);
		regs++;
	}
/* lock 16-color palette and unblank display */
	(void)inb(VGA_INSTAT_READ);
	outb(VGA_AC_INDEX, 0x20);
}

void read_regs(unsigned char *regs)
{
	unsigned i;

/* read MISCELLANEOUS reg */
	*regs = inb(VGA_MISC_READ);
	regs++;
/* read SEQUENCER regs */
	for(i = 0; i < VGA_NUM_SEQ_REGS; i++)
	{
		outb(VGA_SEQ_INDEX, i);
		*regs = inb(VGA_SEQ_DATA);
		regs++;
	}
/* read CRTC regs */
	for(i = 0; i < VGA_NUM_CRTC_REGS; i++)
	{
		outb(VGA_CRTC_INDEX, i);
		*regs = inb(VGA_CRTC_DATA);
		regs++;
	}
/* read GRAPHICS CONTROLLER regs */
	for(i = 0; i < VGA_NUM_GC_REGS; i++)
	{
		outb(VGA_GC_INDEX, i);
		*regs = inb(VGA_GC_DATA);
		regs++;
	}
/* read ATTRIBUTE CONTROLLER regs */
	for(i = 0; i < VGA_NUM_AC_REGS; i++)
	{
		(void)inb(VGA_INSTAT_READ);
		outb(VGA_AC_INDEX, i);
		*regs = inb(VGA_AC_READ);
		regs++;
	}
/* lock 16-color palette and unblank display */
	(void)inb(VGA_INSTAT_READ);
	outb(VGA_AC_INDEX, 0x20);
}

static unsigned get_fb_seg(void)
{
	unsigned seg;

	outb(VGA_GC_INDEX, 6);
	seg = inb(VGA_GC_DATA);
	seg >>= 2;
	seg &= 3;
	switch(seg)
	{
	case 0:
	case 1:
		seg = 0xA000;
		break;
	case 2:
		seg = 0xB000;
		break;
	case 3:
		seg = 0xB800;
		break;
	}
	return seg;
}


static void write_font(unsigned char *buf, unsigned font_height)
{
	unsigned char seq2, seq4, gc4, gc5, gc6;
	unsigned i;

/* save registers
set_plane() modifies GC 4 and SEQ 2, so save them as well */
	outb(VGA_SEQ_INDEX, 2);
	seq2 = inb(VGA_SEQ_DATA);

	outb(VGA_SEQ_INDEX, 4);
	seq4 = inb(VGA_SEQ_DATA);
/* turn off even-odd addressing (set flat addressing)
assume: chain-4 addressing already off */
	outb(VGA_SEQ_DATA, seq4 | 0x04);

	outb(VGA_GC_INDEX, 4);
	gc4 = inb(VGA_GC_DATA);

	outb(VGA_GC_INDEX, 5);
	gc5 = inb(VGA_GC_DATA);
/* turn off even-odd addressing */
	outb(VGA_GC_DATA, gc5 & ~0x10);

	outb(VGA_GC_INDEX, 6);
	gc6 = inb(VGA_GC_DATA);
/* turn off even-odd addressing */
	outb(VGA_GC_DATA, gc6 & ~0x02);
/* write font to plane P4 */
	setPlaneVGA(2);
/* write font 0 */
	for(i = 0; i < 256; i++)
	{
		vmemwr(16384u * 0 + i * 32, buf, font_height);
		buf += font_height;
	}
#if 0
/* write font 1 */
	for(i = 0; i < 256; i++)
	{
		vmemwr(16384u * 1 + i * 32, buf, font_height);
		buf += font_height;
	}
#endif
/* restore registers */
	outb(VGA_SEQ_INDEX, 2);
	outb(VGA_SEQ_DATA, seq2);
	outb(VGA_SEQ_INDEX, 4);
	outb(VGA_SEQ_DATA, seq4);
	outb(VGA_GC_INDEX, 4);
	outb(VGA_GC_DATA, gc4);
	outb(VGA_GC_INDEX, 5);
	outb(VGA_GC_DATA, gc5);
	outb(VGA_GC_INDEX, 6);
	outb(VGA_GC_DATA, gc6);
}

void setPlaneVGA(unsigned p)
{
	unsigned char pmask;

	p &= 3;
	pmask = 1 << p;
/* set read plane */
	outb(VGA_GFX_REG, 4);
	outb(VGA_GFX_REG + 1, p);  // data port
/* set write plane */
	outb(VGA_SEQ_REG, 2);
	outb(VGA_SEQ_REG + 1, pmask); // data port
}

void setTextModeVGA(int hi_res)
{
	unsigned rows, cols, ht, i;

	if(hi_res)
	{
		write_regs(g_90x60_text);
		cols = 90;
		rows = 60;
		ht = 8;
	}
	else
	{
		write_regs(g_80x25_text);
		cols = 80;
		rows = 25;
		ht = 16;
	}
/* set font */
	if(ht >= 16)
		write_font(g_8x16_font, 16);
	else
		write_font(g_8x8_font, 8);
/* tell the BIOS what we've done, so BIOS text output works OK */
	pokew(0x40, 0x4A, cols);	/* columns on screen */
	pokew(0x40, 0x4C, cols * rows * 2); /* framebuffer size */
	pokew(0x40, 0x50, 0);		/* cursor pos'n */
	pokeb(0x40, 0x60, ht - 1);	/* cursor shape */
	pokeb(0x40, 0x61, ht - 2);
	pokeb(0x40, 0x84, rows - 1);	/* rows on screen - 1 */
	pokeb(0x40, 0x85, ht);		/* char height */
/* set white-on-black attributes for all text */
	for(i = 0; i < cols * rows; i++)
		pokeb(0xB800, i * 2 + 1, 7);
}

void setFontVGA(const uint8_t * buffer, int h)
{
   unsigned char seq2, seq4, gfx6;
   int i, j;
   unsigned char * mem;

   seq2 = readRegVGA(VGA_SEQ_REG, VGA_SEQ_I_MAPMASK);
   writeRegVGA(VGA_SEQ_REG, VGA_SEQ_I_MAPMASK, 0x04);

   seq4 = readRegVGA(VGA_SEQ_REG, VGA_SEQ_I_MEMMODE);
   writeRegVGA(VGA_SEQ_REG, VGA_SEQ_I_MEMMODE, 0x06);

   writeRegVGA(VGA_SEQ_REG, VGA_SEQ_I_CHARMAP, 0x00);

   gfx6 = readRegVGA(VGA_GFX_REG, VGA_GFX_I_MISC);
   writeRegVGA(VGA_SEQ_REG, VGA_GFX_I_MISC, 0x00);

   mem = (unsigned char *) 0xB8000;

   for (i = 0; i < 256; i++)
   {
      for (j = 0; j < h; j++)
         mem[j] = buffer[h * i + j];

      mem += 32;
   }

   writeRegVGA(VGA_GFX_REG, VGA_GFX_I_MISC, gfx6);
   writeRegVGA(VGA_SEQ_REG, VGA_SEQ_I_MEMMODE, seq4);
   writeRegVGA(VGA_SEQ_REG, VGA_SEQ_I_MAPMASK, seq2);

   writeRegVGA(VGA_GFX_REG, VGA_GFX_I_MODE, 0x10);
   writeRegVGA(VGA_GFX_REG, VGA_GFX_I_BITMASK, 0xFF);
}





void* memcpy(uint8_t* destination, const uint8_t* source, size_t num)
{
	int i;
	unsigned char* d = destination;
	unsigned const char* s = source;
	for (i = 0; i < num; i++) {
		d[i] = s[i];
	}
	return destination;
}


void printf(char* str)
{
    uint16_t* vm = (uint16_t*)0xb8000;
    static uint8_t x=0,y=0;

    for(int i = 0; str[i] != '\0'; ++i)
    {
        switch(str[i])
        {
            case '\n':
                x = 0;
                y++;
                break;
            default:
                vm[80*y+x] = (vm[80*y+x] & 0xFF00) | str[i];
                x++;
                break;
        }

        if(x >= 80)
        {
            x = 0;
            y++;
        }

        if(y >= 25)
        {
            for(y = 0; y < 25; y++)
                for(x = 0; x < 80; x++)
                    vm[80*y+x] = (vm[80*y+x] & 0xFF00) | ' ';
            x = 0;
            y = 0;
        }
    }
}

void clear()
{
  int x,y;
  for(y = 0; y < 25; y++)
    for(x = 0; x < 80; x++)
      VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0xFF00) | ' ';
}

void printfHex(uint8_t key)
{
    char* foo = "00";
    char* hex = "0123456789ABCDEF";
    foo[0] = hex[(key >> 4) & 0xF];
    foo[1] = hex[key & 0xF];
    printf(foo);
}
void printfHex16(uint16_t key)
{
    printfHex((key >> 8) & 0xFF);
    printfHex( key & 0xFF);
}
void printfHex32(uint32_t key)
{
    printfHex((key >> 24) & 0xFF);
    printfHex((key >> 16) & 0xFF);
    printfHex((key >> 8) & 0xFF);
    printfHex( key & 0xFF);
}