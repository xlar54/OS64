/**
 * @file stdarg.h
 * 
 * This file was taken (in modified form) from Evan Teran's OS.
 */
#ifndef STDARG_H
#define STDARG_H

/* there are some inherant assumptions and limitations that come along with
   the stdarg technique, the most basic of which is that arguments must be
   in order linearly in memory and stack aligned, also no arguments may be
   passed via registers, as that would violate the first rule, after that, 
   easy pickings ;)
*/

/*
  basic type of va_list, this is kosher so long as:
  sizeof(unsigned char) == 1 which it is be definition
*/
typedef unsigned char *va_list;

// these typedefs come from other files, combined to make it simpler
#ifdef __SIZE_TYPE__
typedef __SIZE_TYPE__ size_t;
#else
typedef unsigned int size_t;
#endif
typedef signed int ssize_t;



// i believe that int is always supposed to be the width of stack elements
#define _stacktype int

// this is used to make sure types are aligned properly
#define _va_size(type) \
	((sizeof(type) + sizeof(_stacktype) - 1) & ~(sizeof(_stacktype) - 1))

// simply one element past the last argument
#define va_start(ap, last) \
	((ap) = ((reinterpret_cast<va_list> (&(last))) + _va_size(last)))

// zero out ap to invalidate it
#define va_end(ap) \
	((ap) = 0)

// increment the pointer, and then get the pointer at what it was
#define va_arg(ap, type) \
	(ap = (va_list) ((char *) (ap) + _va_size (type)), \
	*((type *) (void *) ((char *) (ap) - _va_size (type))))

#endif	// STDARG_H