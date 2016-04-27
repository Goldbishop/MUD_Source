#include "os.h"

#include "structs.h"

void do_it (FILE * src, FILE * trg, FILE * out);

int main (int argc, char **argv)
{
  FILE *src, *trg, *out;

  if (argc != 4) {
    fprintf (stderr, "Usage: %s source target ulit\n", argv[0]);
    return 0;
  } else if (!(src = fopen (argv[1], "rb"))) {
    fprintf (stderr, "%s: Could not open.\n", argv[1]);
    return 1;
  } else if (!(trg = fopen (argv[2], "rb"))) {
    fprintf (stderr, "%s: Could not open.\n", argv[2]);
    return 1;
  } else if (!(out = fopen (argv[3], "wb"))) {
    fprintf (stderr, "%s: Could not open.\n", argv[3]);
    return 1;
  } else {
    do_it (src, trg, out);
  }

  fclose (src);
  fclose (trg);
  fclose (out);
  return 0;
}


void do_it (FILE * src, FILE * trg, FILE * out)
{
  struct char_file_u inbuf1, inbuf2;
  int a, b, c, d, n;

  srand ((int) time (0));

  n = 1;

  for (;;) {
    fread (&inbuf1, sizeof (inbuf1), 1, trg);
    fread (&inbuf2, sizeof (inbuf2), 1, src);
    while (strcmp (inbuf1.name, inbuf2.name))
      fread (&inbuf2, sizeof (inbuf2), 1, src);


    if (feof (trg))
      break;
    /* do something */

    inbuf1.points.gold = inbuf2.points.gold;

    if (inbuf1.points.gold > 25000 * inbuf1.level)
      inbuf1.points.gold = 25000 * inbuf1.level;

    /* do something else */
    fwrite (&inbuf1, sizeof (inbuf1), 1, out);
  }

}
