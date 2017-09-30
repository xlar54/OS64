#ifndef STRING_H
#define STRING_H

#include <lib/stdint.h>

void	 strcpy( char *d, const char *s );
int	 strcmp( const char *a, const char *b );
int	 strncmp( const char *a, const char *b, unsigned length );
unsigned strlen( const char *s );
char * strtok( char *s, const char *delim );

const char * strchr( const char *s, char ch );

void	memset( void *d, char value, unsigned length );
void* memcpy(uint16_t* destination, const uint16_t* source, size_t num);

//void printf( const char *s, ... );

#endif