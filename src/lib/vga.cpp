#include <lib/vga.h>

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

void vga_write_regs(unsigned char *regs)
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

void vga_read_regs(unsigned char *regs)
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

uint8_t vga_set_mode(uint32_t width, uint32_t height, uint8_t bpp) 
{
  if (!vga_supports_mode(width, height, bpp)) 
	  return 0;

  if (width == 320 && height == 200 && bpp == 8) vga_write_regs(g_320x200x256);
  else if (width == 320 && height == 240 && bpp == 8) vga_write_regs(g_320x200x256_modex);
  else if (width == 640 && height == 480 && bpp == 4) vga_write_regs(vga_mode_12h);
  else if (width == 80 && height == 25 && bpp == 16) vga_set_textmode(0);
  else if (width == 90 && height == 60 && bpp == 16) vga_set_textmode(1);
  
  //#if VGA_MAX_BPP == 16
  //if (width < 320)
//	  vga_framebuffer_segment = (uint16_t*) vga_get_framebuffer_segment();
  //else
	  vga_framebuffer_segment = (uint8_t*) vga_get_framebuffer_segment();
  //#endif

  vga_mode_width = width;
  vga_mode_height = height;
  vga_mode_bpp = bpp;

  return 1;
	
}

uint8_t* vga_get_framebuffer_segment(void) 
{
  outb(VGA_GC_INDEX, 0x06);

  switch (inb(VGA_GC_DATA) & (3 << 2)) {
	  case 0 << 2: return (uint8_t*) 0x00000;
	  case 1 << 2: return (uint8_t*) 0xA0000;
	  case 2 << 2: return (uint8_t*) 0xB0000;
	  case 3 << 2:
	  default: return (uint8_t*) 0xB8000;
	  
  }
	
}

uint8_t vga_supports_mode(uint32_t width, uint32_t height, uint8_t bpp) 
{
  return \
    (width == 320 && height == 200 && bpp == 8) || \
    (width == 320 && height == 240 && bpp == 8) || \
    (width == 640 && height == 480 && bpp == 4) || \
    (width == 80 && height == 25 && bpp == 16) || \
    (width == 90 && height == 60 && bpp == 16);
	
}

void vga_gfx_clear(int color)
{
  int x,y;
  for(x=0; x<vga_mode_width;x++)
    for(y=0; y<vga_mode_height;y++)
      vga_put_pixel(x,y,color);
}

void vga_put_pixel(int x, int y, uint8_t color)
{
  unsigned short offset = (y<<8) + (y<<6) + x;
  
  //*(vga_framebuffer_segment + offset) = color;
  uint8_t* address = gfxVideoRAM + vga_mode_width * y + x;
  *address = (uint8_t) color;
};

