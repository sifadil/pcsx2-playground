//
// Code base on: vsprintf.c
//
// Print formatting routines
//
// Copyright (C) 2002 Michael Ringgaard. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 
// 1. Redistributions of source code must retain the above copyright 
//    notice, this list of conditions and the following disclaimer.  
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.  
// 3. Neither the name of the project nor the names of its contributors
//    may be used to endorse or promote products derived from this software
//    without specific prior written permission. 
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
// OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
// OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
// SUCH DAMAGE.
// 

// modified by gigahers and air to write formatted output directly into a std::string container.

#include "PrecompiledHeader.h"

#include <math.h>

#ifdef KERNEL
#define NOFLOAT
#endif

#define ZEROPAD 1               // Pad with zero (not to be confused with Zero's PAD plugin)
#define SIGN    2               // Unsigned/signed long
#define PLUS    4               // Show plus
#define SPACE   8               // Space if plus
#define LEFT    16              // Left justified
#define SPECIAL 32              // 0x
#define LARGE   64              // Use 'ABCDEF' instead of 'abcdef'

#define is_digit(c) ((c) >= '0' && (c) <= '9')

static char *digits = "0123456789abcdefghijklmnopqrstuvwxyz";
static char *upper_digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

#ifdef NEED_STRLEN
static size_t strnlen(const char *s, size_t count)
{
  const char *sc;
  for (sc = s; *sc != '\0' && count--; ++sc);
  return sc - s;
}
#endif

static void cvt(char (&buf)[_CVTBUFSIZE], double arg, int preci, int& decpt, int& sign, int eflag)
{
  int r2;
  double fi, fj;
  char *p, *p1;

  r2 = 0;
  sign = 0;
  p = &buf[0];
  if (arg < 0)
  {
    sign = 1;
    arg = -arg;
  }
  arg = modf(arg, &fi);
  p1 = &buf[_CVTBUFSIZE];

  if (fi != 0) 
  {
    while (fi != 0) 
    {
      fj = modf(fi / 10, &fi);
      *--p1 = (int)((fj + .03) * 10) + '0';
      r2++;
    }
    while (p1 < &buf[_CVTBUFSIZE]) *p++ = *p1++;
  } 
  else if (arg > 0)
  {
    while ((fj = arg * 10) < 1) 
    {
      arg = fj;
      r2--;
    }
  }

  p1 = &buf[preci];

  if (eflag == 0) p1 += r2;
  decpt = r2;
  if (p1 < &buf[0]) 
  {
    buf[0] = '\0';
    return;
  }
  while (p <= p1 && p < &buf[_CVTBUFSIZE])
  {
    arg *= 10;
    arg = modf(arg, &fj);
    *p++ = (int) fj + '0';
  }
  if (p1 >= &buf[_CVTBUFSIZE]) 
  {
    buf[_CVTBUFSIZE - 1] = '\0';
    return;
  }
  p = p1;
  *p1 += 5;
  while (*p1 > '9') 
  {
    *p1 = '0';
    if (p1 > buf)
      ++*--p1;
    else 
    {
      *p1 = '1';
      decpt++;
      if (eflag == 0) 
      {
        if (p > buf) *p = '0';
        p++;
      }
    }
  }
  *p = '\0';
  return;
}

static void ecvtbuf(char (&buf)[_CVTBUFSIZE], double arg, int preci, int& decpt, int& sign)
{
  cvt(buf, arg, preci, decpt, sign, 1);
}

static void fcvtbuf(char (&buf)[_CVTBUFSIZE], double arg, int preci, int& decpt, int& sign)
{
  cvt(buf, arg, preci, decpt, sign, 0);
}

static int skip_atoi(const char **s)
{
  int i = 0;
  while (is_digit(**s)) i = i*10 + *((*s)++) - '0';
  return i;
}

