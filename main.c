#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
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

#define B(x) ((x))
#define KB(x) (1024*(B(x)))
#define MB(x) (1024*(KB(x)))

#include "utils.c"

#define H1Index 0
#define H2Index 1
#define H3Index 2
#define H4Index 3

/*
 * comment keywords ref:
 * @todo: things to do
 * @note: a general note
 * @done: tasks done
 */

// @todo: 
// Numbered Lists
// Memory Arenas and Functions
// Custom components
// Custom HTML Headers
// Table Support, through 
// Error Logging: 
//  single, multi-line code, format specifiers not ended properly

typedef struct FileDetails {
  char *FilePath; // Path and Name of file open
  I32 CurrLine; // Line currently being read
} FileDetails;

typedef struct MdConverter {
  I8 FmtIndex;
  B8 UlStarted;
  B8 LiStarted;
  B8 FmtStarted;
  B8 ParaStarted;
  FileDetails File;
} MdConverter;

typedef struct GlobalState {
  char *Src;
  char *Dest;
  char *HeaderTag;
  char *NavBarComp;
} GlobalState;

void ReadDirectoryRecursively(char *SrcDir, char *DestDir, GlobalState *state)
{
  DIR *DirPointer = opendir(SrcDir);
  if (DirPointer != NULL)
  {

    struct stat st = {0};
    if (stat(DestDir, &st) == -1)
    {
      mkdir(DestDir, 0700);
    }
    struct dirent *FilePointer;
    do {
      FilePointer = readdir(DirPointer);
      if (FilePointer == NULL) {
        break;
      }
      if (*FilePointer->d_name == '.')
      {
        /* @note: this is supposed to handle special folders
         * .
         * ..
         * .somename
         * I will be ignoring those and not going into them
         * This is good for me since those are usually some internal structures
         * that should not be used in static files
        */
        continue;
      }
      if (FilePointer->d_type == DT_REG)
      {
        // IF FILE:
        // if FilePointer.d_name has .md: parse
        char *FilePath = malloc(KB(4));
        char *_file_path_ptr = FilePath;
        QCopyStringMoveDest(SrcDir, &_file_path_ptr);
        QCopyStringMoveDest(FilePointer->d_name, &_file_path_ptr);
        *_file_path_ptr = 0;

        B8 IsMd = QFindSubStr(".md", &FilePath);
        if (!IsMd)
        {
          // @note: file is not md, so we just read and copy it to dest
          FILE *IndexFp = fopen(FilePath, "rb");
          fseek(IndexFp, 0, SEEK_END );
          long size = ftell(IndexFp);
          fseek(IndexFp, 0, SEEK_SET );

          char *IndexStr = calloc(1, MB(50));
          fread(IndexStr, 1, size, IndexFp);

          // translate the basic markdown to html
          // ------------------------------------
          // > read and convert buffer file line by line
          char *OutputStr = calloc(1, MB(50));

          char *CurrentChar = IndexStr;
          char *OutputChar = OutputStr;
          // need to copy all bytes till size
          I32 i = 0;
          while (i++ < size)
          {
            *OutputChar++ = *CurrentChar++;
          }
          // write translated file to dest folder
          I32 OutputFpLen = 0;
          char *OutputFilePath = malloc(KB(4));
          OutputFpLen += AppendToPath(&OutputFilePath, DestDir, FilePointer->d_name, 0);
          FILE *WriteFp = fopen(OutputFilePath, "wb");
          size_t StuffWritten = fwrite(OutputStr, 1, size, WriteFp);

          fclose(WriteFp);
          fclose(IndexFp);
          // cleanup
          free(OutputFilePath);
          free(OutputStr);
          free(IndexStr);
          free(FilePath);
        }
        else
        {
          FILE *IndexFp = fopen(FilePath, "r");
          fseek(IndexFp, 0, SEEK_END );
          long size = ftell(IndexFp);
          fseek(IndexFp, 0, SEEK_SET );

          char *IndexStr = calloc(1, MB(1));
          fread(IndexStr, 1, size, IndexFp);

          char *OutputStr = calloc(1, MB(1));
          long OutputSz = 0;

          char *CurrentChar = IndexStr;
          char *OutputChar = OutputStr;

          FileDetails File = {.CurrLine=0,.FilePath=FilePath};
          MdConverter MdState = {
            .FmtIndex=-1, // Format Mappings Index
            .FmtStarted=0, // Is a Format Started
            .UlStarted=0, // Is an Unordered List started
            .LiStarted=0, // Is a list element started
            .ParaStarted=0, // Is a paragraph started
            .File=File,
          };
          /* @note
           * HeaderMappings, FormatMappings, are arranged in an order
           * this is simple, there is an incremental order, which makes
           * sense to treat them as an array and just have the mappings set
           */

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
          while (*CurrentChar != 0)
          {
            // ****** HEADINGS *********
            if (*CurrentChar == '#')
            {
              // check if paragraph should end
              if (MdState.ParaStarted)
              {
                // @note: the only condition alongside 2 newlines where we need to end a paragraph
                OutputSz += QCopyStringMoveDest("</p>", &OutputChar);
                MdState.ParaStarted = 0;
              }
              // @note: Parse Headings, independant
              B8 IsValidHeading = 1;
              I8 HeaderIndex = 0;
              B8 StartedDec = 0;
              char TmpBuffer[4];
              while(*CurrentChar++ != '\n')
              {
                if (IsValidHeading)
                {
                  if (*CurrentChar == '#' && HeaderIndex < 4)
                  {
                    TmpBuffer[HeaderIndex] = '#';
                    ++HeaderIndex;
                  }
                  else if (StartedDec == 0 && *CurrentChar == ' ')
                  {
                    OutputSz += QCopyStringMoveDest(HeaderMappings[HeaderIndex][0], &OutputChar);
                    StartedDec = 1;
                  }
                  else if (*CurrentChar == '\n')
                  {
                    OutputSz += QCopyStringMoveDest(HeaderMappings[HeaderIndex][1], &OutputChar);
                    HeaderIndex = -1;

                    *OutputChar++ = *CurrentChar;
                    ++OutputSz;
                  }
                  else if (StartedDec == 0) // if # ends and we dont get a space then this heading is incorrect
                  {
                    // once we enter this loop, it will prevent reaching here for that line
                    // none of the re-assignment will be repeated
                    // you'd have to corrupt my memory for that to happen, so maybe in a rare solar event
                    IsValidHeading = 0;
                    if (MdState.ParaStarted == 0)
                    {
                      // @note if paragraph is not started, then begin
                      OutputSz += QCopyStringMoveDest("<p>", &OutputChar);
                      MdState.ParaStarted = 1;
                    }
                    OutputSz += QCopyStringMoveDest(TmpBuffer, &OutputChar);
                    *OutputChar++ = *CurrentChar;
                    ++OutputSz;
                  }
                  else
                  {
                    *OutputChar++ = *CurrentChar;
                    ++OutputSz;
                  }
                }
                else
                {
                  // it was not a valid heading, we've been duped
                  // just copy it as usual
                  *OutputChar++ = *CurrentChar;
                  ++OutputSz;
                }
              };
            }
            // ********* LINKS/URLS ***********
            else if (*CurrentChar == '[')
            {
              if (MdState.ParaStarted == 0)
              {
                // @note if paragraph is not started, then begin
                OutputSz += QCopyStringMoveDest("<p>", &OutputChar);
                MdState.ParaStarted = 1;
              }
              // search ahead until a \n, and see if user wanted a valid link 
              char *LookAhead = CurrentChar + 1;
              B8 SquareClosed = 0;
              B8 IsValidLink = 0;
              // html format to convert to
              // <a href="${link_url}">link text</a>
              char *LinkBlock = calloc(2, MB(1));
              char *LinkText = LinkBlock;
              I32 TextSz = 0;
              // program will only allow link text to be 1024 bytes OR 1024 characters
              // not the same as word limit, I don't care about that, nor will I implement that
              // I can just increase memory if that ever happens
              char *LinkUrl = LinkBlock + 1024;
              I32 UrlSz = 0;
              while (1) {
                if (*LookAhead == '\n')
                {
                  break;
                }
                else if (*LookAhead == ']')
                {
                  // OKAY, so the square bracket closes
                  if (*(LookAhead+1) == '(')
                  {
                    // Great, we also have a `(`
                    SquareClosed = 1;
                    LookAhead += 2;
                    continue;
                  }
                  else
                  {
                    // not a valid link
                    break;
                  }
                }
                else if (SquareClosed)
                {
                  // if the square bracket -> [] followed by ( part is good we check if the ) bracket comes
                  if (*LookAhead == ')')
                  {
                    // it comes, link is actually valid and we can convert it to html link, huzzah!
                    IsValidLink = 1;
                    ++LookAhead;
                    break;
                  }
                  else if (*LookAhead == ' ')
                  {
                    // any space in the () is invalid and we break it
                    break;
                  }
                  // now there is no else, because there is another if condition below to deal with char copy
                }
                if (SquareClosed == 0)
                {
                  // we are still [ in this part ...] of the link, squares still not closed
                  if (TextSz < 1023)
                  {
                    // text size below 1023, so we will keep going and copy chars
                    // IF NOT: we don't get inside here and just truncate the text
                    *LinkText++ = *LookAhead;
                    ++TextSz;
                  }
                  ++LookAhead;
                }
                else
                {
                  // we are [not here](in this part ...) of the link
                  if (UrlSz < 1023)
                  {
                    // text size below 1023, so we will keep going and copy chars
                    // IF NOT: we don't get inside here and just truncate the text
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
                // Move Ptr to LookAhead since its done reading the link
                CurrentChar = LookAhead;
              }
              else
              {
                // incase link is not valid
                // move the pointer ahead since I just read the original `[`
                // that got me in this condition
                *OutputChar++ = *CurrentChar++;
                ++OutputSz;
              }
              free(LinkBlock);
            }
            else if (*CurrentChar == ' ' && *(CurrentChar+1) == ' ')
            {
              // ************ LINE BREAKS **************
              if (MdState.ParaStarted == 0)
              {
                // @note if paragraph is not started, then begin
                OutputSz += QCopyStringMoveDest("<p>", &OutputChar);
                MdState.ParaStarted = 1;
              }
              // if there are two space found
              // add a break tag
              OutputSz += QCopyStringMoveDest("<br>", &OutputChar);
              CurrentChar+=2;
            }
            else if (*(CurrentChar-1) == '\n' && *CurrentChar == '-' && *(CurrentChar+1) == ' ')
            {
              // ************** UNORDERED LIST **********************
              if (MdState.ParaStarted == 0)
              {
                // @note if paragraph is not started, then begin
                OutputSz += QCopyStringMoveDest("<p>", &OutputChar);
                MdState.ParaStarted = 1;
              }
              if (MdState.UlStarted == 0)
              {
                OutputSz += QCopyStringMoveDest(UlMapping[0], &OutputChar);
                MdState.UlStarted = 1;
              }
              OutputSz += QCopyStringMoveDest(LiMapping[0], &OutputChar);
              CurrentChar += 2;
              MdState.LiStarted = 1;
            }
            else if (*CurrentChar == '*')
            {
              // ****************** FORMAT SPECIFIERS *******************
              if (MdState.ParaStarted == 0)
              {
                // @note if paragraph is not started, then begin
                OutputSz += QCopyStringMoveDest("<p>", &OutputChar);
              }
              // @todo: I dont know why I am just doing everything top level, since that is a horrible idea now that I see it.
              // I need to move the evaluation for this as well as the code copying inside here, in another loop
              ++MdState.FmtIndex;
              ++CurrentChar;
            }
            else if (MdState.FmtIndex > -1 && *CurrentChar != '*')
            {
              if (MdState.FmtStarted == 0)
              {
                OutputSz += QCopyStringMoveDest(FormatMappings[MdState.FmtIndex][0], &OutputChar);
                MdState.FmtStarted = 1;
              }
              else
              {
                OutputSz += QCopyStringMoveDest(FormatMappings[MdState.FmtIndex][1], &OutputChar);
                MdState.FmtStarted = 0;
              }
              MdState.FmtIndex = -1;
            }
            else if (*CurrentChar == '`')
            {
              // ************ CODE ***************
              if (MdState.ParaStarted == 0)
              {
                // @note if paragraph is not started, then begin
                OutputSz += QCopyStringMoveDest("<p>", &OutputChar);
                MdState.ParaStarted = 1;
              }
              B8 IsMultiline = 0;

              char *LookAhead = CurrentChar;
              char *LookAheadBuffer = calloc(1, MB(1));
              char *BuffPtr = LookAheadBuffer;

              // check if this is multi-line or inline
              if (*(LookAhead+1) == '`' && *(LookAhead+2) == '`')
              {
                IsMultiline = 1;
                LookAhead += 3; // move LookAhead after ```
                if (*(LookAhead) == '\n')
                {
                /* additional check for removing a new line
                 * thing is that, in writing markdown, you create a new line
                 * but that just keeps the newline when converting to html
                 * and you see an extra space in the starting of the code block so its just annoying
                */
                  ++LookAhead;
                }
              }
              else
              {
                ++LookAhead; // move LookAhead after `
              }

              // now check if until the file ends (yes I know), whether the code block ends, cause if it does not its not valid
              if (IsMultiline)
              {
                while (*(LookAhead+2) != 0)
                {
                  if (*LookAhead == '`' && *(LookAhead+1) == '`' && *(LookAhead+2) == '`')
                  {
                    LookAhead += 3; // move LookAhead after the ```
                    break;
                  }
                  *BuffPtr++ = *LookAhead++;
                }
              }
              else
              {
                while (*LookAhead != '\n' && *LookAhead != 0)
                {
                  if (*LookAhead == '`')
                  {
                    ++LookAhead; // move LookAhead after `
                    break;
                  }
                  *BuffPtr++ = *LookAhead++;
                }
              }

              /*
               * @note: so, I no longer care about validity. Since most markdown converters don't care, I don't either.
               * Everytime I see an invalid/incomplete code whether inline or block, I'll just autoclose it
               */
              if (IsMultiline)
              {
                OutputSz += QCopyStringMoveDest("<div class=\"multiline\">", &OutputChar);
                OutputSz += QCopyStringMoveDest(LookAheadBuffer, &OutputChar);
                OutputSz += QCopyStringMoveDest("</div>", &OutputChar);
                CurrentChar = LookAhead; // move current char to look ahead, as everything is now done here
              }
              else
              {
                OutputSz += QCopyStringMoveDest("<code>", &OutputChar);
                OutputSz += QCopyStringMoveDest(LookAheadBuffer, &OutputChar);
                OutputSz += QCopyStringMoveDest("</code>", &OutputChar);
                CurrentChar = LookAhead; // move current char to look ahead, as everything is now done here
              }
            }
            else if (*CurrentChar == '\n')
            {
              if (*(CurrentChar+1) == '\n' && MdState.ParaStarted)
              {
                // @note: the only condition alongside 2 newlines where we need to end a paragraph
                OutputSz += QCopyStringMoveDest("</p>", &OutputChar);
                MdState.ParaStarted = 0;
              } 

              // @todo: have code for list evaluation and conversion completely separate
              if (MdState.LiStarted == 1)
              {
                // closes a list element
                OutputSz += QCopyStringMoveDest(LiMapping[1], &OutputChar);
                MdState.LiStarted = 0;
              }
              else if (MdState.UlStarted == 1)
              {
                // in the case a list element was closen with \n
                // having this will close the list
                OutputSz += QCopyStringMoveDest(UlMapping[1], &OutputChar);
                MdState.UlStarted = 0;
              }
              *OutputChar++ = *CurrentChar++;
              ++OutputSz;
            }
            else 
            {
              if (MdState.ParaStarted == 0)
              {
                // @note if paragraph is not started, then begin
                OutputSz += QCopyStringMoveDest("<p>", &OutputChar);
                MdState.ParaStarted = 1;
              }
              *OutputChar++ = *CurrentChar++;
              ++OutputSz;
            }
          }
          if (MdState.ParaStarted == 1)
          {
            // @note if paragraph is not started, then begin
            OutputSz += QCopyStringMoveDest("</p>", &OutputChar);
            MdState.ParaStarted = 0;
          }
          OutputSz += QCopyStringMoveDest("</article>\n</body>\n</html>", &OutputChar);
          // write translated file to dest folder
          I32 OutputFpLen = 0;
          char *OutputFilePath = malloc(KB(4));
          OutputFpLen += AppendToPath(&OutputFilePath, DestDir, FilePointer->d_name, 0);
          // check if the source filename is .md
          // this will be converted to html
          // get the .md extension offet
          char *MdOffset = OutputFilePath + OutputFpLen - 3;
          if (QStrEqual(MdOffset,".md")) 
          {
            QCopyStringMoveDest(".html", &MdOffset);
            *MdOffset = 0;
          }
          FILE *WriteFp = fopen(OutputFilePath, "w");
          size_t StuffWritten = fwrite(OutputStr, 1, OutputSz, WriteFp);

          // cleanup
          fclose(WriteFp);
          fclose(IndexFp);

          free(OutputFilePath);
          free(OutputStr);
          free(IndexStr);
          free(FilePath);
        }
      }
      else if (FilePointer->d_type == DT_DIR)
      {
        // FilePointer pointing to a directory
        // IF DIRECTORY:
        char *SrcSubDir = malloc(KB(4));
        char *DestSubDir= malloc(KB(4));

        AppendToPath(&DestSubDir, DestDir, FilePointer->d_name, 1);
        struct stat subSt = {0};
        // open dir
        if (stat(DestSubDir, &subSt) == -1)
        {
          // create the same directory in dest path
          mkdir(DestSubDir, 0700);
        }
        // START
        // create the directory prefix
        // this will create the path where it adds the parent directory to the 
        AppendToPath(&SrcSubDir, SrcDir, FilePointer->d_name, 1);
        ReadDirectoryRecursively(SrcSubDir, DestSubDir, state);
        // END
        free(DestSubDir);
        free(SrcSubDir);
      }
      // === figure out
    } while (FilePointer != NULL);
    closedir(DirPointer);
  }
}

int main(int argc, char **argv)
{
  char *src = malloc(KB(4));
  char *dest = malloc(KB(4));


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
    "<link rel='icon' type='image/x-icon' href='/assets/favicon.ico.png' />\n"
    "<link rel='stylesheet' href='/styles.css' />\n"
  "</head>";
  char *NavBarComp = "<ul class='nav-bar'>\n"
  "<li><a href='/index.html'>Home</a></li>\n"
  "<li><a href='/blog/root.html'>Blog</a></li>\n"
  "<li><a href='/projects/root.html'>Projects</a></li>\n"
  "<li><a href='/work/root.html'>Work</a></li>\n"
  "<li><a href='/assets/resume.pdf' target='_blank'>Resume</a></li>\n"
  "</ul>";
  GlobalState state = {0};
  state.Src = src;
  state.Dest = dest;
  state.HeaderTag = HeaderTag;
  state.NavBarComp = NavBarComp;

  ReadDirectoryRecursively(src, dest, &state);
  // cleanup
  free(dest);
  free(src);
  return 1;
}
