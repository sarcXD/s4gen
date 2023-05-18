#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define I8 int8_t
#define I16 int16_t
#define I32 int32_t
#define I64 int64_t
#define U8 uint8_t
#define U16 uint16_t
#define U32 uint32_t
#define U64 uint64_t
#define B8 U8
#define B32 I32

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
   * Header Mappings ordering:
   * Header tag, start, end
   */
  char *HeaderMappings[][2] = {
    {"<h1>", "</h1>"}, // #
    {"<h2>", "</h2>"}, // ##
    {"<h3>", "</h3>"}, // ###
    {"<h4>", "</h4>"}, // ####
  };

  char *FormatMappings[][2] = {
    {"<em>", "</em>"},                  // *
    {"<strong>", "</strong>"},          // **
    {"<em><strong>", "</strong></em>"}  // ***
  };

  char *UlMapping[2] = {"<ul>", "</ul>"};
  char *LiMapping[2] = {"<li>", "</li>"};

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

  // translate the basic markdown to html
  // ------------------------------------
  // > read and convert buffer file line by line

  long ScaledSize = size*1.5;
  char *OutputStr = malloc(ScaledSize);
  long OutputSz = 0;

  char *CurrentChar = IndexStr;
  char *OutputChar = OutputStr;

  int s = 0;
  int e = 1;

  int FmtWrapperInd = 1;

  int FmtIndex = -1;
  int HeaderIndex = -1;
  B8 UlStarted = 0;
  B8 LiStarted = 0;
  B8 FmtStarted = 0;
  B8 InlineCodeStarted = 0;
  /*
   * Ordering
   * #
   * ##
   * ###
   * ####
   * ^ these all take an entire line
   * *italic*
   * **bold**
   * - (an unsorted list)
   * */
  while (*CurrentChar != '\0')
  {
    if (*CurrentChar == '#')
    {
      // Headings Parser
      ++HeaderIndex;
      B8 StartedDec = 0;
      while(*CurrentChar++ != '\n')
      {
        if (*CurrentChar == '#')
        {
          ++HeaderIndex;
        }
        else if (StartedDec == 0 && *CurrentChar == ' ')
        {
          OutputSz += QCopyStringMoveDest(HeaderMappings[HeaderIndex][s], &OutputChar);
          StartedDec = 1;
        }
        else if (*CurrentChar == '\n')
        {
          OutputSz += QCopyStringMoveDest(HeaderMappings[HeaderIndex][e], &OutputChar);
          HeaderIndex = -1;

          *OutputChar++ = *CurrentChar;
          ++OutputSz;
        }
        else 
        {
          *OutputChar++ = *CurrentChar;
          ++OutputSz;
        }
      };
    }
    // link handling
    // else if (*CurrentChar == '[')
    // {
    //   // link handling
    //   *TmpBuffer++ = *CurrentChar++;
    //   while (1)
    //   {
    //     if (*CurrentChar
    //   }
    // }
    else if (*CurrentChar == ' ' && *(CurrentChar+1) == ' ')
    {
      // if there are two space found
      // add a break tag
      OutputSz += QCopyStringMoveDest("<br>", &OutputChar);
      CurrentChar+=2;
    }
    else if (*CurrentChar == '-' && *(CurrentChar+1) == ' ')
    {
      if (UlStarted == 0)
      {
        OutputSz += QCopyStringMoveDest(UlMapping[0], &OutputChar);
        UlStarted = 1;
      }
      OutputSz += QCopyStringMoveDest(LiMapping[0], &OutputChar);
      CurrentChar += 2;
      LiStarted = 1;
    }
    else if (*CurrentChar == '`')
    {
      if (InlineCodeStarted == 0)
      {
        OutputSz += QCopyStringMoveDest("<code>", &OutputChar);
        InlineCodeStarted = 1;
      }
      else
      {
        OutputSz += QCopyStringMoveDest("</code>", &OutputChar);
        InlineCodeStarted = 0;
      }
      ++CurrentChar;
    }
    else if (*CurrentChar == '*')
    {
      ++FmtIndex;
      ++CurrentChar;
    }
    else if (FmtIndex > -1 && *CurrentChar != '*')
    {
      // reach non star character
      // add starting tag corresponding to format specifier
      if (FmtStarted == 0)
      {
        OutputSz += QCopyStringMoveDest(FormatMappings[FmtIndex][0], &OutputChar);
        FmtStarted = 1;
      }
      else
      {
        OutputSz += QCopyStringMoveDest(FormatMappings[FmtIndex][1], &OutputChar);
        FmtStarted = 0;
      }
      FmtIndex = -1;
    }
    else if (*CurrentChar == '\n')
    {
      if (LiStarted == 1)
      {
        // closes a list element
        OutputSz += QCopyStringMoveDest(LiMapping[1], &OutputChar);
        LiStarted = 0;
      }
      else if (UlStarted == 1)
      {
        // in the case a list element was closen with \n
        // having this will close the list
        OutputSz += QCopyStringMoveDest(UlMapping[1], &OutputChar);
        UlStarted = 0;
      }
      *OutputChar++ = *CurrentChar++;
      ++OutputSz;
    }
    else 
    {
      *OutputChar++ = *CurrentChar++;
      ++OutputSz;
    }
  }

  // write translated file to dest folder
  FILE *WriteFp = fopen("./index.html", "w");
  fwrite(OutputStr, 1, OutputSz, WriteFp);

  // cleanup
  free(OutputStr);
  free(IndexStr);
  free(dest);
  free(src);
  return 1;
}