static void number(std::string& dest, long num, int base, int size, int precision, int type)
{
  char c, sign, tmp[66];
  char *dig = digits;
  int i;

  if (type & LARGE)  dig = upper_digits;
  if (type & LEFT) type &= ~ZEROPAD;
  if (base < 2 || base > 36) return;
  
  c = (type & ZEROPAD) ? '0' : ' ';
  sign = 0;
  if (type & SIGN)
  {
    if (num < 0)
    {
      sign = '-';
      num = -num;
      size--;
    }
    else if (type & PLUS)
    {
      sign = '+';
      size--;
    }
    else if (type & SPACE)
    {
      sign = ' ';
      size--;
    }
  }

  if (type & SPECIAL)
  {
    if (base == 16)
      size -= 2;
    else if (base == 8)
      size--;
  }

  i = 0;

  if (num == 0)
    tmp[i++] = '0';
  else
  {
    while (num != 0)
    {
      tmp[i++] = dig[((unsigned long) num) % (unsigned) base];
      num = ((unsigned long) num) / (unsigned) base;
    }
  }

  if (i > precision) precision = i;
  size -= precision;
  if (!(type & (ZEROPAD | LEFT))) while (size-- > 0) dest += ' ';
  if (sign) dest += sign;
  
  if (type & SPECIAL)
  {
    if (base == 8)
      dest += '0';
    else if (base == 16)
    {
      dest += '0';
      dest += digits[33];
    }
  }

  if (!(type & LEFT)) while (size-- > 0) dest += c;
  while (i < precision--) dest += '0';
  while (i-- > 0) dest += tmp[i];
  while (size-- > 0) dest += ' ';
}

static void eaddr( std::string& dest, unsigned char *addr, int size, int precision, int type)
{
  char tmp[24];
  char *dig = digits;
  int i, len;

  if (type & LARGE)  dig = upper_digits;
  len = 0;
  for (i = 0; i < 6; i++)
  {
    if (i != 0) tmp[len++] = ':';
    tmp[len++] = dig[addr[i] >> 4];
    tmp[len++] = dig[addr[i] & 0x0F];
  }

  if (!(type & LEFT)) while (len < size--) dest += ' ';
  for (i = 0; i < len; ++i) dest += tmp[i];
  while (len < size--) dest += ' ';
}

static void iaddr( std::string& dest, unsigned char *addr, int size, int precision, int type)
{
  char tmp[24];
  int i, n, len;

  len = 0;
  for (i = 0; i < 4; i++)
  {
    if (i != 0) tmp[len++] = '.';
    n = addr[i];
    
    if (n == 0)
      tmp[len++] = digits[0];
    else
    {
      if (n >= 100) 
      {
        tmp[len++] = digits[n / 100];
        n = n % 100;
        tmp[len++] = digits[n / 10];
        n = n % 10;
      }
      else if (n >= 10) 
      {
        tmp[len++] = digits[n / 10];
        n = n % 10;
      }

      tmp[len++] = digits[n];
    }
  }

  if (!(type & LEFT)) while (len < size--) dest += ' ';
  for (i = 0; i < len; ++i) dest += tmp[i];
  while (len < size--) dest += ' ';
}

#ifndef NOFLOAT

static void cfltcvt(double value, char *buffer, char fmt, int precision)
{
  int decpt, sign, exp, pos;
  char cvtbuf[_CVTBUFSIZE];
  int capexp = 0;
  int magnitude;

  if (fmt == 'G' || fmt == 'E')
  {
    capexp = 1;
    fmt += 'a' - 'A';
  }

  if (fmt == 'g')
  {
    ecvtbuf(cvtbuf, value, precision, decpt, sign);

    magnitude = decpt - 1;
    if (magnitude < -4  ||  magnitude > precision - 1)
    {
      fmt = 'e';
      precision -= 1;
    }
    else
    {
      fmt = 'f';
      precision -= decpt;
    }
  }

  if (fmt == 'e')
  {
    ecvtbuf(cvtbuf, value, precision+1, decpt, sign);

	const char* digits = cvtbuf;

    if (sign) *buffer++ = '-';
    *buffer++ = *digits;
    if (precision > 0) *buffer++ = '.';
    memcpy(buffer, digits + 1, precision);
    buffer += precision;
    *buffer++ = capexp ? 'E' : 'e';

    if (decpt == 0)
    {
      if (value == 0.0)
        exp = 0;
      else
        exp = -1;
    }
    else
      exp = decpt - 1;

    if (exp < 0)
    {
      *buffer++ = '-';
      exp = -exp;
    }
    else
      *buffer++ = '+';

    buffer[2] = (exp % 10) + '0';
    exp = exp / 10;
    buffer[1] = (exp % 10) + '0';
    exp = exp / 10;
    buffer[0] = (exp % 10) + '0';
    buffer += 3;
  }
  else if (fmt == 'f')
  {
	fcvtbuf(cvtbuf, value, precision, decpt, sign);

	const char* digits = cvtbuf;

    if (sign) *buffer++ = '-';
    if (*digits)
    {
      if (decpt <= 0)
      {
        *buffer++ = '0';
        *buffer++ = '.';
        for (pos = 0; pos < -decpt; pos++) *buffer++ = '0';
        while (*digits) *buffer++ = *digits++;
      }
      else
      {
        pos = 0;
        while (*digits)
        {
          if (pos++ == decpt) *buffer++ = '.';
          *buffer++ = *digits++;
        }
      }
    }
    else
    {
      *buffer++ = '0';
      if (precision > 0)
      {
        *buffer++ = '.';
        for (pos = 0; pos < precision; pos++) *buffer++ = '0';
      }
    }
  }

  *buffer = '\0';
}

