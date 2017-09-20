 

#ifndef __MYOS__DRIVERS__VGA_H
#define __MYOS__DRIVERS__VGA_H

#include <common/types.h>
#include <hardwarecommunication/port.h>
#include <drivers/driver.h>

#define SCREENX	320
#define	SCREENY	200
#define COLORDEPTH 8

namespace myos
{
    namespace drivers
    {
        
        class VideoGraphicsArray
        {
        protected:
            hardwarecommunication::Port8Bit miscPort;
            hardwarecommunication::Port8Bit crtcIndexPort;
            hardwarecommunication::Port8Bit crtcDataPort;
            hardwarecommunication::Port8Bit sequencerIndexPort;
            hardwarecommunication::Port8Bit sequencerDataPort;
            hardwarecommunication::Port8Bit graphicsControllerIndexPort;
            hardwarecommunication::Port8Bit graphicsControllerDataPort;
            hardwarecommunication::Port8Bit attributeControllerIndexPort;
            hardwarecommunication::Port8Bit attributeControllerReadPort;
            hardwarecommunication::Port8Bit attributeControllerWritePort;
            hardwarecommunication::Port8Bit attributeControllerResetPort;
            
            void WriteRegisters(uint8_t* registers);
            uint8_t* GetFrameBufferSegment();
            
            virtual uint8_t GetColorIndex(uint8_t r, uint8_t g, uint8_t b);
            
            
        public:
            VideoGraphicsArray();
            ~VideoGraphicsArray();
	    uint32_t* frame;
	    uint32_t backgroundColor;
	    uint32_t foregroundColor;
	    uint8_t row;
	    uint8_t column;
            
            virtual bool SupportsMode(uint32_t width, uint32_t height, uint32_t colordepth);
            virtual bool SetMode(uint32_t width, uint32_t height, uint32_t colordepth);
            virtual void PutPixel(int32_t x, int32_t y, uint8_t r,uint8_t g,uint8_t b);
            virtual void PutPixel(int32_t x,int32_t y,uint8_t colorIndex);
            
            virtual void FillRectangle(uint32_t x,uint32_t y,uint32_t w,uint32_t h,  uint8_t r,uint8_t g,uint8_t b);
	    virtual void Clear();

        };
        
    }
}

#endif