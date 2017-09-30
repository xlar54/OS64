#include <lib/string.h>

void strcpy( char *d, const char *s )
{
	while(*s) {
		*d++ = *s++;
	}
	*d = 0;
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

const char * strchr( const char *s, char ch )
{
	while(*s) {
		if(*s==ch) return s;
		s++;
	}
	return 0;
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

void	memset( void *vd, char value, unsigned length )
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

