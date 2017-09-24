#include <vga.h>

// Basic port Functions

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

// Graphic mode functions

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
		outb(VGA_SEQ_REG, i);
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
		outb(VGA_AC_INDEX, *regs);
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
		outb(VGA_SEQ_REG, i);
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

void vga_clear(int color)
{
  int x,y;
  for(x=0; x<320;x++)
    for(y=0; y<200;y++)
      vga_put_pixel(x,y,color);
}

void vga_put_pixel(int x, int y, int color)
{
  unsigned short offset = (y<<8) + (y<<6) + x;
  *((uint8_t*)(0xA0000 + offset)) = color;
  
};

void vga_draw_rect(int x, int y, int n, uint8_t color)
{

  for(int i=0; i < n ; i++)
  {
    unsigned short offset = (y<<8) + (y<<6) + (x+i);
    *((uint8_t*)(0xA0000 + offset)) = color;
  }
}

int abs (int n) 
{
    const int ret[2] = { n, -n };
    return ret [n<0];
}

int sgn(int v) 
{
  if (v < 0) return -1;
  if (v > 0) return 1;
  return 0;
}

void vga_draw_line(int x1, int y1, int x2, int y2, uint8_t color)
{
  int i,dx,dy,sdx,sdy,dxabs,dyabs,x,y,px,py;

  dx=x2-x1;      /* the horizontal distance of the line */
  dy=y2-y1;      /* the vertical distance of the line */
  dxabs=abs(dx);
  dyabs=abs(dy);
  sdx=sgn(dx);
  sdy=sgn(dy);
  x=dyabs>>1;
  y=dxabs>>1;
  px=x1;
  py=y1;

  vga_put_pixel(px,py,color);

  if (dxabs>=dyabs) /* the line is more horizontal than vertical */
  {
    for(i=0;i<dxabs;i++)
    {
      y+=dyabs;
      if (y>=dxabs)
      {
        y-=dxabs;
        py+=sdy;
      }
      px+=sdx;
      vga_put_pixel(px,py,color);
    }
  }
  else /* the line is more vertical than horizontal */
  {
    for(i=0;i<dyabs;i++)
    {
      x+=dxabs;
      if (x>=dyabs)
      {
        x-=dyabs;
        px+=sdx;
      }
      py+=sdy;
      vga_put_pixel(px,py,color);
    }
  }
}

//  Text Mode Functions

static uint8_t textCols=80, textRows=25;
static uint8_t cursorX=0,cursorY=0;
static uint8_t backColor=0, foreColor=15;
static uint8_t cursorOn=0;

void setTextModeVGA(int hi_res)
{
	unsigned rows, cols, ht, i;

	if(hi_res)
	{
	  write_regs(g_90x60_text);
	  textCols = 90;
	  textRows = 60;
	  ht = 8;
	}
	else
	{
	  write_regs(g_80x25_text);
	  textCols = 80;
	  textRows = 25;
	  ht = 16;
	}
	
	// set font
	if(ht == 16)
	  setFontVGA(g_8x16_font, 16);
	else
	  setFontVGA(g_8x8_font, 8);

	// Standard PC colors	18 bit
	setColorVGA(0,0,0,0);
	setColorVGA(1,0,0,42);
	setColorVGA(2,0,42,0);
	setColorVGA(3,0,42,42);
	setColorVGA(4,42,0,0);
	setColorVGA(5,42,0,42);
	setColorVGA(6,42,42,0);
	setColorVGA(7,52,52,52);
	setColorVGA(8,42,42,42);
	setColorVGA(9,0,0,63);
	setColorVGA(10,0,63,0);
	setColorVGA(11,0,63,63);
	setColorVGA(12,63,0,0);
	setColorVGA(13,63,0,63);
	setColorVGA(14,63,63,0);
	setColorVGA(15,63,63,63);
	
	clear();
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

   gfx6 = readRegVGA(VGA_GC_INDEX, VGA_GFX_I_MISC);
   writeRegVGA(VGA_SEQ_REG, VGA_GFX_I_MISC, 0x00);

   mem = (uint8_t *) 0xB8000;
   
   for (i = 0; i < 256; i++)
   {
      for (j = 0; j < h; j++)
         mem[j] = buffer[h * i + j];
      mem += 32;
   }

   writeRegVGA(VGA_GC_INDEX, VGA_GFX_I_MISC, gfx6);
   writeRegVGA(VGA_SEQ_REG, VGA_SEQ_I_MEMMODE, seq4);
   writeRegVGA(VGA_SEQ_REG, VGA_SEQ_I_MAPMASK, seq2);

   writeRegVGA(VGA_GC_INDEX, VGA_GFX_I_MODE, 0x10);
   writeRegVGA(VGA_GC_INDEX, VGA_GFX_I_BITMASK, 0xFF);
}

