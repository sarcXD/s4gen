#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>

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

typedef struct GlobalState{
  char *Src;
  char *Dest;
  char *HeaderTag;
  char *NavBarComp;
} GlobalState;

void ReadDirectoryRecursively(char *DirPath, GlobalState *state)
{
  /**
   * for a basic v1
   * I will only focus on markdown parsing
   */
  DIR *DirPointer = opendir(DirPath);
  if (DirPointer != NULL)
  {
    struct dirent *FilePointer;
    do {
      FilePointer = readdir(DirPointer);
      if (*FilePointer->d_name == '.')
      {
        continue;
      }
      if (FilePointer->d_type == DT_REG)
      {
        // IF FILE:
        // if FilePointer.d_name has .md: parse
        char *FilePath = malloc(256);
        char *_file_path_ptr = FilePath;
        QCopyStringMoveDest(DirPath, &_file_path_ptr);
        QCopyString(FilePointer->d_name, &_file_path_ptr);

        FILE *IndexFp = fopen(FilePath, "r");
        fseek(IndexFp, 0, SEEK_END );
        long size = ftell(IndexFp);
        fseek(IndexFp, 0, SEEK_SET );

        char *IndexStr = malloc(size);
        fread(IndexStr, 1, size, IndexFp);

        // translate the basic markdown to html
        // ------------------------------------
        // > read and convert buffer file line by line

        long ScaledSize = size*2;
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
        /**
         * Header Mappings ordering:
         * Header tag, start, end
         */
        char *HeaderMappings[4][2] = {
          {"<h1>", "</h1>"}, // #
          {"<h2>", "</h2>"}, // ##
          {"<h3>", "</h3>"}, // ###
          {"<h4>", "</h4>"}, // ####
        };

        char *FormatMappings[3][2] = {
          {"<em>", "</em>"},                  // *
          {"<strong>", "</strong>"},          // **
          {"<em><strong>", "</strong></em>"}  // ***
        };

        char *UlMapping[2] = {"<ul>", "</ul>"};
        char *LiMapping[2] = {"<li>", "</li>"};

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
        // Copy over the html header tag
        // see: HeaderTag
        OutputSz += QCopyStringMoveDest(state->HeaderTag, &OutputChar);
        // Copy over the custom navbar component
        // see: NavBarComp
        OutputSz += QCopyStringMoveDest(state->NavBarComp, &OutputChar);

        OutputSz += QCopyStringMoveDest("<body>", &OutputChar);
        OutputSz += QCopyStringMoveDest("<article class=\"article-ctn\">", &OutputChar);
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
          else if (*CurrentChar == '[')
          {
            // search ahead until a \n, and see if user wanted a valid link 
            char *LookAhead = CurrentChar + 1;
            B8 SquareClosed = 0;
            B8 IsValidLink = 0;
            // html format to convert to
            // <a href="${link_url}">link text</a>
            char *LinkBlock = malloc(sizeof(char)*2048);
            char *LinkText = LinkBlock;
            I32 TextSz = 0;
            char *LinkUrl = LinkBlock + 1024;
            I32 UrlSz = 0;

            while (1) {
              if (*LookAhead == '\n')
              {
                break;
              }
              else if (*LookAhead == ']')
              {
                if (*(LookAhead+1) == '(')
                {
                  SquareClosed = 1;
                  LookAhead += 2;
                  continue;
                }
                else
                {
                  break;
                }
              }
              else if (SquareClosed)
              {
                if (*LookAhead == ')')
                {
                  IsValidLink = 1;
                  ++LookAhead;
                  break;
                }
                else if (*LookAhead == ' ')
                {
                  break;
                }
              }
              if (SquareClosed == 0)
              {
                if (TextSz < 1023)
                {
                  *LinkText++ = *LookAhead;
                  ++TextSz;
                }
                ++LookAhead;
              }
              else
              {
                if (UrlSz < 1023)
                {
                  *LinkUrl++ = *LookAhead;
                  ++UrlSz;
                }
                ++LookAhead;
              }
            }
            if (IsValidLink)
            {
              // =========== parse: <a href="${link_url}"> ===========
              int BytesWritten = 0;
              BytesWritten = QCopyStringMoveDest("<a href=\"", &OutputChar);
              OutputSz += BytesWritten;
              char *TextOffset = LinkBlock;
              char *UrlOffset = LinkBlock + 1024;
              BytesWritten = QCopyStringMoveDest(UrlOffset, &OutputChar);
              OutputSz += BytesWritten;
              *OutputChar++ += '"'; 
              *OutputChar++ += '>';
              OutputSz += 2;
              // =========== parse: link text</a> ==================
              BytesWritten = QCopyStringMoveDest(TextOffset, &OutputChar);
              OutputSz += BytesWritten;
              BytesWritten = QCopyStringMoveDest("</a>", &OutputChar);
              OutputSz += BytesWritten;
              // Move Ptr
              CurrentChar = LookAhead;
            }
            else
            {
              *OutputChar++ = *CurrentChar++;
              ++OutputSz;
            }
            free(LinkBlock);
          }
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
        OutputSz += QCopyStringMoveDest("</article>\n</body>\n</html>", &OutputChar);
        long OutLen = strlen(OutputStr);
        closedir(DirPointer);
        // write translated file to dest folder
        // @fix
        FILE *WriteFp = fopen("./index.html", "w");
        fwrite(OutputStr, 1, OutputSz, WriteFp);

        // cleanup
        free(OutputStr);
        free(IndexStr);
        free(FilePath);
        return;
      }
      else if (FilePointer->d_type == DT_DIR)
      {
        // else: copy file to dest location
        // IF DIRECTORY:
        // open dir
        char *DirName = malloc(256*sizeof(char));
        // START
        // create the directory prefix
        // this will create the path where it adds the parent directory to the 
        // dir path
        char *_dir_ptr = DirName;
        QCopyStringMoveDest(DirPath, &_dir_ptr);
        // check if previous index has slash
        if (*(_dir_ptr-1) != '/') {
          *_dir_ptr++ = '/';
        }
        QCopyString(FilePointer->d_name, &_dir_ptr);
        ReadDirectoryRecursively(DirName, state);
        // END
        free(DirName);
      }
      // === figure out
    } while (FilePointer != NULL);
    closedir(DirPointer);
  }
}

