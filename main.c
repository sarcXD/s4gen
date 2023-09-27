#include "amr_strings.h"
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
#include "amr_strings.c"

#define H1Index 0
#define H2Index 1
#define H3Index 2
#define H4Index 3

#define MAX_PATH_LEN 256
/*
 * comment keywords ref:
 * @todo: things to do
 * @note: a general note
 * @done: tasks done
 */

// @todo: 
// maybe make everything more stateful based on the MdConverter, so I dont have to rely on previous ifs or
// changes that happened in a previous loop to understand what to do, huge pain
// fix random paragraph tags on newlines
// Images
// Memory Arenas and Functions
// Custom HTML Headers
// Custom components
// Table Support
// Error Logging: 
//  single, multi-line code, format specifiers not ended properly

typedef struct FileDetails {
  I32 CurrLine; // Line currently being read
  char *FilePath; // Path and Name of file open
} FileDetails;

typedef struct MdConverter {
  I8 FmtIndex;
  B8 UlStarted;
  B8 OlStarted;
  B8 LiStarted;
  B8 FmtStarted;
  B8 ParaStarted;
  FileDetails File;
} MdConverter;

struct global_state {
  struct amrs_string src_path;
  struct amrs_string dest_path;
  struct amrs_string header_tag;
  struct amrs_string navbar_component;
};

#define NUM_START 48
#define NUM_END 57

// check if a character is a valid number
B8 IsStrNumber(char *X)
{
  char *iter = X;
  while (*iter != 0)
  {
    I32 Conv = (I32)*iter;
    if (Conv < NUM_START || Conv > NUM_END) // check if this is not within number ascii range
    {
      return 0;
    }
    ++iter;
  }
  return 1;
}

B8 IsCharNumber(char X)
{
  B8 valid = X >= NUM_START && X <= NUM_END; // check if this is not within number ascii range
  return valid;
}

B8 IsValidOrderedList(char *X)
{
  B8 valid = 0;
  char *LookAhead = X+1; // set look ahead after newline
  while (*LookAhead != '\n' && *LookAhead != '.')
  {
    if (IsCharNumber(*LookAhead))
    {
      valid = 1;
    }
    else
    {
      valid = 0;
      break;
    }
    ++LookAhead;
  }

  if (valid)
  {
      return valid;
  }

  if (*LookAhead == '.' && *(LookAhead+1) == ' ')
  {
    valid = 1;
  }

  return valid;
}

