
#include <drivers/vga.h>

using namespace myos::drivers;

           
            
VideoGraphicsArray::VideoGraphicsArray() : 
    miscPort(0x3c2),
    crtcIndexPort(0x3d4),
    crtcDataPort(0x3d5),
    sequencerIndexPort(0x3c4),
    sequencerDataPort(0x3c5),
    graphicsControllerIndexPort(0x3ce),
    graphicsControllerDataPort(0x3cf),
    attributeControllerIndexPort(0x3c0),
    attributeControllerReadPort(0x3c1),
    attributeControllerWritePort(0x3c0),
    attributeControllerResetPort(0x3da)
{
}

VideoGraphicsArray::~VideoGraphicsArray()
{
}


            
void VideoGraphicsArray::WriteRegisters(uint8_t* registers)
{
    //  misc
    miscPort.Write(*(registers++));
    
    // sequencer
    for(uint8_t i = 0; i < 5; i++)
    {
        sequencerIndexPort.Write(i);
        sequencerDataPort.Write(*(registers++));
    }
    
    // cathode ray tube controller
    crtcIndexPort.Write(0x03);
    crtcDataPort.Write(crtcDataPort.Read() | 0x80);
    crtcIndexPort.Write(0x11);
    crtcDataPort.Write(crtcDataPort.Read() & ~0x80);
    
    registers[0x03] = registers[0x03] | 0x80;
    registers[0x11] = registers[0x11] & ~0x80;
    
    for(uint8_t i = 0; i < 25; i++)
    {
        crtcIndexPort.Write(i);
        crtcDataPort.Write(*(registers++));
    }
    
    // graphics controller
    for(uint8_t i = 0; i < 9; i++)
    {
        graphicsControllerIndexPort.Write(i);
        graphicsControllerDataPort.Write(*(registers++));
    }
    
    // attribute controller
    for(uint8_t i = 0; i < 21; i++)
    {
        attributeControllerResetPort.Read();
        attributeControllerIndexPort.Write(i);
        attributeControllerWritePort.Write(*(registers++));
    }
    
    attributeControllerResetPort.Read();
    attributeControllerIndexPort.Write(0x20);
    
}

bool VideoGraphicsArray::SupportsMode(uint32_t width, uint32_t height, uint32_t colordepth)
{
    return width == SCREENX && height == SCREENY && colordepth == COLORDEPTH;
}

bool VideoGraphicsArray::SetMode(uint32_t width, uint32_t height, uint32_t colordepth)
{
    if(!SupportsMode(width, height, colordepth))
        return false;
    
    unsigned char mode_320_200_256[] =
    {
        /* MISC */
            0x63,
        /* SEQ */
            0x03, 0x01, 0x0F, 0x00, 0x0E,
        /* CRTC */
            0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0xBF, 0x1F,
            0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x9C, 0x0E, 0x8F, 0x28, 0x40, 0x96, 0xB9, 0xA3,
            0xFF,
        /* GC */
            0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x0F,
            0xFF,
        /* AC */
            0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
            0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
            0x41, 0x00, 0x0F, 0x00, 0x00
    };
    
    unsigned char mode_80_25_text[]={
      0x67,
      0x03,0x00,0x03,0x00,0x02,
      /* CRTC 0x3d4 */
      0x5F, 0x4F, 0x50, 0x82, 0x55, 0x81, 0xBF, 0x1F,
      0x00, 0x4F, 0x0D, 0x0E, 0x00, 0x00, 0x00, 0x50,
      0x9C, 0x0E, 0x8F, 0x28,   0x1F, 0x96, 0xB9, 0xA3,
      0xFF,
      /* GC 0x3ce */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x0E, 0x00,
      0xFF,
      /* AC 0x3C0*/
      0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07,
      0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
      0x0C, 0x00, 0x0F, 0x08,   0x00
    };
    
    unsigned char mode_640_480_16[] =
    {
    /* MISC */
	    0xE3,
    /* SEQ */
	    0x03, 0x01, 0x08, 0x00, 0x06,
    /* CRTC */
	    0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0x0B, 0x3E,
	    0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	    0xEA, 0x0C, 0xDF, 0x28, 0x00, 0xE7, 0x04, 0xE3,
	    0xFF,
    /* GC */
	    0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x05, 0x0F,
	    0xFF,
    /* AC */
	    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07,
	    0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
	    0x01, 0x00, 0x0F, 0x00, 0x00
    };
    
    WriteRegisters(mode_320_200_256);
    //WriteRegisters(mode_640_480_16);
    //WriteRegisters(mode_80_25_text);
    return true;
}


