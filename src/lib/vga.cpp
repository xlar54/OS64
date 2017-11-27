#include <lib/vga.h>


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

// Graphic mode functions

void vga_init(uint32_t* videoMem, uint32_t width, uint32_t height, uint32_t pitch, uint8_t bpp)
{
    vga_framebuffer = (uint8_t*)videoMem;
    vga_width = width;
    vga_height = height;
    vga_pitch = pitch;
    vga_mode_bpp = bpp;
    vga_pixel_width = bpp/8;
    
    //vga_textRows = height/16;
    //vga_textCols = pitch/8;
       
    /*vga_textscreen = new uint8_t*[vga_textCols];
    for(int i = 0; i < vga_textCols; i++) {
	vga_textscreen[i] = new uint8_t[vga_textRows];
    }*/
    
    vga_set_color_palette();
}

void vga_clear()
{
  int x,y;
  for(x=0; x<vga_width;x++)
    for(y=0; y<vga_height;y++)
      vga_put_pixel(x,y,vga_backColor);
    
  vga_cursorRow=0;
  vga_cursorCol=0;
  
  for(int c=0;c<vga_textCols;c++)
    for(int r=0;r<vga_textRows;r++)
      vga_textscreen[c][r] = ' ';
}

void vga_put_pixel(int x, int y, uint8_t color)
{
  uint8_t *pixel = vga_framebuffer + y*vga_pitch + x*vga_pixel_width;     
  *pixel = vga_color_palette[color] & 255;              
  *(pixel + 1) = (vga_color_palette[color] >> 8) & 255;
};

void vga_draw_rect(int x, int y, int n, uint8_t color)
{
  for(int i=1; i <= n ; i++)
    *(vga_framebuffer + y * vga_pitch + x + i) = color;
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

void vga_set_color_palette()
{
  vga_color_palette[0] = ((0x00>>3)<<11) | ((0x00>>2)<<5) | (0x00>>3);
  vga_color_palette[1] = ((0xff>>3)<<11) | ((0xff>>2)<<5) | (0xff>>3);
  vga_color_palette[2] = ((0x68>>3)<<11) | ((0x37>>2)<<5) | (0x2b>>3);
  vga_color_palette[3] = ((0x70>>3)<<11) | ((0xa4>>2)<<5) | (0xb2>>3);
  vga_color_palette[4] = ((0x6f>>3)<<11) | ((0x3d>>2)<<5) | (0x86>>3);
  vga_color_palette[5] = ((0x58>>3)<<11) | ((0x8d>>2)<<5) | (0x43>>3);
  vga_color_palette[6] = ((0x35>>3)<<11) | ((0x28>>2)<<5) | (0x79>>3);
  vga_color_palette[7] = ((0xb8>>3)<<11) | ((0xc7>>2)<<5) | (0x6f>>3);
  vga_color_palette[8] = ((0x6f>>3)<<11) | ((0x4f>>2)<<5) | (0x25>>3);
  vga_color_palette[9] = ((0x43>>3)<<11) | ((0x39>>2)<<5) | (0x00>>3);
  vga_color_palette[10] = ((0x9a>>3)<<11) | ((0x67>>2)<<5) | (0x59>>3);
  vga_color_palette[11] = ((0x44>>3)<<11) | ((0x44>>2)<<5) | (0x44>>3);
  vga_color_palette[12] = ((0x6c>>3)<<11) | ((0x6c>>2)<<5) | (0x6c>>3);
  vga_color_palette[13] = ((0x9a>>3)<<11) | ((0xd2>>2)<<5) | (0x84>>3);
  vga_color_palette[14] = ((0x6c>>3)<<11) | ((0x5e>>2)<<5) | (0xb5>>3);
  vga_color_palette[15] = ((0x95>>3)<<11) | ((0x95>>2)<<5) | (0x95>>3);
}

void vga_cursor_enable()
{
  vga_cursorOn = 1;
}

void vga_cursor_disable()
{
  vga_cursorOn = 0;
}

void vga_cursor_update()
{
  if(vga_cursorOn == 1)
  {
    putc_col_row(vga_cursorCol, vga_cursorRow, '_');
  }
}

void vga_scroll()
{
  if(vga_cursorRow >= vga_textRows)
  {
    vga_cursorRow=0;
    vga_cursorCol=0;
    
    for (int r=0;r<vga_textRows;r++)
      for(int c=0;c<vga_textCols;c++)
	vga_textscreen[c][r] = vga_textscreen[c][r+1];

    for (int c=0;c<vga_textCols;c++)
      vga_textscreen[c][vga_textRows-1] = ' ';

    vga_restore_textscreen();
    
    // The cursor should now be on the last line.
    vga_cursorRow = vga_textRows-1;
    
  }
}

void vga_restore_textscreen()
{
  for (int r=0;r<vga_textRows;r++)
  {
    for(int c=0;c<vga_textCols;c++)
      putc_col_row(c,r, vga_textscreen[c][r]);
  }
}

void puts(char* string)
{
  while(*string != '\0')
  {
    putc(*string);
    string++;
  }
}

void putc_col_row(int8_t col, uint8_t row, uint8_t c)
{
  int y=row*16;
  int x=col*8;
    
  for(int z=0;z<16;z++)
  {
    uint8_t fdata = g_8x16_font[16*c+z];
    
    for (int bit = 8; bit > 0; bit--) {
      if ((fdata & (1 << bit)))
	vga_put_pixel(x++, y, vga_foreColor);
      else
	vga_put_pixel(x++, y, vga_backColor);
    }
    x=col*8;
    y++;
  }
}

void putc(uint8_t c)
{
  switch(c)
  {
    // backspace
    case 0x0E:
      if(vga_cursorCol > 0)
      {
	if(vga_cursorOn == 1) putc_col_row(vga_cursorCol, vga_cursorRow, ' ');
	vga_cursorCol--;
	putc_col_row(vga_cursorCol,vga_cursorRow,' ');
	vga_textscreen[vga_cursorCol][vga_cursorRow] = ' ';
	vga_cursor_update();
	return;
      }
      else if(vga_cursorCol == 0 && vga_cursorRow>0)
      {
	if(vga_cursorOn == 1) putc_col_row(vga_cursorCol, vga_cursorRow, ' ');
	vga_cursorCol = vga_textCols-1;
	vga_cursorRow--;
	putc_col_row(vga_cursorCol,vga_cursorRow,' ');
	vga_textscreen[vga_cursorCol][vga_cursorRow] = ' ';
	vga_cursor_update();
	return;
      }
      else if(vga_cursorCol == 0 && vga_cursorRow==0)
	return;
      break;
    //case 0x0F:
      //cursorX = (cursorX + 8) & ~(8 - 1);
      //vga_update_cursor();
      //break;
    case '\n':
      if(vga_cursorOn == 1) putc_col_row(vga_cursorCol, vga_cursorRow, ' ');
      vga_cursorCol = 0;
      vga_cursorRow++;
      break;
    default:
      putc_col_row(vga_cursorCol,vga_cursorRow,c);
      vga_textscreen[vga_cursorCol][vga_cursorRow] = c;
      vga_cursorCol++;
      break;
  }
  
  if(vga_cursorCol >= vga_textCols)
  {
    vga_cursorCol = 0;
    vga_cursorRow++;
  }
 
  vga_scroll();
  
  vga_cursor_update();
}
