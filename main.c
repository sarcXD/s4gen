#include <stdio.h>
#include <stdlib.h>

#define b32 int
#define Kb(x) (1024*x)
#define Mb(x) (1024*(Kb(x)))

#include "utils.c"

#define H1Index 0
#define H2Index 1
#define H3Index 2
#define H4Index 3

int main(int argc, char **argv)
{
  char *src = malloc(256);
  char *dest = malloc(256);

  /**
   * Md Mappings ordering:
   * md tag, start, end
   */
  char *MdMappings[][3] = {
    {"# ", "<h1>", "</h1>"},
    {"## ", "<h2>", "</h2>"},
    {"### ", "<h3>", "</h3>"},
    {"#### ", "<h4>", "</h4>"},
  };

  while (*++argv != 0) 
  {
    if (QStrEqual(*argv, "--src")) 
    {
      QCopyString(*++argv, &src);
      printf("Src path is %s\n", src);
    }
    else if (QStrEqual(*argv, "--dest"))
    {
      QCopyString(*++argv, &dest);
      printf("Dest path is %s\n", dest);
    }
    else
    {
      printf("Unrecognized argument: %s\n", *argv);
      free(dest);
      free(src);
      return -1;
    }
  }

  /**
   * for a basic v1
   * I will only focus on markdown parsing
   */

  // go to source folder and get a handle to the directory

  // load index.md file to read buffer

  FILE *IndexFp = fopen("./index.md", "r");
  fseek(IndexFp, 0, SEEK_END );
  long size = ftell(IndexFp);
  fseek(IndexFp, 0, SEEK_SET );

  char *IndexStr = malloc(size);
  fread(IndexStr, 1, size, IndexFp);
  printf("the file:\n%s\n", IndexStr);
  free(IndexStr);

  // translate the basic markdown to html

  // write translated file to dest folder

  // cleanup
  free(dest);
  free(src);
  return 1;
}
