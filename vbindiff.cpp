//--------------------------------------------------------------------
// $Id: vbindiff.cpp,v 2.0 1997/10/13 22:49:43 Madsen Exp $
//--------------------------------------------------------------------
//
//   Visual Binary Diff
//   Copyright 1995-7 by Christopher J. Madsen
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

#include "StdAfx.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <fstream>
#include <strstream>
using namespace std;

#define __STDC__ 1
#define __GNU_LIBRARY__
#include "getopt.h"
#undef __GNU_LIBRARY__
#undef __STDC__

#include "ConWin.hpp"

//====================================================================
// Type definitions:

typedef unsigned char   Byte;
typedef unsigned short  Word;

typedef Byte  Command;

//====================================================================
// Constants:

const Byte  cBackground = F_WHITE|B_BLUE;
const Byte  cPromptWin  = F_WHITE|B_BLUE;
const Byte  cPromptKey  = F_WHITE|B_BLUE|FOREGROUND_INTENSITY;
const Byte  cPromptBdr  = F_WHITE|B_BLUE|FOREGROUND_INTENSITY;
const Byte  cFileName   = F_BLACK|B_WHITE;
const Byte  cFileWin    = F_WHITE|B_BLUE;
const Byte  cFileDiff   = F_RED|B_BLUE|FOREGROUND_INTENSITY;

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

const Command  cmgGoto        = 0x04; // Commands 4-7
const Command  cmgGotoTop     = 0x01;
const Command  cmgGotoBottom  = 0x02;
const Command  cmgGotoBoth    = cmgGotoTop|cmgGotoBottom;

const Command  cmNothing      = 0;
const Command  cmNextDiff     = 1;
const Command  cmQuit         = 2;

const int  numLines  = 9;       // Number of lines of each file to display
const int  lineWidth = 16;      // Number of bytes displayed per line
const int  bufSize   = numLines * lineWidth;

const int  inWidth = 10;        // Width of input window (excluding border)
const int  screenWidth = 80;

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
  ConWindow          win;
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
  void         display();
  const Byte*  getBuffer() const { return buffer; };
  void         move(int step)    { moveTo(offset + streampos(step)); };
  void         moveTo(streampos newOffset);
  bool         setFile(const char* aFileName);
}; // end FileDisplay

class Difference
{
  friend void FileDisplay::display();

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
// Global Variables:

ConWindow    promptWin,inWin;
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
//     An array of bools for each byte in the FileDisplay buffers
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
      table[i] = true;
      ++different;
    }

  size = max(file1->bufContents, file2->bufContents);

  if (i < size) {
    // One buffer has more data than the other:
    different += size - i;
    for (; i < size; i++)
      table[i] = true;          // These bytes are only in 1 buffer
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

  win.init(0,y, screenWidth,numLines+1+(y==0), cFileWin); // FIXME

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
} // end FileDisplay::shutDown

//--------------------------------------------------------------------
// Display the file contents:

