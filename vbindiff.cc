//--------------------------------------------------------------------
// $Id: vbindiff.cc,v 1.7 1996/01/18 21:29:11 Madsen Exp $
//--------------------------------------------------------------------
//
//   Visual Binary Diff
//   Copyright 1995 by Christopher J. Madsen
//
//   Visual display of differences in binary files
//
//   This program is free software; you can redistribute it and/or
//   modify it under the terms of the GNU General Public License as
//   published by the Free Software Foundation; either version 2 of
//   the License, or (at your option) any later version.
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with this program; if not, write to the Free Software
//   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//--------------------------------------------------------------------

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fstream.h>
#include <strstream.h>
#include <sys/winmgr.h>
#include <sys/kbdscan.h>

#include "getopt.h"

//====================================================================
// Type definitions:

typedef unsigned char   Byte;
typedef unsigned short  Word;

typedef Byte  Command;

enum Boolean {False, True};

//====================================================================
// Constants:

const Byte  cBackground = F_WHITE|B_BLUE;
const Byte  cPromptWin  = F_WHITE|B_BLUE;
const Byte  cPromptKey  = F_WHITE|B_BLUE|INTENSITY;
const Byte  cPromptBdr  = F_WHITE|B_BLUE|INTENSITY;
const Byte  cFileName   = F_BLACK|B_WHITE;
const Byte  cFileWin    = F_WHITE|B_BLUE;
const Byte  cFileDiff   = F_RED|B_BLUE|INTENSITY;

const Command  cmmMove        = 0x80;

const Command  cmmMoveSize    = 0x03;
const Command  cmmMoveForward = 0x04;
const Command  cmmMoveTop     = 0x08;
const Command  cmmMoveBottom  = 0x10;

const Command  cmmMoveByte    = 0x00; // Move 1 byte
const Command  cmmMoveLine    = 0x01; // Move 1 line
const Command  cmmMovePage    = 0x02; // Move 1 page
const Command  cmmMoveAll     = 0x03; // Move to beginning

const Command  cmmMoveBoth    = cmmMoveTop|cmmMoveBottom;

const Command  cmNothing      = 0;
const Command  cmNextDiff     = 1;
const Command  cmQuit         = 2;

const int  numLines  = 9;       // Number of lines of each file to display
const int  lineWidth = 16;      // Number of bytes displayed per line
const int  bufSize   = numLines * lineWidth;

const int  maxPath = 260;

// The number of bytes to move for each possible step size:
//   See cmmMoveByte, cmmMoveLine, cmmMovePage
const int  steps[4] = {1, lineWidth, bufSize-lineWidth, 0};

//====================================================================
// Class Declarations:

class Difference;

class FileDisplay
{
  friend Difference;

 protected:
  int                bufContents;
  const Difference*  diffs;
  ifstream           file;
  char               fileName[maxPath];
  streampos          offset;
  wm_handle          win;
  int                yPos;
  union {
    Byte             line[numLines][lineWidth];
    Byte             buffer[bufSize];
  };
 public:
  FileDisplay();
  ~FileDisplay();
  void         init(int y, const Difference* aDiff=NULL,
                    const char* aFileName=NULL);
  void         shutDown();
  void         display()   const;
  const Byte*  getBuffer() const { return buffer; };
  void         move(int step)    { moveTo(offset + step); };
  void         moveTo(streampos newOffset);
  Boolean      setFile(const char* aFileName);
}; // end FileDisplay

class Difference
{
  friend FileDisplay::display() const;

 protected:
  const FileDisplay*  file1;
  const FileDisplay*  file2;
  int                 numDiffs;
  union {
    Byte              line[numLines][lineWidth];
    Byte              table[bufSize];
  };
 public:
  Difference(const FileDisplay* aFile1, const FileDisplay* aFile2);
  int  compute();
  int  getNumDiffs() const { return numDiffs; };
}; // end Difference

//====================================================================
// Inline Functions:

template <class T> inline const T& min(const T& t1, const T& t2)
{
  if  (t1 < t2)
    return t1;
  else
    return t2;
}

template <class T> inline const T& max(const T& t1, const T& t2)
{
  if  (t1 > t2)
    return t1;
  else
    return t2;
}