static void forcdecpt(char *buffer)
{
  while (*buffer)
  {
    if (*buffer == '.') return;
    if (*buffer == 'e' || *buffer == 'E') break;
    buffer++;
  }

  if (*buffer)
  {
    int n = strlen(buffer);
    while (n > 0) 
    {
      buffer[n + 1] = buffer[n];
      n--;
    }

    *buffer = '.';
  }
  else
  {
    *buffer++ = '.';
    *buffer = '\0';
  }
}

static void cropzeros(char *buffer)
{
  char *stop;

  while (*buffer && *buffer != '.') buffer++;
  if (*buffer++)
  {
    while (*buffer && *buffer != 'e' && *buffer != 'E') buffer++;
    stop = buffer--;
    while (*buffer == '0') buffer--;
    if (*buffer == '.') buffer--;
    while (*++buffer = *stop++);
  }
}

static void flt( std::string& dest, double num, int size, int precision, char fmt, int flags)
{
  char tmp[80];
  char c, sign;
  int n, i;

  // Left align means no zero padding
  if (flags & LEFT) flags &= ~ZEROPAD;

  // Determine padding and sign char
  c = (flags & ZEROPAD) ? '0' : ' ';
  sign = 0;
  if (flags & SIGN)
  {
    if (num < 0.0)
    {
      sign = '-';
      num = -num;
      size--;
    }
    else if (flags & PLUS)
    {
      sign = '+';
      size--;
    }
    else if (flags & SPACE)
    {
      sign = ' ';
      size--;
    }
  }

  // Compute the precision value
  if (precision < 0)
    precision = 6; // Default precision: 6
  else if (precision == 0 && fmt == 'g')
    precision = 1; // ANSI specified

  // Convert floating point number to text
  cfltcvt(num, tmp, fmt, precision);

  // '#' and precision == 0 means force a decimal point
  if ((flags & SPECIAL) && precision == 0) forcdecpt(tmp);

  // 'g' format means crop zero unless '#' given
  if (fmt == 'g' && !(flags & SPECIAL)) cropzeros(tmp);

  n = strlen(tmp);

  // Output number with alignment and padding
  size -= n;
  if (!(flags & (ZEROPAD | LEFT))) while (size-- > 0) dest += ' ';
  if (sign) dest += sign;
  if (!(flags & LEFT)) while (size-- > 0) dest += c;
  for (i = 0; i < n; i++) dest += tmp[i];
  while (size-- > 0) dest += ' ';
}

#endif

