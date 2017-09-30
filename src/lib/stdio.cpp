
#include <lib/stdio.h>


char *convert(unsigned int num, int base, int caseflg, int digits)
{
  static char buff[33]="00000000000000000000000000000000";
  char *ptr;
  
  for(int x=0;x<sizeof(buff)-1;x++)
    buff[x] = '0';
  
  ptr=&buff[sizeof(buff)-1];
  *ptr='\0';
   
  do
  {
    if (caseflg ==0)
      *--ptr="0123456789abcdef"[num%base];
    else
      *--ptr="0123456789ABCDEF"[num%base];
    num/=base;
  }while(num!=0);
  
  if (digits > 0)
  {
    int length = digits - strlen(ptr);
    
    for(int l=0;l<length;l++)
    {
      ptr--;
    }
  }
  
  return(ptr);
}

void printf(char* format,...)
{
    int i;
    char *p;
    char *s;
    unsigned u;
    va_list argp;
    va_start(argp, format);	
  
    p=format;
    for(p=format; *p!='\0';p++)
    {
      int digits=0; 
      
      if(*p != '%')
      {
	putc(*p);
	continue;
      }

      p++;
      
      if (*p == '0')
      {
	p++;
	if (*p == '\0') goto end;
	if (*p >= '0' && *p <= '9')
	{
	  digits = *p - '0';
	  p++;
	  if (*p == '\0') goto end;
	}
      }

      switch(*p)
      {
	case 'c' : i=va_arg(argp,int);putc(i);break;
	case 'd' : i=va_arg(argp,int);
	if(i<0){i=-i;putc('-');}puts(convert(i,10,0,digits));break;
	case 'o': i=va_arg(argp,unsigned int); puts(convert(i,8,0,digits));break;
	case 's': s=va_arg(argp,char *); puts(s); break;
	case 'u': u=va_arg(argp, unsigned int); puts(convert(u,10,0,digits));break;
	case 'x': u=va_arg(argp, unsigned int); puts(convert(u,16,0,digits));break;
	case 'X': u=va_arg(argp, unsigned int); puts(convert(u,16,1,digits));break;
	case '%': putc('%');break;
      }
    }
end:
    va_end(argp);
}

/*void printfHex(uint8_t key)
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
}*/

void displayMemory(uint8_t* buffer, uint16_t size)
{
    int newLine = 1;
    for(int z=0;z<size;z++)
    {
      if (newLine)
      {
	printf("\n%04X: ",z);
	newLine=0;
      }
      
      printf("%02X ", buffer[z]);
   
      if((z+1) % 16 == 0)
      {
	for(int bb=15; bb>=0;bb--)
	{
	  int idx = z-bb;
	  if(buffer[idx] > 0x1F && buffer[idx] < 0x7F)
	  {
	    char foo[2];
	    foo[0] = buffer[idx];
	    foo[1] = '\0';
	    printf(foo);
	  }
	    
	  else
	  {
	    printf(".");
	  }   
	}
	newLine=1;
      }

    }
}