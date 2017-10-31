#ifndef STRING_H
#define STRING_H

#include <lib/stdint.h>

void strcpy( char *d, const char *s );
char* strncpy(char *dst, const char* src, size_t n);

int strcmp( const char *a, const char *b );
int strncmp( const char *a, const char *b, unsigned length );

unsigned strlen( const char *s );

const char* strchr(const char *source, int c);
char* strrchr(char *source, int c);

char* strtok( char *s, const char *delim );

void memset( void *d, char value, unsigned length );
void* memcpy(uint16_t* destination, const uint16_t* source, size_t num);
void* memcpy(uint8_t* destination, const uint8_t* source, size_t num);
void memcpy(void *dest, const void *source, size_t num);

int strncasecmp(const char *s1, const char *s2, size_t n);
char toupper(char c);
char tolower(char ch);
#endif