/* Present a message on a port */

#include "os.h"

void watch (int port, char *text);
void wave (int sock, char *text);
int new_connection (int s);
int init_socket (int port);
int write_to_descriptor (int desc, char *txt);
void nonblock (int s);




int main (int argc, char **argv)
{
  int port;
  char txt[2048], buf[83];
  FILE *fl;

  if (argc != 3) {
    fputs ("Usage: sign (<filename> | - ) <port #>\n", stderr);
    exit (1);
  }
  if (!strcmp (argv[1], "-")) {
    fl = stdin;
    puts ("Input text (terminate with ^D)");
  } else if (!(fl = fopen (argv[1], "rb"))) {
    perror (argv[1]);
    exit (1);
  }
  for (;;) {
    FGETS (buf, 81, fl);
    if (feof (fl))
      break;
    strcat (buf, "\r");
    if (strlen (buf) + strlen (txt) > 2048) {
      fputs ("String too long\n", stderr);
      exit (1);
    }
    strcat (txt, buf);
  }
  if ((port = atoi (argv[2])) <= 1024) {
    fputs ("Illegal port #\n", stderr);
    exit (1);
  }
  WIN32STARTUP
  watch (port, txt);
  WIN32CLEANUP
  return 0;
}




void watch (int port, char *text)
{
  int mother;
  fd_set input_set;

  mother = init_socket (port);

  FD_ZERO (&input_set);
  for (;;) {
    FD_SET (mother, &input_set);
    if (select (64, &input_set, 0, 0, 0) < 0) {
      perror ("select");
      WIN32CLEANUP
      exit (1);
    }
    if (FD_ISSET (mother, &input_set))
      wave (mother, text);
  }
}



void wave (int sock, char *text)
{
  int s;

  if ((s = new_connection (sock)) < 0)
    return;

  write_to_descriptor (s, text);
#if defined WIN32
  Sleep (6000);
#else
  sleep (6);
#endif
  close (s);
}



int new_connection (int s)
{
  struct sockaddr_in isa;
  size_t i;
  int t;
  char buf[100];

  i = sizeof (isa);

  if ((t = accept (s, (struct sockaddr*)&isa, &i)) < 0) {
    perror ("Accept");
    return (-1);
  }
  nonblock (t);

  return (t);
}






int init_socket (int port)
{
  int s;
  char *opt;
  char hostname[1024];
  struct sockaddr_in sa;
  struct hostent *hp;
  struct linger ld;

  bzero (&sa, sizeof (struct sockaddr_in));
  gethostname (hostname, 1023);
  hp = gethostbyname (hostname);
  if (hp == NULL) {
    perror ("gethostbyname");
    exit (1);
  }
  sa.sin_family = hp->h_addrtype;
  sa.sin_port = htons (port);
  s = socket (AF_INET, SOCK_STREAM, 0);
  if (s < 0) {
    perror ("Init-socket");
    exit (1);
  }
  if (setsockopt (s, SOL_SOCKET, SO_REUSEADDR,
      (char *) &opt, sizeof (opt)) < 0) {
    perror ("setsockopt REUSEADDR");
    exit (1);
  }

  ld.l_onoff = 1;
  ld.l_linger = 1000;
  if (setsockopt (s, SOL_SOCKET, SO_LINGER, (char *) &ld, sizeof (ld)) < 0) {
    perror ("setsockopt LINGER");
    exit (1);
  }
  if (bind (s, (struct sockaddr*)&sa, sizeof (sa)) < 0) {
    perror ("bind");
    close (s);
    exit (1);
  }
  listen (s, 5);
  return (s);
}




int write_to_descriptor (int desc, char *txt)
{
  int sofar, thisround, total;

  total = strlen (txt);
  sofar = 0;

  do {
    thisround = send (desc, txt + sofar, total - sofar, 0);
    if (thisround < 0) {
      perror ("Write to socket");
      return (-1);
    }
    sofar += thisround;
  }
  while (sofar < total);

  return (0);
}




void nonblock (int s)
{
#ifdef WIN32
  unsigned long flags = 1;

  if (ioctlsocket (s, FIONBIO, &flags)) {
#else
  if (fcntl (s, F_SETFL, FNDELAY) == -1) {
#endif
    perror ("Noblock");
    exit (1);
  }
}
