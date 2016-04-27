/*! \file os.c
  The intent of this header file is to put implementations
  of operating system and compiler dependent functions.

 \author Jon A. Lambert
 \date 01/16/2005
 \version 0.1
 \remarks
  This source code copyright (C) 2005 by Jon A. Lambert
  All rights reserved.

  Released under the same terms of DikuMud
*/

#include "os.h"


#if defined WIN32
/*
  Windows fgets depends on finding strings terminated
  by CRLF sequence.  This one will operate like unixes
  and depend on LF terminating string.

 \remarks
 If the user coverts their files to dos/windows style
 line endings intentionally or unintentionally then
 all bets are off.
 */
char * fgets_win (char *buf, int n, FILE * fp)
{
  int c;
  char *s;

  if ((n < 2) || (buf == NULL))
    return NULL;

  s = buf;
  while (--n > 0 && (c = getc (fp)) != EOF) {
    *s++ = c;
    if (c == '\n')
      break;
  }
  if (c == EOF && s == buf)
  {
    return NULL;
  }
  *s = 0;
  return buf;
}

/*
  Not implemented in windows, although all the structural
  support is found in winsock.h

  \remarks
   This could use the finer granularity of GetSystemTime.
 */
void gettimeofday (struct timeval *tp, struct timezone *tzp)
{
  tp->tv_sec = time (NULL);
  tp->tv_usec = 0;
}
#endif