//====================================================================
// Global Variables:

wm_handle    bgWin, promptWin;
FileDisplay  file1, file2;
Difference   diffs(&file1, &file2);
const char*  program_name; // Name under which this program was invoked

//====================================================================
// Class Difference:
//
// Member Variables:
//   file1, file2:
//     The FileDisplay objects being compared
//   numDiffs:
//     The number of differences between the two FileDisplay buffers
//   line/table:
//     An array of Booleans for each byte in the FileDisplay buffers
//     True marks differences
//
//--------------------------------------------------------------------
// Constructor:
//
// Input:
//   aFile1, aFile2:
//     Pointers to the FileDisplay objects to compare

Difference::Difference(const FileDisplay* aFile1, const FileDisplay* aFile2)
: file1(aFile1),
  file2(aFile2)
{
} // end Difference::Difference

//--------------------------------------------------------------------
// Compute differences:
//
// Input Variables:
//   file1, file2:  The files to compare
//
// Returns:
//   The number of differences between the buffers
//   -1 if both buffers are empty
//
// Output Variables:
//   numDiffs:  The number of differences between the buffers

int Difference::compute()
{
  memset(table, 0, sizeof(table)); // Clear the difference table

  int  different = 0;

  const Byte*  buf1 = file1->buffer;
  const Byte*  buf2 = file2->buffer;

  int  size = min(file1->bufContents, file2->bufContents);

  int  i;
  for (i = 0; i < size; i++)
    if (*(buf1++) != *(buf2++)) {
      table[i] = True;
      ++different;
    }

  size = max(file1->bufContents, file2->bufContents);

  if (i < size) {
    // One buffer has more data than the other:
    different += size - i;
    for (; i < size; i++)
      table[i] = True;          // These bytes are only in 1 buffer
  } else if (!size)
    return -1;                  // Both buffers are empty

  numDiffs = different;

  return different;
} // end Difference::compute

//====================================================================
// Class FileDisplay:
//
// Member Variables:
//   bufContents:
//     The number of bytes in the file buffer
//   diffs:
//     A pointer to the Difference object related to this file
//   file:
//     The file being displayed
//   fileName:
//     The relative pathname of the file being displayed
//   offset:
//     The position in the file of the first byte in the buffer
//   win:
//     The handle of the window used for display
//   yPos:
//     The vertical position of the display window
//   buffer/line:
//     The currently displayed portion of the file
//
//--------------------------------------------------------------------
// Constructor:

FileDisplay::FileDisplay()
: bufContents(0),
  diffs(NULL),
  offset(0),
  win(NULL),
  yPos(0)
{
  fileName[0] = '\0';
} // end FileDisplay::FileDisplay

//--------------------------------------------------------------------
// Initialize:
//
// Creates the display window and opens the file.
//
// Input:
//   y:          The vertical position of the display window
//   aDiff:      The Difference object related to this buffer
//   aFileName:  The name of the file to display

void FileDisplay::init(int y, const Difference* aDiff,
                       const char* aFileName)
{
  diffs = aDiff;
  yPos  = y;

  win = wm_create (0,y+1, 79,y+numLines, // Window corners
                         0,              // No border
                         cFileWin,       // Border attributes
                         cFileWin);      // Default attributes
  wm_update(win, 0);
  wm_open(win);

  if (aFileName)
    setFile(aFileName);
} // end FileDisplay::init

//--------------------------------------------------------------------
// Destructor:

FileDisplay::~FileDisplay()
{
  shutDown();
  file.close();
} // end FileDisplay::~FileDisplay

//--------------------------------------------------------------------
// Shut down the file display:
//
// Deletes the display window.

void FileDisplay::shutDown()
{
  if (win) {
    wm_close(win);
    wm_delete(win);
    win = NULL;
  }
} // end FileDisplay::shutDown

//--------------------------------------------------------------------
// Display the file contents:

