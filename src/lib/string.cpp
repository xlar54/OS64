#include <lib/string.h>

void strcpy( char *d, const char *s )
{
  while(*s) {
    *d++ = *s++;
  }
  *d = 0;
}

char* strncpy(char *dst, const char* src, size_t n)
{
  if ((dst == NULL) || (src == NULL))
	return NULL;
  size_t i;
  for(i = 0; i < n && src[i] != '\0'; ++i)
	dst[i] = src[i];
  for(; i < n; ++i)
	dst[i] = '\0';
  return dst;
}

int strcmp( const char *a, const char *b )
{
  while(1) {
    if(*a<*b) {
	    return -1;
    } else if(*a>*b) {
	    return 1;
    } else if(*a==0) {
	    return 0;
    } else {
	    a++;
	    b++;
    }
  }			
}

int strncmp( const char *a, const char *b, unsigned length )
{
  while(length>0) {
    if(*a<*b) {
	    return -1;
    } else if(*a>*b) {
	    return 1;
    } else if(*a==0) {
	    return 0;
    } else {
	    a++;
	    b++;
	    length--;
    }
  }
  return 0;
}

unsigned strlen( const char *s )
{
  unsigned len=0;
  while(*s) {
    len++;
    s++;
  }
  return len;
}

const char* strchr(const char *source, int c)
{
  while(*source){
    if(*source == c)
      break;
    source++;
  }
  return source;
}

char* strrchr(char *s, int c)
{
  char* ret=0;
  do {
      if( *s == (char)c )
	  ret=s;
  } while(*s++);
  return ret;
}

char * strtok ( char *s, const char *delim)
{
  static char *oldword=0;
  char *word;

  if(!s) s=oldword;

  while(*s && strchr(delim,*s)) s++;

  if(!*s) {
    oldword = s;
    return 0;
  }

  word = s;
  while(*s && !strchr(delim,*s)) s++;

  if(*s) {
      *s = 0;
      oldword = s+1;
  } else {
      oldword = s;
  }

  return word;
}

void memset( void *vd, char value, unsigned length )
{
  char *d = (char *)vd;
  while(length) {
    *d = value;
    length--;
    d++;
  }
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

void* memcpy(uint8_t* destination, const uint8_t* source, size_t num)
{
  int i;
  uint8_t* d = destination;
  const uint8_t* s = source;
  for (i = 0; i < num; i++) {
    d[i] = s[i];
  }
  return destination;
}

void memcpy(void *dest, const void *source, size_t num) {
  int i = 0;
  // casting pointers
  char *dest8 = (char *)dest;
  char *source8 = (char *)source;
  for (i = 0; i < num; i++) {
    dest8[i] = source8[i];
  }
}

char tolower(char ch)
{
        if(ch >= 'A' && ch <= 'Z')
                return ('a' + ch - 'A');
        else
                return ch;
}

char toupper(char c) 
{
    if(c >= 'a' && c <= 'z') {
        return c + ('A'-'a');
    }
    else {
        return c;
    }
}
int strncasecmp(const char *s1, const char *s2, size_t n)
{
    if (n == 0)
        return 0;

    while (n > 1 && *s1 != 0 && tolower(*s1) == tolower(*s2)) {
        s1++;
        s2++;
        n--;
    }
    
    return (int)*s1 - (int)*s2;
}