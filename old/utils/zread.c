#include "sbsat.h"

int check_gzip (char *filename) {
  int is_gzip=0;
  FILE *fin;
  if (!strcmp(filename, "-")) fin = stdin;
  else  fin = fopen (filename, "rb");
  if (fin) {
    unsigned char x1 = (unsigned char)fgetc (fin);
    unsigned char x2 = (unsigned char)fgetc (fin);
    unsigned char x3 = (unsigned char)fgetc (fin);
    if (x1 == (unsigned char)'\037' &&
	x2 == (unsigned char)'\213') is_gzip=1;
    else if (x1 == (unsigned char)'B' &&
	     x2 == (unsigned char)'Z' &&
	     x3 == (unsigned char)'h'
	     ) is_gzip=2;
    ungetc(x3, fin);
    ungetc(x2, fin);
    ungetc(x1, fin);
    if (fin != stdin) fclose (fin);
  }
  return is_gzip;
}

FILE* zread(char *filename, int zip) {
  FILE *infile;
  char cmd[256];
  
  switch(zip) {
  case 1: strcpy(cmd, "gzip -dc "); break;
  case 2: strcpy(cmd, "bzip2 -dc "); break;
  default: fprintf(stderr, "Unknown compression method\n"); break;
  }
  
  if (!strcmp(filename, "-")) {
    fprintf(stderr, "Can not accept zipped data on the standard input\n");
    fprintf(stderr, "Please use the filename.gz as a parameter or %s for the stdin instead\n", cmd);
    fprintf(stderr, "Example: cat filename.gz | %s | sbsat\n", cmd);
    exit(1);
  }
  
  strncat(cmd, filename, sizeof(cmd)-strlen(cmd)-1);
  infile = popen(cmd, "r");  /* use "w" for zwrite */
  if (infile == NULL) {
    fprintf(stderr, "popen('%s', 'r') failed\n", cmd);
    exit(1);
  }
  int c = fgetc(infile);
  if (c == EOF) {
    fprintf(stderr, "Can't use %s\n", cmd);
    exit(1);
  }
  ungetc(c, infile);
  
  return infile;
}