void FileDisplay::display()
{
  const short  leftMar  = 11;     // Starting column of hex display
  const short  leftMar2 = 61;     // Starting column of ASCII display

  streampos  lineOffset = offset;

  short i,j,index,lineLength;
  char  buf[lineWidth + lineWidth/8 + 1];
  buf[sizeof(buf)-1] = '\0';

  char  buf2[screenWidth+1];
  buf2[screenWidth] = '\0';

  memset(buf, ' ', sizeof(buf));

  for (i = 0; i < numLines; i++) {
//    cerr << i << '\n';
    char*  str = buf2;
    str +=
      sprintf(str, "%04X %04X:",Word(lineOffset>>16),Word(lineOffset&0xFFFF));

    lineLength  = min(lineWidth, bufContents - i*lineWidth);

    for (j = 0, index = -1; j < lineLength; j++) {
      if (j % 8 == 0) {
        *(str++) = ' ';
        ++index;
      }
      str += sprintf(str, "%02X ", line[i][j]);

      buf[index++] = (line[i][j] >= ' ') ? line[i][j] : '.';
    }
    memset(buf + index, ' ', sizeof(buf) - index - 1);
    memset(str, ' ', screenWidth - (str - buf2));

    win.put(0,i+1, buf2);
    win.put(leftMar2,i+1, buf);

    if (diffs)
      for (j = 0; j < lineWidth; j++)
        if (diffs->line[i][j]) {
          win.putAttribs(j*3 + leftMar  + (j>7),i+1, cFileDiff,2);
          win.putAttribs(j   + leftMar2 + (j>7),i+1, cFileDiff,1);
        }
    lineOffset += lineWidth;
  }

  win.update();
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
  file.read(reinterpret_cast<char*>(buffer), bufSize);
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

bool FileDisplay::setFile(const char* aFileName)
{
  strncpy(fileName, aFileName, maxPath);
  fileName[maxPath-1] = '\0';

  win.put(0,0, fileName);
  win.putAttribs(0,0, cFileName, screenWidth);
  win.update();                 // FIXME

  if (file.is_open())
    file.close();
  bufContents = 0;
  file.open(fileName, ios::in|ios::binary);

  if (!file)
    return false;

  offset = 0;
  file.read(reinterpret_cast<char*>(buffer), bufSize);
  bufContents = file.gcount();

  return true;
} // end FileDisplay::setFile

//====================================================================
// Main Program:
//--------------------------------------------------------------------
// Initialize program:
//
// Returns:
//   True:   Initialization complete
//   False:  Error

bool initialize()
{
  if (!ConWindow::startup())
    return false;

  ConWindow::hideCursor();

  inWin.init(0,0, inWidth+2,3, cPromptBdr);
  inWin.boxSingle(0,0, inWidth+2,3);
  inWin.put((inWidth-4)/2,0, "\264Goto\303");
  inWin.setAttribs(cPromptWin);

  promptWin.init(0,21, screenWidth,4, F_WHITE|B_BLUE);
  promptWin.boxSingle(0,0, screenWidth,4);
  promptWin.put(1,1, "\x1A fwd 1 byte   \x19 fwd 1 line"
                "  RET next difference  ESC quit  ALT  freeze top");
  promptWin.put(1,2, "\x1B back 1 byte  \x18 back 1 line   G goto position"
                "      Q quit  CTRL freeze bottom");
  promptWin.putAttribs( 1,1, cPromptKey, 1);
  promptWin.putAttribs(16,1, cPromptKey, 1);
  promptWin.putAttribs(30,1, cPromptKey, 3);
  promptWin.putAttribs(51,1, cPromptKey, 3);
  promptWin.putAttribs(61,1, cPromptKey, 4);
  promptWin.putAttribs( 1,2, cPromptKey, 1);
  promptWin.putAttribs(16,2, cPromptKey, 1);
  promptWin.putAttribs(32,2, cPromptKey, 1);
  promptWin.putAttribs(53,2, cPromptKey, 1);
  promptWin.putAttribs(61,2, cPromptKey, 4);
  promptWin.update();

  file1.init(0,&diffs);
  file2.init(11,&diffs);

  return true;
} // end initialize

//--------------------------------------------------------------------
// Display license and warranty information and exit:

static void license()
{
  cout << "\
Visual Binary Diff\n\
Copyright 1995-7 by Christopher J. Madsen\n\
\n\
This program is free software; you can redistribute it and/or\n\
modify it under the terms of the GNU General Public License as\n\
published by the Free Software Foundation; either version 2 of\n\
the License, or (at your option) any later version.\n\
\n\
This program is distributed in the hope that it will be useful,\n\
but WITHOUT ANY WARRANTY; without even the implied warranty of\n\
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n\
GNU General Public License for more details.\n\
\n\
You should have received a copy of the GNU General Public License\n\
along with this program; if not, write to the Free Software\n\
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
    cout << "Usage: " << program_name << " FILE1 FILE2\n\
Compare FILE1 and FILE2 byte by byte.\n\
\n\
Options:\n\
      --help               display this help information and exit\n\
      -L, --license        display license & warranty information and exit\n\
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
  KEY_EVENT_RECORD e;
  Command  cmd = cmNothing;

  while (cmd == cmNothing) {
    ConWindow::readKey(e);

    switch (toupper(e.uChar.AsciiChar)) {
     case 0x0D:               // Enter
      cmd = cmNextDiff;
      break;

     case 'G':
      if (e.dwControlKeyState & (LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED))
        cmd = cmgGoto|cmgGotoBottom;
      else
        cmd = cmgGoto|cmgGotoBoth;
      break;

     case 0x07:               // Ctrl+G
      cmd = cmgGoto|cmgGotoTop;
      break;

     case 0x1B:               // Esc
     case 0x03:               // Ctrl+C
     case 'Q':
      cmd = cmQuit;
      break;

     default:                 // Try extended codes
      if (e.dwControlKeyState & (LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED)) {
        switch (e.wVirtualKeyCode) {
         case VK_DOWN:
          cmd = cmmMove|cmmMoveBottom|cmmMoveLine|cmmMoveForward;
          break;
         case VK_RIGHT:
          cmd = cmmMove|cmmMoveBottom|cmmMoveByte|cmmMoveForward;
          break;
         case VK_NEXT:
          cmd = cmmMove|cmmMoveBottom|cmmMovePage|cmmMoveForward;
          break;
         case VK_LEFT:
          cmd = cmmMove|cmmMoveBottom|cmmMoveByte;
          break;
         case VK_UP:
          cmd = cmmMove|cmmMoveBottom|cmmMoveLine;
          break;
         case VK_PRIOR:
          cmd = cmmMove|cmmMoveBottom|cmmMovePage;
          break;
         case VK_HOME:
          cmd = cmmMove|cmmMoveBottom|cmmMoveAll;
          break;
        } // end switch alt virtual key code
      } else if (e.dwControlKeyState & (LEFT_CTRL_PRESSED|RIGHT_CTRL_PRESSED)) {
        switch (e.wVirtualKeyCode) {
         case VK_DOWN:
          cmd = cmmMove|cmmMoveTop|cmmMoveLine|cmmMoveForward;
          break;
         case VK_RIGHT:
          cmd = cmmMove|cmmMoveTop|cmmMoveByte|cmmMoveForward;
          break;
         case VK_NEXT:
          cmd = cmmMove|cmmMoveTop|cmmMovePage|cmmMoveForward;
          break;
         case VK_LEFT:
          cmd = cmmMove|cmmMoveTop|cmmMoveByte;
          break;
         case VK_UP:
          cmd = cmmMove|cmmMoveTop|cmmMoveLine;
          break;
         case VK_PRIOR:
          cmd = cmmMove|cmmMoveTop|cmmMovePage;
          break;
         case VK_HOME:
          cmd = cmmMove|cmmMoveTop|cmmMoveAll;
          break;
        } // end switch control virtual key code
      } else {
        switch (e.wVirtualKeyCode) {
         case VK_DOWN:
          cmd = cmmMove|cmmMoveBoth|cmmMoveLine|cmmMoveForward;
          break;
         case VK_RIGHT:
          cmd = cmmMove|cmmMoveBoth|cmmMoveByte|cmmMoveForward;
          break;
         case VK_NEXT:
          cmd = cmmMove|cmmMoveBoth|cmmMovePage|cmmMoveForward;
          break;
         case VK_LEFT:
          cmd = cmmMove|cmmMoveBoth|cmmMoveByte;
          break;
         case VK_UP:
          cmd = cmmMove|cmmMoveBoth|cmmMoveLine;
          break;
         case VK_PRIOR:
          cmd = cmmMove|cmmMoveBoth|cmmMovePage;
          break;
         case VK_HOME:
          cmd = cmmMove|cmmMoveBoth|cmmMoveAll;
          break;
        } // end switch virtual key code
      } // end else not Alt or Ctrl
      break;
    } // end switch ASCII code
  } // end while no command

  return cmd;
} // end getCommand

//--------------------------------------------------------------------
// Get a file position:

void gotoPosition(Command cmd)
{
  inWin.move((screenWidth-inWidth-2)/2,
             ((cmd & cmgGotoBottom)
              ? ((cmd & cmgGotoTop) ? 10 : 15)
              : 4));

  inWin.putChar(1,1, ' ', inWidth);
  inWin.update();
  inWin.setCursor(2,1);
  ConWindow::showCursor();

  const int  bufLen = inWidth-2;
  char  buf[bufLen+1];
  bool  done = false;
  int   i = 0;
  KEY_EVENT_RECORD e;

  memset(buf, ' ', bufLen);
  buf[bufLen] = '\0';

  while (!done) {
    inWin.put(2,1,buf);
    inWin.update();
    inWin.setCursor(2+i,1);
    ConWindow::readKey(e);

    char key = toupper(e.uChar.AsciiChar);

    if (key) {
      if (strchr("0123456789ABCDEF", key)) {
        if (i >= bufLen) continue;
        buf[i++] = key;
      } else if (key == 0x0D) { // Enter
        buf[i] = '\0';
        done = true;
      } else if (key == 0x08) { // Backspace
        if (!i) continue;
        buf[--i] = ' ';
      } else if (key == 0x1B) { // ESC
        buf[0] = '\0';
        done = true;
      }
    } // end if key
  } // end while

  ConWindow::hideCursor();

  if (!buf[0])
    return;

  streampos  pos = strtoul(buf, NULL, 16);

  if (cmd & cmgGotoTop)
    file1.moveTo(pos);
  if (cmd & cmgGotoBottom)
    file2.moveTo(pos);
} // end gotoPosition

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
  else if ((cmd & cmgGoto) == cmgGoto)
    gotoPosition(cmd);
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
    // Isolate the filename:
    ++program_name;
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
      cerr << "VBinDiff $Revision: 2.0 $\n";
      exit(0);
  }

  if (show_help)
    usage(0);

  if (show_license)
    license();

  if (argc != 3)
    usage(1);

  cout << "\
VBinDiff $Revision: 2.0 $, Copyright 1995-7 Christopher J. Madsen\n\
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

  ConWindow::shutdown();

  if (*error) {
    errMsg << '\n' << '\0';
    cerr << '\n' << program_name << ": " << error;
    return 1;
  }
  return 0;
} // end main