void vssprintf(std::string& dest, const std::string& format, va_list args)
{
  int len;
  unsigned long num;
  int i, base;

  int flags;            // Flags to number()

  int field_width;      // Width of output field
  int precision;        // Min. # of digits for integers; max number of chars for from string
  int qualifier;        // 'h', 'l', or 'L' for integer fields

  dest.clear();

  for( const char* fmt = format.c_str(); *fmt; fmt++ )
  {
    if (*fmt != '%')
    {
      dest += *fmt;
      continue;
    }
                  
    // Process flags
    flags = 0;
repeat:
    fmt++; // This also skips first '%'
    switch (*fmt)
    {
      case '-': flags |= LEFT; goto repeat;
      case '+': flags |= PLUS; goto repeat;
      case ' ': flags |= SPACE; goto repeat;
      case '#': flags |= SPECIAL; goto repeat;
      case '0': flags |= ZEROPAD; goto repeat;
    }
          
    // Get field width
    field_width = -1;
    if (is_digit(*fmt))
      field_width = skip_atoi(&fmt);
    else if (*fmt == '*')
    {
      fmt++;
      field_width = va_arg(args, int);
      if (field_width < 0)
      {
        field_width = -field_width;
        flags |= LEFT;
      }
    }

    // Get the precision
    precision = -1;
    if (*fmt == '.')
    {
      ++fmt;    
      if (is_digit(*fmt))
        precision = skip_atoi(&fmt);
      else if (*fmt == '*')
      {
        ++fmt;
        precision = va_arg(args, int);
      }
      if (precision < 0) precision = 0;
    }

    // Get the conversion qualifier
    qualifier = -1;
    if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L')
    {
      qualifier = *fmt;
      fmt++;
    }

    // Default base
    base = 10;

    switch (*fmt)
    {
      case 'c':
        if (!(flags & LEFT)) while (--field_width > 0) dest += ' ';
        dest += (unsigned char) va_arg(args, int);
        while (--field_width > 0) dest += ' ';
        continue;

      case 's':
	  {
		const char* s = va_arg(args, char *);
        if (!s) s = "<NULL>";
		len = strnlen(s, precision);
		if (!(flags & LEFT)) while (len < field_width--) dest += ' ';
		for (i = 0; i < len; ++i) dest += *s++;
		while (len < field_width--) dest += ' ';
	  }
      continue;

      // let's add support for std:string as a formatted parameter!  (air)
	  case 'S':
	  {
		  std::string* ss = va_arg(args, std::string*);
		  const char* s = ( ss!=NULL ) ? ss->c_str() : "<NULL>";
			len = strnlen(s, precision);
			if (!(flags & LEFT)) while (len < field_width--) dest += ' ';
			for (i = 0; i < len; ++i) dest += *s++;
			while (len < field_width--) dest += ' ';
      }
	  continue;

	  case 'p':
      {
        if (field_width == -1)
        {
          field_width = 2 * sizeof(void *);
          flags |= ZEROPAD;
        }
        number(dest, (unsigned long) va_arg(args, void *), 16, field_width, precision, flags);
	  }
      continue;

      case 'n':
        if (qualifier == 'l')
        {
          long *ip = va_arg(args, long *);
		  *ip = dest.length();
        }
        else
        {
          int *ip = va_arg(args, int *);
          *ip = dest.length();
        }
        continue;

      case 'A':
        flags |= LARGE;

      case 'a':
        if (qualifier == 'l')
          eaddr(dest, va_arg(args, unsigned char *), field_width, precision, flags);
        else
          iaddr(dest, va_arg(args, unsigned char *), field_width, precision, flags);
        continue;

      // Integer number formats - set up the flags and "break"
      case 'o':
        base = 8;
        break;

      case 'X':
        flags |= LARGE;

      case 'x':
        base = 16;
        break;

      case 'd':
      case 'i':
        flags |= SIGN;

      case 'u':
        break;

#ifndef NOFLOAT

      case 'E':
      case 'G':
      case 'e':
      case 'f':
      case 'g':
        flt(dest, va_arg(args, double), field_width, precision, *fmt, flags | SIGN);
        continue;

#endif

      default:
        if (*fmt != '%') dest += '%';
        if (*fmt)
          dest += *fmt;
        else
          --fmt;
        continue;
    }

    if (qualifier == 'l')
      num = va_arg(args, unsigned long);
    else if (qualifier == 'h')
    {
      if (flags & SIGN)
        num = va_arg(args, short);
      else
        num = va_arg(args, unsigned short);
    }
    else if (flags & SIGN)
      num = va_arg(args, int);
    else
      num = va_arg(args, unsigned int);

    number(dest, num, base, field_width, precision, flags);
  }
}

void ssprintf(std::string& str, const std::string& fmt, VARG_PARAM dummy, ...)
{
	dummy_assert();

	va_list args;
	va_start(args, dummy);
	vssprintf(str, fmt, args);
	va_end(args);
}

std::string fmt_string( const std::string& fmt, VARG_PARAM dummy, ... )
{
	dummy_assert();

	std::string retval;
	va_list args;
	va_start( args, dummy );
	vssprintf( retval, fmt, args );
	va_end( args );

	return retval;
}