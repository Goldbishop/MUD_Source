/*! \file os.h
  The intent of this header file is to put operating system and compiler
  specific portability changes in one place.

 \author Jon A. Lambert
 \date 01/12/2005
 \version 0.3
 \remarks
  This source code copyright (C) 2004,2005 by Jon A. Lambert
  All rights reserved.

  Released under the same terms of DikuMud
*/

#ifndef OS_H
#define OS_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <signal.h>
#include <time.h>               /* Gentoo complains */
#include <sys/types.h>
#include <limits.h>
#include <fcntl.h>
#include <assert.h>

#ifdef WIN32                    /* Windows portability */

#define ARG_MAX (16384 - 256)
#define FD_SETSIZE 1024
#define NOFILE FD_SETSIZE
#include <winsock2.h>
#include <sys/stat.h>
#include <process.h>
#if defined __LCC__ || defined _MSC_VER
#include <direct.h>
#else
#include <dir.h>
#endif
#define EWOULDBLOCK       WSAEWOULDBLOCK
#undef EINTR
#undef EMFILE
#define EINTR             WSAEINTR
#define EMFILE            WSAEMFILE
#define GETERROR     WSAGetLastError()
#define WIN32STARTUP \
    { \
      WSADATA wsaData; \
      int err = WSAStartup(0x202,&wsaData); \
      if (err) \
        fprintf(stderr,"Error(WSAStartup):%d\n",err); \
    }
#define WIN32CLEANUP WSACleanup();
#define close(X) closesocket(X)
#define index(s, c) strchr((s), (c))
#define bcopy(s, d, n)   memcpy((d), (s), (n))
#define bzero(s, n)      memset((s), 0, (n))
#define getdtablesize() FD_SETSIZE
#define RAND rand
#define SRAND srand
#define crypt(s, r) (s)
#ifndef __LCC__     /* simply does not like our prototype ? */
void gettimeofday (struct timeval *tp, struct timezone *tzp);
#endif
#define FGETS fgets_win
char * fgets_win (char *buf, int n, FILE * fp);

#else /* Unix portability - some a consequence of above */

#include <sys/time.h>           /* Redhat BSD need this */
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/param.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <crypt.h>
#define GETERROR  errno
#define INVALID_SOCKET -1       /* 0 on Windows */
#define WIN32STARTUP
#define WIN32CLEANUP
#define RAND random
#define SRAND srandom
#define FGETS fgets

#endif

#if defined _POSIX_ARG_MAX
#define MAXCMDLEN _POSIX_ARG_MAX
#elif defined ARG_MAX
#define MAXCMDLEN ARG_MAX
#elif defined NCARGS
#define MAXCMDLEN  NCARGS
#else
#error Cannot determine maximum command argument size
#endif

#if !defined NOFILE
#if defined OPEN_MAX
#define NOFILE OPEN_MAX
#else
#error Cannot determine maximum open files
#endif
#endif

#endif /* OS_H */