#if 0
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
            .FmtIndex=-1,     // Format Mappings Index
            .FmtStarted=0,    // Is a Format Started?
            .UlStarted=0,     // Is an Unordered List started?
            .OlStarted=0,     // Is an ordered list started?
            .LiStarted=0,     // Is a list element started?
            .ParaStarted=0,   // Is a paragraph started?
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
          char *OlMapping[2] = {"<ol>", "</ol>"};
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
              while(*CurrentChar != '\n' && *CurrentChar != 0)
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
                  else if (StartedDec == 0) // if # ends and we dont get a space then this heading is incorrect
                  {
                    // once we enter this loop, it will prevent reaching here for that line
                    // none of the re-assignment will be repeated
                    // you'd have to corrupt my memory for that to happen, so maybe in a rare solar event
                    IsValidHeading = 0;
                    // @todo: fix this if condition for list check.
                    // also what the hell is going on with the progression here even
                    if (MdState.UlStarted == 0 && MdState.ParaStarted == 0)
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
                ++CurrentChar;
              };
              if (*CurrentChar == '\n')
              {
                OutputSz += QCopyStringMoveDest(HeaderMappings[HeaderIndex][1], &OutputChar);
                HeaderIndex = -1;
              }
            }
            // ********* LINKS/URLS ***********
            else if (*CurrentChar == '[')
            {
              if (MdState.UlStarted == 0 && MdState.ParaStarted == 0)
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
              if (MdState.UlStarted == 0 && MdState.ParaStarted == 0)
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
            else if (*CurrentChar == '*')
            {
              // ****************** FORMAT SPECIFIERS *******************
              if (MdState.UlStarted == 0 && MdState.ParaStarted == 0)
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
              if (MdState.UlStarted == 0 && MdState.ParaStarted == 0)
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
              // then check another newline
              if (*(CurrentChar+1) == '\n')
              {
                // @note: if a list is started, give preferance to list closing, since that also has 2 \n chars
                // to indicate ending a list
                if (MdState.LiStarted == 1)
                {
                  // first check for li ending
                  // closes a list element
                  OutputSz += QCopyStringMoveDest(LiMapping[1], &OutputChar);
                  MdState.LiStarted = 0;
                }
                if (MdState.UlStarted == 1)
                {
                  OutputSz += QCopyStringMoveDest(UlMapping[1], &OutputChar);
                  MdState.UlStarted = 0;
                }
                else if (MdState.OlStarted == 1)
                {
                  OutputSz += QCopyStringMoveDest(OlMapping[1], &OutputChar);
                  MdState.OlStarted = 0;
                }
                else if (MdState.ParaStarted)
                {
                  // check if paragraph shoul end
                  // @note: the only condition alongside 2 newlines where we need to end a paragraph
                  OutputSz += QCopyStringMoveDest("</p>", &OutputChar);
                  MdState.ParaStarted = 0;
                } 
                // add both of the new lines to output
                *OutputChar++ = *CurrentChar++;
                ++OutputSz;
                *OutputChar++ = *CurrentChar++;
                ++OutputSz;
              }
              else
              {
                if (MdState.LiStarted == 1)
                {
                  // first check for li ending
                  // closes a list element
                  OutputSz += QCopyStringMoveDest(LiMapping[1], &OutputChar);
                  MdState.LiStarted = 0;
                }
                *OutputChar++ = *CurrentChar++;
                ++OutputSz;
                if (MdState.OlStarted == 0 && *(CurrentChar) == '-' && *(CurrentChar+1) == ' ')
                {
                  // ************** UNORDERED LIST **********************
                  if (MdState.UlStarted == 0)
                  {
                    OutputSz += QCopyStringMoveDest(UlMapping[0], &OutputChar);
                    MdState.UlStarted = 1;
                  }
                  OutputSz += QCopyStringMoveDest(LiMapping[0], &OutputChar);
                  CurrentChar += 2;
                  MdState.LiStarted = 1;
                }
                else if (MdState.UlStarted == 0 && IsValidOrderedList(CurrentChar))
                {
                  // ************** ORDERED LIST **********************
                  if (MdState.OlStarted == 0)
                  {
                    OutputSz += QCopyStringMoveDest(OlMapping[0], &OutputChar);
                    MdState.OlStarted = 1;
                  }
                  OutputSz += QCopyStringMoveDest(LiMapping[0], &OutputChar);
                  CurrentChar += 2;
                  MdState.LiStarted = 1;
                }
              }
            }
            else 
            {
              if (MdState.UlStarted == 0 && MdState.ParaStarted == 0)
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
#endif

void Add_Dir_Slash(struct amrs_string *path)
{
    // TODO(talha): add error codes here?
    if (*(path->last_element) != '/') {
        Amrs_Append_Const_Str_Raw(path, "/", 1);
    }
}

void Make_Dir(struct amrs_string dir_path)
{
    struct stat st = {0};
    if (stat(dir_path.buffer, &st) == -1) {
        mkdir(dir_path.buffer, 0700);
    }

    return;
}

uint32_t Get_File_Size(FILE *file_pointer)
{
    fseek(file_pointer, 0, SEEK_END);
    uint32_t size = ftell(file_pointer);
    fseek(file_pointer, 0, SEEK_SET);

    return size;
}

// Need to add error codes and error handling
// Goal: make this work for linux and windows
// only focusing on linux for now
uint32_t Read_File(char *file_path, const char *read_attributes, 
        char *file_buffer, uint32_t buffer_size)
{
    FILE *read_fp = fopen(file_path, read_attributes);
    uint32_t file_size = Get_File_Size(read_fp);

    uint32_t bytes_to_read_num = 0;
    if (buffer_size < file_size)
    {
        printf("WARNING! file_size larger than buffer_size. Truncating to buffer_size\n");
        printf("filepath: %s\n", file_path);

        bytes_to_read_num = buffer_size;
    }
    else
    {
        bytes_to_read_num = file_size;
    }

    fread(file_buffer, 1, bytes_to_read_num, read_fp);

    return file_size;
}

uint32_t Write_File(char *file_path, const char *write_attributes,
        char *file_contents, uint32_t file_contents_len)
{
    FILE *write_fp = fopen(file_path, write_attributes);
    uint32_t bytes_written = fwrite(file_contents, 1, file_contents_len, write_fp);

    return bytes_written;
}

void Process(struct global_state state, struct amrs_string src_path, struct amrs_string dest_path)
{
    if (!src_path.is_allocated || !dest_path.is_allocated) {
        return;
    }

    // open directory
    DIR *dir_pointer = opendir(src_path.buffer);
    if (dir_pointer == NULL)
    {
        printf("Error! failed to open directory\n");
        return;
    }
    
    // make directory
    Make_Dir(dest_path);
    struct dirent *file_pointer;
    do {
        file_pointer = readdir(dir_pointer);
        if (file_pointer == NULL) {
            break;
        }
        if (*file_pointer->d_name == '.') 
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

        struct amrs_string subpath = {0};

        Amrs_Init_Empty(&subpath, MAX_PATH_LEN);
        Amrs_Copy_Str(&src_path, &subpath);
        Amrs_Append_Str_Raw(&subpath, file_pointer->d_name);

        // amrf_is_directory
        struct stat st_subpath = {0};
        if (stat(subpath.buffer, &st_subpath) != 0) {
            return;
        }

        size_t status_is_dir = S_ISDIR(st_subpath.st_mode);
        if (status_is_dir)
        {
            Add_Dir_Slash(&subpath);
            printf("found dir %s\n", file_pointer->d_name);
            
            // process_directory
            struct amrs_string *sub_src_path = &subpath;

            struct amrs_string sub_dest_path = {0};
            Amrs_Init_Empty(&sub_dest_path, MAX_PATH_LEN);
            Amrs_Copy_Str(&dest_path, &sub_dest_path);
            Amrs_Append_Str_Raw(&sub_dest_path, file_pointer->d_name);
            Add_Dir_Slash(&sub_dest_path);

            Process(state, *sub_src_path, sub_dest_path);

            Amrs_Free(&sub_dest_path);
        }

        size_t status_is_file = S_ISREG(st_subpath.st_mode);
        if (status_is_file)
        {
            struct amrs_string *filepath = &subpath;

            struct amrs_result_u32 find_md_substr = Amrs_Find_Const_Substring_Raw(filepath, ".md", 3);

            if (find_md_substr.status == AMRS_NO_MATCH_FOUND)
            {
                uint32_t max_file_size = MB(64);
                char *file_contents = calloc(1, max_file_size);
                uint32_t read_file_size = Read_File(filepath->buffer, "rb",
                        file_contents, max_file_size);

                struct amrs_string output_file_path = {0};
                Amrs_Init_Empty(&output_file_path, MAX_PATH_LEN);
                Amrs_Copy_Str(&dest_path, &output_file_path);
                Amrs_Append_Str_Raw(&output_file_path, file_pointer->d_name);

                Write_File(output_file_path.buffer, "wb",
                        file_contents, read_file_size);

                Amrs_Free(&output_file_path);
                free(file_contents);
            }
            else if (find_md_substr.status == AMRS_OK)
            {
                // read file
                uint32_t md_file_capacity = MB(64);
                char *md_file = calloc(1, md_file_capacity);
                uint32_t md_file_size = Read_File(filepath->buffer, "r",
                        md_file, md_file_capacity);

                // prepare html buffer
                uint32_t html_buffer_capacity = MB(128);
                struct amrs_string html_file = {0};
                uint8_t status = Amrs_Init_Empty(&html_file, html_buffer_capacity);
                if (status == AMRS_OK)
                {
                    const char *header_mappings[6][2] = {
                        // header mapping
                        {"<h1>", "</h1>"},                          // #
                        {"<h2>", "</h2>"},                          // ##
                        {"<h3>", "</h3>"},                          // ###
                        {"<h4>", "</h4>"},                          // ####
                        {"<h5>", "</h5>"},                          // ####
                        {"<h6>", "</h6>"},                          // ####
                    };
                    uint32_t len_header_mappings = 6;

                    const char *format_mappings[3][2] = {
                        // format mappings
                        {"<em>", "</em>"},                          // *
                        {"<strong>", "</strong>"},                  // **
                        {"<em><strong>", "</strong></em>"},         // ***
                    };
                    uint32_t len_format_mappings = 3;

                    const char *ul_mapping[2] = {"<ul>", "</ul>"};  // -
                    const char *ol_mapping[2] = {"<ol>", "</ol>"};  // 1.
                    const char *li_mapping[2] = {"<li>", "</li>"};  // <element specifier for each lits>

                    // open html file tags
                    Amrs_Append_Str(&html_file, &state.header_tag);
                    Amrs_Append_Str(&html_file, &state.navbar_component);
                    Amrs_Append_Str_Raw(&html_file, "<body>");
                    Amrs_Append_Str_Raw(&html_file, "<article class=\"article-ctn\">");

                    // convert md to html
                    uint32_t running_index = 0;
                    while(running_index < html_file.len)
                    {
                        // @note: what is the precedence for conversion
                        // Headings (most important and exclusive)
                        // - when should tag open
                        // - when should tag close
                        // - h6 is the max tag allowed
                        struct amrs_result_char index = Amrs_Index(html_file, running_index);
                        if (index.status != AMRS_OK)
                        {
                            printf("Error: %d, In converting md file to html\n", index.status);
                        }

                        if (index.val == '#')
                        {
                        }

                        // Multiline Code

                        // Lists
                        // ordered list
                        // unordered list
                        if (index.val == '-')
                        {
                        }

                        running_index++;
                    }

                    // close html file tags
                    Amrs_Append_Str_Raw(&html_file, "</article>");
                    Amrs_Append_Str_Raw(&html_file, "</body>");
                    Amrs_Append_Str_Raw(&html_file, "</html>");


                    // construct destination file path
                    struct amrs_string output_file_path = {0};
                    Amrs_Init_Empty(&output_file_path, MAX_PATH_LEN);
                    Amrs_Copy_Str(&dest_path, &output_file_path);
                    Amrs_Append_Str_Raw(&output_file_path, file_pointer->d_name);

                    // replace .md with .html (the correct format)
                    struct amrs_result_u32 md_find = Amrs_Find_Const_Substring_Raw(&output_file_path, ".md", 3);
                    if (md_find.status != AMRS_OK)
                    {
                        printf("Error! failed to find 'md' substring. Exiting\n");
                        return;
                    }
                    if (Amrs_Replace_Const_Str_Raw(&output_file_path, md_find.val, ".html", 5) != AMRS_OK)
                    {
                        printf("Error! failed to replace 'md' substring with 'html'. Exiting\n");
                        return;
                    }

                    Write_File(output_file_path.buffer, "w",
                            html_file.buffer, html_file.len);

                    Amrs_Free(&output_file_path);
                    Amrs_Free(&html_file);
                }
                else
                {
                    printf("Error! failed to create html file string for conversion.\n");
                    printf("    status = %d\n", status);
                    continue;
                }

                Amrs_Free(&html_file);
                free(md_file);

            }

        }

        Amrs_Free(&subpath);
    } while (file_pointer != NULL);
    closedir(dir_pointer);
}

int main(int argc, char **argv)
{
  struct amrs_string src_path = {0};
  struct amrs_string dest_path = {0};

  Amrs_Init_Empty(&src_path, MAX_PATH_LEN);
  Amrs_Init_Empty(&dest_path, MAX_PATH_LEN);

  while (*++argv != 0) 
  {
      if (QStrEqual(*argv, "--src")) 
      {
          uint8_t status = Amrs_Append_Str_Raw(&src_path, *++argv);
          if (status != AMRS_OK) {
              return -1;
          }

          Add_Dir_Slash(&src_path);
          printf("Src path is %s\n", src_path.buffer);
      }
      else if (QStrEqual(*argv, "--dest"))
      {
          uint8_t status = Amrs_Append_Str_Raw(&dest_path, *++argv);
          if (status != AMRS_OK) {
              return -1;
          }
          Add_Dir_Slash(&dest_path);

          printf("Dest path is %s\n", dest_path.buffer);
      }
      else
      {
          printf("Unrecognized argument: %s\n", *argv);
          Amrs_Free(&src_path);
          Amrs_Free(&dest_path);
          return 0;
      }
  }

  const char *HeaderTag = "<!DOCTYPE html>\n"
  "<html>\n<head>\n"
    "<title>Talha Aamir</title>\n"
    "<link rel='icon' type='image/x-icon' href='/assets/favicon.ico.png' />\n"
    "<link rel='stylesheet' href='/styles.css' />\n"
  "</head>";
  uint32_t header_len = strlen(HeaderTag);

  const char *NavBarComp = "<ul class='nav-bar'>\n"
  "<li><a href='/index.html'>Home</a></li>\n"
  "<li><a href='/blog/root.html'>Blog</a></li>\n"
  "<li><a href='/projects/root.html'>Projects</a></li>\n"
  "<li><a href='/work/root.html'>Work</a></li>\n"
  "<li><a href='/assets/resume.pdf' target='_blank'>Resume</a></li>\n"
  "</ul>";
  uint32_t navbar_len = strlen(NavBarComp);

  struct amrs_string header_tag = {0};
  Amrs_Init_Const_Str_Raw(&header_tag, 512, HeaderTag, header_len);

  struct amrs_string navbar_component = {0};
  Amrs_Init_Const_Str_Raw(&navbar_component, 512, NavBarComp, navbar_len);

  struct global_state state = {0};
  state.src_path = src_path;
  state.dest_path = dest_path;
  state.navbar_component = navbar_component;
  state.header_tag = header_tag;

  //ReadDirectoryRecursively(src, dest, &state);
  Process(state, src_path, dest_path);

  // cleanup
  Amrs_Free(&navbar_component);
  Amrs_Free(&header_tag);
  Amrs_Free(&dest_path);
  Amrs_Free(&src_path);

  return 1;
}