void FileDisplay::display() const
{
  const int  leftMar  = 11;     // Starting column of hex display
  const int  leftMar2 = 61;     // Starting column of ASCII display

  streampos  lineOffset = offset;

  wm_clear(win);

  int i,j,index,lineLength;
  char  buf[lineWidth + lineWidth/8 + 1];

  memset(buf, ' ', sizeof(buf));

  for (i = 0; i < numLines; i++) {
    wm_gotoxy(win, 0,i);
    wm_printf(win, "%04X %04X:",Word(lineOffset>>16),Word(lineOffset&0xFFFF));

    lineLength  = min(lineWidth, bufContents - i*lineWidth);

    for (j = index = 0; j < lineLength; j++) {
      if (j % 8 == 0) {
        wm_putc(win, ' ');
        ++index;
      }
      wm_printf(win, "%02X ", line[i][j]);
      buf[index++] = (line[i][j] >= ' ') ? line[i][j] : '.';
    }
    buf[index] = '\0';

    wm_puts_at(win, leftMar2-1,i, buf);

    if (diffs)
      for (j = 0; j < lineWidth; j++)
        if (diffs->line[i][j]) {
          wm_puta_at(win, j*3 + leftMar  + (j>7),i, cFileDiff,2);
          wm_puta_at(win, j   + leftMar2 + (j>7),i, cFileDiff,1);
        }
    lineOffset += lineWidth;
  }
  wm_update(win, 0);
} // end FileDisplay::display

//--------------------------------------------------------------------
// Change the file position:
//
// Changes the file offset and updates the buffer.
// Does not update the display.
//
// Input:
//   step:
//     The number of bytes to move
//     A negative value means to move backward
//
// void FileDisplay::move(int step) /* Inline function */

//--------------------------------------------------------------------
// Change the file position:
//
// Changes the file offset and updates the buffer.
// Does not update the display.
//
// Input:
//   newOffset:
//     The new position of the file

void FileDisplay::moveTo(streampos newOffset)
{
  offset = newOffset;

  if (offset < 0)
    offset = 0;

  if (file.fail())
    file.clear();

  file.seekg(offset);
  file.read(buffer, bufSize);
  bufContents = file.gcount();
} // end FileDisplay::moveTo

//--------------------------------------------------------------------
// Open a file for display:
//
// Opens the file, updates the filename display, and reads the start
// of the file into the buffer.
//
// Input:
//   aFileName:  The name of the file to open
//
// Returns:
//   True:   Operation successful
//   False:  Unable to open file

Boolean FileDisplay::setFile(const char* aFileName)
{
  strncpy(fileName, aFileName, maxPath);
  fileName[maxPath-1] = '\0';

  wm_puts_at(bgWin, 0,yPos, fileName);
  wm_puta_at(bgWin, 0,yPos, cFileName, 80);

  file.close();
  bufContents = 0;
  file.open(fileName, ios::in|ios::bin|ios::nocreate);

  if (!file)
    return False;

  offset = 0;
  file.read(buffer, bufSize);
  bufContents = file.gcount();

  return True;
} // end FileDisplay::setFile

//====================================================================
// Main Program:
//--------------------------------------------------------------------
// Create & display prompt window:
//
// Returns:
//   True:   Prompt window created & displayed
//   False:  Unable to display prompt

Boolean initPrompt()
{
  promptWin = wm_create (1,22, 78,23, // Window corners
                         1,           // Single line border
                         cPromptBdr,  // Border attributes
                         cPromptWin); // Default attributes
  if (promptWin == NULL)
    return False;

  wm_puts(promptWin, "\x1A forward 1 char   \x19 forward 1 line"
          "   RET next difference  ALT  freeze top\n"
          "\x1B backward 1 char  \x18 backward 1 line"
          "  ESC quit             CTRL freeze bottom");
  wm_puta_at(promptWin,  0,0, cPromptKey, 1);
  wm_puta_at(promptWin, 19,0, cPromptKey, 1);
  wm_puta_at(promptWin, 38,0, cPromptKey, 3);
  wm_puta_at(promptWin, 59,0, cPromptKey, 4);
  wm_puta_at(promptWin,  0,1, cPromptKey, 1);
  wm_puta_at(promptWin, 19,1, cPromptKey, 1);
  wm_puta_at(promptWin, 38,1, cPromptKey, 3);
  wm_puta_at(promptWin, 59,1, cPromptKey, 4);
  wm_open (promptWin);          // Open the window

  return True;
} // end initPrompt

//--------------------------------------------------------------------
// Initialize program:
//
// Returns:
//   True:   Initialization complete
//   False:  Error