int main(int argc, char **argv)
{
  char *src = malloc(256);
  char *dest = malloc(256);


  while (*++argv != 0) 
  {
    if (QStrEqual(*argv, "--src")) 
    {
      char *SrcPtr = src;
      QCopyStringMoveDest(*++argv, &SrcPtr);
      // handling the case where slashes at end excluded
      if (*(SrcPtr-1) != '/') *SrcPtr = '/';
      printf("Src path is %s\n", src);
    }
    else if (QStrEqual(*argv, "--dest"))
    {
      char *DestPtr = dest;
      QCopyStringMoveDest(*++argv, &DestPtr);
      // handling the case where slashes at end excluded
      if (*(DestPtr-1) != '/') *DestPtr = '/';
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


  char *HeaderTag = "<!DOCTYPE html>\n"
  "<html>\n<head>\n"
    "<title>Talha Aamir</title>\n"
    "<link rel='icon' type='image/x-icon' href='./assets/favicon.ico.png' />\n"
    "<link rel='stylesheet' href='./styles.css' />\n"
  "</head>";
  char *NavBarComp = "<ul class='nav-bar'>\n"
  "<li><a href='./index.html'>Home</a></li>\n"
  "<li><a href='./blog/root.html'>Blog</a></li>\n"
  "<li><a href='./projects/root.html'>Projects</a></li>\n"
  "<li><a href='./work/root.html'>Work</a></li>\n"
  "<li><a href='./assets/resume.pdf' target='_blank'>Resume</a></li>\n"
  "</ul>";
  GlobalState state = {0};
  state.Src = src;
  state.Dest = dest;
  state.HeaderTag = HeaderTag;
  state.NavBarComp = NavBarComp;

  ReadDirectoryRecursively(src, &state);
  // go to source folder and get a handle to the directory
  // cleanup
  free(dest);
  free(src);
  return 1;
}