void* memcpy(uint16_t* destination, const uint16_t* source, size_t num)
{
	int i;
	uint16_t* d = destination;
	const uint16_t* s = source;
	for (i = 0; i < num; i++) {
		d[i] = s[i];
	}
	return destination;
}

void setColorVGA(int colorIndex, int red, int green, int blue)
{
  // 18-bit color.  Each color ranges 0-63
  outb(0x3C8, colorIndex);
  outb(0x3C9, red);
  outb(0x3C9, green);
  outb(0x3C9, blue);
}

void putc(unsigned char c, int x, int y)
{
     uint16_t attrib = (backColor << 4) | (foreColor & 0x0F);
     volatile uint16_t * where;
     where = (volatile uint16_t *)0xB8000 + (y * textCols + x) ;
     *where = c | (attrib << 8);
}

void printf(char* str)
{
    uint16_t* vm = (uint16_t*)0xb8000;
    
    for(int i = 0; str[i] != '\0'; ++i)
    {
        switch(str[i])
        {
	  // backspace
	  case 0x0E:
	    if(cursorX > 0)
	    {
	      cursorX--;
	      putc(' ',cursorX,cursorY); 
	      vga_update_cursor();
	      return;
	    }
	    else if(cursorX == 0 && cursorY>0)
	    {
	      cursorX = textCols-1;
	      cursorY--;
	      putc(' ',cursorX,cursorY); 
	      vga_update_cursor();
	      return;
	    }
	    else if(cursorX == 0 && cursorY==0)
	      return;
	    break;
	  case 0x0F:
	    cursorX = (cursorX + 8) & ~(8 - 1);
	    vga_update_cursor();
	    break;
	  case '\n':
	    cursorX = 0;
	    cursorY++;
	    break;
	  default:
	    putc(str[i],cursorX,cursorY);  
	    cursorX++;
	    break;
        }

        if(cursorX >= textCols)
        {
            cursorX = 0;
            cursorY++;
        }

        if(cursorY >= textRows-1)
        {
	  uint16_t blank = 0x20 | (0x0F << 8);
	  uint8_t tempx=0;
	  
	  int temp = cursorY - textRows + 1;
	  memcpy (vm, vm + temp * textCols, (textRows - temp) * textCols * 2);

	  for(; tempx<textCols;tempx++);
	    putc(' ',tempx,textRows);
	  
	  cursorY = textRows-1;
        }
        
        vga_update_cursor();
    }
}

void vga_update_cursor()
{
  if (cursorOn == 1)
  {
    unsigned temp;
    temp = cursorY * textCols + cursorX;
    outb(0x3D4, 14);
    outb(0x3D5, temp >> 8);
    outb(0x3D4, 15);
    outb(0x3D5, temp);
  }
  else
  {
    outb(0x3D4, 0x0A);
    outb(0x3D5, 0x20);
  }
}

void clear()
{
  int x;
  int y;
    
  for(y = 0; y < textRows*2; y++)
    for(x = 0; x < textCols; x++)
      putc(' ',x,y);
    
  cursorX = 0; cursorY = 0;
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