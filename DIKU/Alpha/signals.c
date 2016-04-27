/* ************************************************************************
*  file: signals.c , trapping of signals from Unix.       Part of DIKUMUD *
*  Usage : Signal Trapping.                                               *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include "os.h"

#include "structs.h"
#include "utils.h"
#include "prototypes.h"

void checkpointing (int);
void shutdown_request (int);
void logsig (int);
void hupsig (int);

void signal_setup (void)
{
#if !defined WIN32
  struct itimerval itime;
  struct timeval interval;
#endif

#if !defined __DMC__ && !defined _MSC_VER && !defined __LCC__
  signal (SIGUSR2, shutdown_request);
#endif

  /* just to be on the safe side: */

  signal (SIGINT, hupsig);
  signal (SIGTERM, hupsig);
#if !defined WIN32
  signal (SIGHUP, hupsig);
  signal (SIGPIPE, SIG_IGN);
  signal (SIGALRM, logsig);
#endif

  /* set up the deadlock-protection */

#if !defined WIN32
  interval.tv_sec = 900;        /* 15 minutes */
  interval.tv_usec = 0;
  itime.it_interval = interval;
  itime.it_value = interval;
  setitimer (ITIMER_VIRTUAL, &itime, 0);
  signal (SIGVTALRM, checkpointing);
#endif
}



void checkpointing (int sig)
{
  extern int tics;

  if (!tics) {
    log ("CHECKPOINT shutdown: tics not updated");
    abort ();
  } else
    tics = 0;
}




void shutdown_request (int sig)
{
  extern int shutdown_server;

  log ("Received USR2 - shutdown request");
  shutdown_server = 1;
}



/* kick out players etc */
void hupsig (int sig)
{
  extern int shutdown_server;

  log ("Received SIGHUP, SIGINT, or SIGTERM. Shutting down");
  WIN32CLEANUP
  exit (0);                     /* something more elegant should perhaps be substituted */
}



void logsig (int sig)
{
  log ("Signal received. Ignoring.");
}


void block_signals(void)
{
  signal (SIGINT, SIG_IGN);
  signal (SIGTERM, SIG_IGN);
#if !defined __DMC__ && !defined _MSC_VER && !defined __LCC__
  signal (SIGUSR1, SIG_IGN);
  signal (SIGUSR2, SIG_IGN);
#endif
#if !defined WIN32
  signal (SIGPIPE, SIG_IGN);
  signal (SIGXCPU, SIG_IGN);
  signal (SIGVTALRM, SIG_IGN);
  signal (SIGALRM, SIG_IGN);
  signal (SIGURG, SIG_IGN);
  signal (SIGHUP, SIG_IGN);
#endif
}

void restore_signals(void)
{
  signal (SIGINT, SIG_DFL);
  signal (SIGTERM, SIG_DFL);
#if !defined __DMC__ && !defined _MSC_VER && !defined __LCC__
  signal (SIGUSR1, SIG_DFL);
  signal (SIGUSR2, SIG_DFL);
#endif
#if !defined WIN32
  signal (SIGPIPE, SIG_DFL);
  signal (SIGXCPU, SIG_DFL);
  signal (SIGVTALRM, SIG_DFL);
  signal (SIGALRM, SIG_DFL);
  signal (SIGURG, SIG_DFL);
  signal (SIGHUP, SIG_DFL);
#endif
  signal_setup ();
}