uint8_t* VideoGraphicsArray::GetFrameBufferSegment()
{
    graphicsControllerIndexPort.Write(0x06);
    uint8_t segmentNumber = graphicsControllerDataPort.Read() & (3<<2);
    switch(segmentNumber)
    {
        default:
        case 0<<2: return (uint8_t*)0x00000;
        case 1<<2: return (uint8_t*)0xA0000;
        case 2<<2: return (uint8_t*)0xB0000;
        case 3<<2: return (uint8_t*)0xB8000;
    }
}
            
void VideoGraphicsArray::PutPixel(int32_t x, int32_t y,  uint8_t colorIndex)
{
    if(x < 0 || SCREENX <= x || y < 0 || SCREENY <= y)
        return;
        
    uint8_t* pixelAddress = GetFrameBufferSegment() + SCREENX*y + x;
    *pixelAddress = colorIndex;
}

uint8_t VideoGraphicsArray::GetColorIndex(uint8_t r, uint8_t g, uint8_t b)
{
    /*if(r == 0x00 && g == 0x00 && b == 0x00) return 0x00; // black
    if(r == 0x00 && g == 0x00 && b == 0xA8) return 0x01; // blue
    if(r == 0x00 && g == 0xA8 && b == 0x00) return 0x02; // green
    if(r == 0xA8 && g == 0x00 && b == 0x00) return 0x04; // red
    if(r == 0xFF && g == 0xFF && b == 0xFF) return 0x3F; // white*/
  
    if(r == 0x00 && g == 0x00 && b == 0x00) return 0x00; // black
    if(r == 0x00 && g == 0x00 && b == 0xFF) return 0x01; // blue
    if(r == 0x00 && g == 0xFF && b == 0x00) return 0x02; // green
    if(r == 0x66 && g == 0xda && b == 0xFF) return 0x03; // cyan
    if(r == 0xab && g == 0x31 && b == 0x26) return 0x04; // red
    if(r == 0xbb && g == 0x3f && b == 0xb8) return 0x05; // magenta
    if(r == 0x78 && g == 0x53 && b == 0x00) return 0x06; // brown
    if(r == 0xb8 && g == 0xb8 && b == 0xb8) return 0x07; // lgrey
    if(r == 0x5b && g == 0x5b && b == 0x5b) return 0x08; // grey
    if(r == 0xaa && g == 0x9d && b == 0xef) return 0x09; // lblue
    if(r == 0xb0 && g == 0xf4 && b == 0xac) return 0x0A; // lgreen
    if(r == 0x66 && g == 0xda && b == 0xFF) return 0x0B; // lcyan
    if(r == 0xdd && g == 0x93 && b == 0x87) return 0x0C; // lred
    if(r == 0xb9 && g == 0x74 && b == 0x18) return 0x0D; // lmagenta
    if(r == 0xea && g == 0xf5 && b == 0x7c) return 0x0E; // yellow
    if(r == 0xff && g == 0xff && b == 0xff) return 0x0F; // white
     
    return 0x00;
}
           
void VideoGraphicsArray::PutPixel(int32_t x, int32_t y,  uint8_t r, uint8_t g, uint8_t b)
{
    PutPixel(x,y, GetColorIndex(r,g,b));
}

void VideoGraphicsArray::Clear()
{
  uint32_t col = backgroundColor;
  
  uint8_t r =  (col>>16) & 0xFF;
  uint8_t g =  (col>>8) & 0xFF;
  uint8_t b =  col & 0xFF;
  
  FillRectangle(0, 0, SCREENX, SCREENY,r,g,b);
}

void VideoGraphicsArray::FillRectangle(uint32_t x, uint32_t y, uint32_t w, uint32_t h,   uint8_t r, uint8_t g, uint8_t b)
{
    for(int32_t Y = y; Y < y+h; Y++)
        for(int32_t X = x; X < x+w; X++)
            PutPixel(X, Y, r, g, b);
}