Boolean initialize()
{
  if (!wm_init(7))
    return False;

  // Create & display background window:
  bgWin = wm_create (0,0, 79,24,   // Window corners
                     0,            // No border
                     cBackground,  // Border attributes
                     cBackground); // Default attributes
  if (bgWin == NULL)
    return False;
  wm_open(bgWin);

  if (!initPrompt())
    return False;

  file1.init(0,&diffs);
  file2.init(11,&diffs);

  wm_top(promptWin);

  return True;
} // end initialize

//--------------------------------------------------------------------
// Display license and warranty information and exit:

static void license()
{
  cout << "\
Visual Binary Diff
Copyright 1995 by Christopher J. Madsen

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of
the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.\n";
  exit(0);
} // end license

//--------------------------------------------------------------------
// Display usage information and exit:
//
// Input:
//   status:
//     The desired exit status
//       If status is 0 or 1, prints complete usage information
//       If status is > 1, prints only "try --help" message

static void usage(int status)
{
  if (status > 1)
    cerr << "Try `" << program_name << " --help' for more information.\n";
  else {
    cout << "Usage: " << program_name << " FILE1 FILE2
Compare FILE1 and FILE2 byte by byte.

Options:
      --help               display this help information and exit
      -L, --license        display license & warranty information and exit
      -V, --version        display version information and exit\n";
  }
  exit(status);
} // end usage

//--------------------------------------------------------------------
// Get a command from the keyboard:
//
// Returns:
//   Command code

Command getCommand()
{
  Command  cmd = cmNothing;

  while (_read_kbd(0,0,0) != -1) // Clear keyboard buffer
    ;

  int  key = _read_kbd(0,1,0);

  key = toupper(key);

  switch (key) {
   case 0:                      // Extended code
    key = _read_kbd(0,1,0);
    switch (key) {
     case K_DOWN:
      cmd = cmmMove|cmmMoveBoth|cmmMoveLine|cmmMoveForward;
      break;
     case K_RIGHT:
      cmd = cmmMove|cmmMoveBoth|cmmMoveByte|cmmMoveForward;
      break;
     case K_PAGEDOWN:
      cmd = cmmMove|cmmMoveBoth|cmmMovePage|cmmMoveForward;
      break;
     case K_LEFT:
      cmd = cmmMove|cmmMoveBoth|cmmMoveByte;
      break;
     case K_UP:
      cmd = cmmMove|cmmMoveBoth|cmmMoveLine;
      break;
     case K_PAGEUP:
      cmd = cmmMove|cmmMoveBoth|cmmMovePage;
      break;
     case K_HOME:
      cmd = cmmMove|cmmMoveBoth|cmmMoveAll;
      break;

     case K_ALT_DOWN:
      cmd = cmmMove|cmmMoveBottom|cmmMoveLine|cmmMoveForward;
      break;
     case K_ALT_RIGHT:
      cmd = cmmMove|cmmMoveBottom|cmmMoveByte|cmmMoveForward;
      break;
     case K_ALT_PAGEDOWN:
      cmd = cmmMove|cmmMoveBottom|cmmMovePage|cmmMoveForward;
      break;
     case K_ALT_LEFT:
      cmd = cmmMove|cmmMoveBottom|cmmMoveByte;
      break;
     case K_ALT_UP:
      cmd = cmmMove|cmmMoveBottom|cmmMoveLine;
      break;
     case K_ALT_PAGEUP:
      cmd = cmmMove|cmmMoveBottom|cmmMovePage;
      break;
     case K_ALT_HOME:
      cmd = cmmMove|cmmMoveBottom|cmmMoveAll;
      break;

     case K_CTRL_DOWN:
      cmd = cmmMove|cmmMoveTop|cmmMoveLine|cmmMoveForward;
      break;
     case K_CTRL_RIGHT:
      cmd = cmmMove|cmmMoveTop|cmmMoveByte|cmmMoveForward;
      break;
     case K_CTRL_PAGEDOWN:
      cmd = cmmMove|cmmMoveTop|cmmMovePage|cmmMoveForward;
      break;
     case K_CTRL_LEFT:
      cmd = cmmMove|cmmMoveTop|cmmMoveByte;
      break;
     case K_CTRL_UP:
      cmd = cmmMove|cmmMoveTop|cmmMoveLine;
      break;
     case K_CTRL_PAGEUP:
      cmd = cmmMove|cmmMoveTop|cmmMovePage;
      break;
     case K_CTRL_HOME:
      cmd = cmmMove|cmmMoveTop|cmmMoveAll;
      break;
    } // end inner switch
    break;

   case 0x0D:                   // Enter
    cmd = cmNextDiff;
    break;

   case 0x1B:                   // Esc
   case 0x03:                   // Ctrl+C
   case 'Q':
    cmd = cmQuit;
    break;
  } // end switch key

  return cmd;
} // end getCommand

