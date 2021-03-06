/* ************************************************************************
*  file: signals.c , trapping of signals from Unix.       Part of DIKUMUD *
*  Usage : Signal Trapping.                                               *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/time.h>

#include "utils.h"

void checkpointing(int signum);
void shutdown_request(int signum);
void logsig(int signum);
void hupsig(int signum);

void signal_setup(void)
{
	struct itimerval itime;
	struct timeval interval;

	signal(SIGUSR2, shutdown_request);

	/* just to be on the safe side: */

	signal(SIGHUP, hupsig);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGINT, hupsig);
	signal(SIGALRM, logsig);
	signal(SIGTERM, hupsig);

	/* set up the deadlock-protection */

	interval.tv_sec = 900;    /* 15 minutes */
	interval.tv_usec = 0;
	itime.it_interval = interval;
	itime.it_value = interval;
	setitimer(ITIMER_VIRTUAL, &itime, 0);
	signal(SIGVTALRM, checkpointing);
}



void checkpointing(int signum)
{
	extern int tics;
	
	if (!tics)
	{
		log("CHECKPOINT shutdown: tics not updated");
		abort();
	}
	else
		tics = 0;
}




void shutdown_request(int signum)
{
	extern int diku_shutdown;

	log("Received USR2 - shutdown request");
	diku_shutdown = 1;
}



/* kick out players etc */
void hupsig(int signum)
{
	extern int diku_shutdown;

	log("Received SIGHUP, SIGINT, or SIGTERM. Shutting down");
	exit(0);   /* something more elegant should perhaps be substituted */
}



void logsig(int signum)
{
	log("Signal received. Ignoring.");
}