void vga_draw_rect(int x, int y, int n, uint8_t color)
{

  for(int i=0; i < n ; i++)
  {  
    uint8_t* address = gfxVideoRAM + vga_mode_width * y + x + i;
    *address = (uint8_t) color;
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

void vga_set_textmode(int hi_res)
{
  unsigned rows, cols, ht, i;

  if(hi_res)
  {
    vga_write_regs(g_90x60_text);
    textCols = 90;
    textRows = 60;
    ht = 8;
  }
  else
  {
    vga_write_regs(g_80x25_text);
    textCols = 80;
    textRows = 25;
    ht = 16;
  }

  // set font
  if(ht == 16)
    vga_set_textfont(g_8x16_font, 16);
  else
    vga_set_textfont(g_8x8_font, 8);

  // Standard PC colors	18 bit
  vga_set_color(0,0,0,0);
  vga_set_color(1,0,0,42);
  vga_set_color(2,0,42,0);
  vga_set_color(3,0,42,42);
  vga_set_color(4,42,0,0);
  vga_set_color(5,42,0,42);
  vga_set_color(6,42,42,0);
  vga_set_color(7,52,52,52);
  vga_set_color(8,42,42,42);
  vga_set_color(9,0,0,63);
  vga_set_color(10,0,63,0);
  vga_set_color(11,0,63,63);
  vga_set_color(12,63,0,0);
  vga_set_color(13,63,0,63);
  vga_set_color(14,63,63,0);
  vga_set_color(15,63,63,63);

  vga_textmode_clear();
}

void vga_set_textfont(const uint8_t * buffer, int h)
{
   unsigned char seq2, seq4, gfx6;
   int i, j;
   uint8_t* mem;

   seq2 = readRegVGA(VGA_SEQ_REG, VGA_SEQ_I_MAPMASK);
   writeRegVGA(VGA_SEQ_REG, VGA_SEQ_I_MAPMASK, 0x04);

   seq4 = readRegVGA(VGA_SEQ_REG, VGA_SEQ_I_MEMMODE);
   writeRegVGA(VGA_SEQ_REG, VGA_SEQ_I_MEMMODE, 0x06);

   writeRegVGA(VGA_SEQ_REG, VGA_SEQ_I_CHARMAP, 0x00);

   gfx6 = readRegVGA(VGA_GC_INDEX, VGA_GFX_I_MISC);
   writeRegVGA(VGA_SEQ_REG, VGA_GFX_I_MISC, 0x00);

   mem = (uint8_t *) txtVideoRAM;
   
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

void vga_set_color(int colorIndex, int red, int green, int blue)
{
  // 18-bit color.  Each color ranges 0-63
  outb(0x3C8, colorIndex);
  outb(0x3C9, red);
  outb(0x3C9, green);
  outb(0x3C9, blue);
}

void vga_scroll()
{
  uint8_t attributeByte = ( backColor /*black*/ << 4 ) | ( foreColor /*white*/ & 0x0F );
  uint16_t blank = 0x20 /* space */ | ( attributeByte << 8 );

  if(cursorY >= textRows)
  {
    int it;

    for ( it = 0*textCols; it < (textRows-1)*textCols; it++ ) {
      txtVideoRAM[it] = txtVideoRAM[it+textCols];
    }

    // The last line should now be blank. Do this by writing 80 spaces to it.
    for ( it = (textRows-1)*textCols; it < textRows*textCols; it++ ) {
      txtVideoRAM[it] = blank;
    }

    // The cursor should now be on the last line.
    cursorY = textRows-1;
  }
}

void vga_update_cursor()
{
  //if (vga_cursorOn == 1)
  //{
    uint16_t temp;
    temp = cursorY * textCols + cursorX;
    outb(0x3D4, 14);		// Tell the VGA board we are setting the high cursor byte.
    outb(0x3D5, temp >> 8);	// Send the high cursor byte.
    outb(0x3D4, 15);		// Tell the VGA board we are setting the low cursor byte.
    outb(0x3D5, temp);		// Send the low cursor byte.
  //}
  //else
  //{
  //  outb(0x3D4, 0x0A);
  //  outb(0x3D5, 0x20);
  //}
}

void vga_textmode_clear()
{
  int x;
  int y;
   
  uint8_t attributeByte = ( backColor << 4 ) | ( foreColor & 0x0F );
  uint16_t blank = 0x20 /* space */ | ( attributeByte << 8 );
  int ti;
  for (ti = 0; ti < textRows*textCols; ti++ )
    txtVideoRAM[ti] = blank;
    
  cursorX = 0; cursorY = 0;
}

void puts(char* string)
{
  while(*string != '\0')
  {
    putc(*string);
    string++;
  }
}

void putc(uint8_t c)
{
  // The attribute byte is made up of two nibbles - the lower being the
  // foreground colour, and the upper the background colour.
  uint16_t attrib = (backColor << 4) | (foreColor & 0x0F);
  uint16_t *location;
   
  
  switch(c)
  {
    // backspace
    case 0x0E:
      if(cursorX > 0)
      {
	cursorX--;
	location = txtVideoRAM + (cursorY * textCols + cursorX) ;
	*location = ' ' | (attrib << 8);
	vga_update_cursor();
	return;
      }
      else if(cursorX == 0 && cursorY>0)
      {
	cursorX = textCols-1;
	cursorY--;
	location = txtVideoRAM + (cursorY * textCols + cursorX) ;
	*location = ' ' | (attrib << 8); 
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
      location = txtVideoRAM + (cursorY * textCols + cursorX) ;
      *location = c | (attrib << 8); 
      cursorX++;
      break;
  }
  
  if(cursorX >= textCols)
  {
      cursorX = 0;
      cursorY++;
  }

  vga_scroll();
  
  vga_update_cursor();
}