//--------------------------------------------------------------------
// Handle a command:
//
// Input:
//   cmd:  The command to be handled

void handleCmd(Command cmd)
{
  if (cmd & cmmMove) {
    int  step = steps[cmd & cmmMoveSize];

    if ((cmd & cmmMoveForward) == 0)
      step *= -1;               // We're moving backward

    if (cmd & cmmMoveTop)
      if (step)
        file1.move(step);
      else
        file1.moveTo(0);

    if (cmd & cmmMoveBottom)
      if (step)
        file2.move(step);
      else
        file2.moveTo(0);
  } // end if move
  else if (cmd == cmNextDiff) {
    do {
      file1.move(bufSize);
      file2.move(bufSize);
    } while (!diffs.compute());
  } // end else if cmNextDiff

  // Make sure we haven't gone past the end of both files:
  while (diffs.compute() < 0) {
    file1.move(-steps[cmmMovePage]);
    file2.move(-steps[cmmMovePage]);
  }

  file1.display();
  file2.display();
} // end handleCmd

//--------------------------------------------------------------------
int main(int argc, char* argv[])
{
  // If non-zero, display usage information and exit.
  static int show_help;

  // If non-zero, display license & warranty information and exit.
  static int show_license;

  // If non-zero, print the version on standard output then exit.
  static int show_version;

  static struct option const long_options[] = {
    {"help",    no_argument, &show_help, 1},
    {"license", no_argument, &show_license, 1},
    {"version", no_argument, &show_version, 1},
    {NULL, 0, NULL, 0}
  };

  if ((program_name = strrchr(argv[0], '\\')))
    // Isolate the filename and convert to lowercase:
    strlwr(++const_cast<char*>(program_name));
  else
    program_name = argv[0];

  // Parse command line options:
  int c;
  while ((c = getopt_long(argc, argv, "LV", long_options, (int *)0))
	 != EOF) {
    switch (c) {
     case 0:
      break;

     case 'L':
      show_license = 1;
      break;

     case 'V':
      show_version = 1;
      break;

     default:
      usage(2);
    } // end switch
  } // end while options

  if (show_version) {
      cerr << "VBinDiff $Revision: 1.7 $\n";
      exit(0);
  }

  if (show_help)
    usage(0);

  if (show_license)
    license();

  if (argc != 3)
    usage(1);

  cout << "\
VBinDiff $Revision: 1.7 $, Copyright 1995 Christopher J. Madsen
VBinDiff comes with ABSOLUTELY NO WARRANTY; for details type `vbindiff -L'.\n";

  if (!initialize()) {
    cerr << '\n' << program_name << ": Unable to initialize windows\n";
    return 1;
  }

  char  error[80] = "";
  ostrstream errMsg(error, sizeof(error));

  if (!file1.setFile(argv[1]))
    errMsg << "Unable to open " << argv[1];
  else if (!file2.setFile(argv[2]))
    errMsg << "Unable to open " << argv[2];
  else {
    diffs.compute();

    file1.display();
    file2.display();

    Command  cmd;
    while ((cmd = getCommand()) != cmQuit)
      handleCmd(cmd);
  } // end else files opened successfully

  file1.shutDown();
  file2.shutDown();

  wm_close_all();               // Close all windows
  wm_exit();                    // End window manager

  if (*error) {
    errMsg << '\n' << '\0';
    cerr << '\n' << program_name << ": " << error;
    return 1;
  }
  return 0;
} // end main

// Local Variables:
// pathsearch-c-include-path: emx-c-include-path
// End:
