//--------------------------------------------------------------------
// $Id: vbindiff.cpp 4594 2005-03-14 15:39:43Z cjm $
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

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <fstream>
#include <sstream>
using namespace std;

#include "config.h"

#include "GetOpt/GetOpt.hpp"

#include "ConWin.hpp"

const char titleString[] =
  "\nVBinDiff " PACKAGE_VERSION " by Christopher J. Madsen";

void usage(bool showHelp=true, int exitStatus=0);

//====================================================================
// Type definitions:

typedef unsigned char   Byte;
typedef unsigned short  Word;

typedef Byte  Command;

enum LockState { lockNeither = 0, lockTop, lockBottom };

//====================================================================
// Constants:

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
const Command  cmgGotoMask    = ~cmgGotoBoth;

const Command  cmNothing      = 0;
const Command  cmNextDiff     = 1;
const Command  cmQuit         = 2;
const Command  cmEditTop      = 8;
const Command  cmEditBottom   = 9;
const Command  cmUseTop       = 10;
const Command  cmUseBottom    = 11;
const Command  cmToggleASCII  = 12;

const short  leftMar  = 11;     // Starting column of hex display
const short  leftMar2 = 61;     // Starting column of ASCII display

const int  numLines  = 9;       // Number of lines of each file to display
const int  lineWidth = 16;      // Number of bytes displayed per line
const int  bufSize   = numLines * lineWidth;

const int  inWidth = 10;        // Width of input window (excluding border)
const int  screenWidth = 80;

const int  maxPath = 260;

// The number of bytes to move for each possible step size:
//   See cmmMoveByte, cmmMoveLine, cmmMovePage
const int  steps[4] = {1, lineWidth, bufSize-lineWidth, 0};

const char asciiDisplayTable[256] = {
   '.',  '.',  '.',  '.',  '.',  '.',  '.',  '.',
   '.',  '.',  '.',  '.',  '.',  '.',  '.',  '.',
   '.',  '.',  '.',  '.',  '.',  '.',  '.',  '.',
   '.',  '.',  '.',  '.',  '.',  '.',  '.',  '.',
  0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
  0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
  0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
  0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
  0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
  0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
  0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
  0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
  0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
  0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
  0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
  0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E,  '.',
   '.',  '.',  '.',  '.',  '.',  '.',  '.',  '.',
   '.',  '.',  '.',  '.',  '.',  '.',  '.',  '.',
   '.',  '.',  '.',  '.',  '.',  '.',  '.',  '.',
   '.',  '.',  '.',  '.',  '.',  '.',  '.',  '.',
   '.',  '.',  '.',  '.',  '.',  '.',  '.',  '.',
   '.',  '.',  '.',  '.',  '.',  '.',  '.',  '.',
   '.',  '.',  '.',  '.',  '.',  '.',  '.',  '.',
   '.',  '.',  '.',  '.',  '.',  '.',  '.',  '.',
   '.',  '.',  '.',  '.',  '.',  '.',  '.',  '.',
   '.',  '.',  '.',  '.',  '.',  '.',  '.',  '.',
   '.',  '.',  '.',  '.',  '.',  '.',  '.',  '.',
   '.',  '.',  '.',  '.',  '.',  '.',  '.',  '.',
   '.',  '.',  '.',  '.',  '.',  '.',  '.',  '.',
   '.',  '.',  '.',  '.',  '.',  '.',  '.',  '.',
   '.',  '.',  '.',  '.',  '.',  '.',  '.',  '.',
   '.',  '.',  '.',  '.',  '.',  '.',  '.',  '.'
}; // end asciiDisplayTable

const char ebcdicDisplayTable[256] = {
   '.',  '.',  '.',  '.',  '.',  '.',  '.',  '.',
   '.',  '.',  '.',  '.',  '.',  '.',  '.',  '.',
   '.',  '.',  '.',  '.',  '.',  '.',  '.',  '.',
   '.',  '.',  '.',  '.',  '.',  '.',  '.',  '.',
   '.',  '.',  '.',  '.',  '.',  '.',  '.',  '.',
   '.',  '.',  '.',  '.',  '.',  '.',  '.',  '.',
   '.',  '.',  '.',  '.',  '.',  '.',  '.',  '.',
   '.',  '.',  '.',  '.',  '.',  '.',  '.',  '.',
  0x20,  '.',  '.',  '.',  '.',  '.',  '.',  '.',
   '.',  '.',  '.', 0x2E, 0x3C, 0x28, 0x2B, 0x7C,
  0x26,  '.',  '.',  '.',  '.',  '.',  '.',  '.',
   '.',  '.', 0x21, 0x24, 0x2A, 0x29, 0x3B, 0x5E,
  0x2D, 0x2F,  '.',  '.',  '.',  '.',  '.',  '.',
   '.',  '.',  '.', 0x2C, 0x25, 0x5F, 0x3E, 0x3F,
   '.',  '.',  '.',  '.',  '.',  '.',  '.',  '.',
   '.', 0x60, 0x3A, 0x23, 0x40, 0x27, 0x3D, 0x22,
   '.', 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
  0x68, 0x69,  '.',  '.',  '.',  '.',  '.',  '.',
   '.', 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F, 0x70,
  0x71, 0x72,  '.',  '.',  '.',  '.',  '.',  '.',
   '.', 0x7E, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78,
  0x79, 0x7A,  '.',  '.',  '.', 0x5B,  '.',  '.',
   '.',  '.',  '.',  '.',  '.',  '.',  '.',  '.',
   '.',  '.',  '.',  '.',  '.', 0x5D,  '.',  '.',
  0x7B, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
  0x48, 0x49,  '.',  '.',  '.',  '.',  '.',  '.',
  0x7D, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50,
  0x51, 0x52,  '.',  '.',  '.',  '.',  '.',  '.',
  0x5C,  '.', 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
  0x59, 0x5A,  '.',  '.',  '.',  '.',  '.',  '.',
  0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
  0x38, 0x39,  '.',  '.',  '.',  '.',  '.',  '.'
}; // end ebcdicDisplayTable

//====================================================================
// Class Declarations:

void showEditPrompt();
void showPrompt();

class Difference;

class FileDisplay
{
  friend class Difference;

 protected:
  int                bufContents;
  const Difference*  diffs;
  fstream            file;
  char               fileName[maxPath];
  streampos          offset;
  ConWindow          win;
  bool               writable;
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
  bool         edit(const FileDisplay* other);
  const Byte*  getBuffer() const { return buffer; };
  void         move(int step)    { moveTo(offset + streampos(step)); };
  void         moveTo(streampos newOffset);
  bool         setFile(const char* aFileName);
 protected:
  void  setByte(short x, short y, Byte b);
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
const char*  displayTable = asciiDisplayTable;
const char*  program_name; // Name under which this program was invoked
LockState    lockState = lockNeither;

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
  writable(false),
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
  win.close();
} // end FileDisplay::shutDown

//--------------------------------------------------------------------
// Display the file contents:

void FileDisplay::display()
{
  streampos  lineOffset = offset;

  short i,j,index,lineLength;
  char  buf[lineWidth + lineWidth/8 + 1];
  buf[sizeof(buf)-1] = '\0';

  char  buf2[screenWidth+1];
  buf2[screenWidth] = '\0';

  memset(buf, ' ', sizeof(buf)-1);

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

      buf[index++] = displayTable[line[i][j]];
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
// Edit the file:
//
// Returns:
//   true:  File changed
//   false:  File did not change

bool FileDisplay::edit(const FileDisplay* other)
{
  return false;
#if 0
  if (!bufContents && offset)
    return false;               // You must not be completely past EOF

  if (!writable) {
    file.clear();
    file.close();
    file.open(fileName, ios::in|ios::out|ios::binary);
    writable = true;
    if (!file) {
      writable = false;
      file.clear();
      file.open(fileName, ios::in|ios::binary);
      if (!file) {
        ConWindow::shutdown();
        cerr << "Unable to open " << fileName << " for writing\n";
        exit(1);
      }
      return false;
    }
  }

  if (bufContents < bufSize)
    memset(buffer + bufContents, 0, bufSize - bufContents);

  short x = 0;
  short y = 0;
  bool  hiNib = true;
  bool  ascii = false;
  bool  changed = false;
  KEY_EVENT_RECORD e;

  showEditPrompt();
  win.setCursor(leftMar,1);
  ConWindow::showCursor();

  for (;;) {
    win.setCursor((ascii ? leftMar2 + x : leftMar + 3*x + !hiNib) + (x / 8),
                  y+1);
    ConWindow::readKey(e);

    switch (e.wVirtualKeyCode) {
     case VK_ESCAPE: goto done;
     case VK_TAB:
      hiNib = true;
      ascii = !ascii;
      break;

     case VK_BACK:
     case VK_LEFT:
      if (!hiNib)
        hiNib = true;
      else {
        if (!ascii) hiNib = false;
        if (--x < 0) x = lineWidth-1;
      }
      if (hiNib || (x < lineWidth-1))
        break;
      // else fall thru
     case VK_UP:   if (--y < 0) y = numLines-1; break;

     default: {
       short newByte = -1;
       if ((e.wVirtualKeyCode == VK_RETURN) && other &&
           (other->bufContents > x + y*lineWidth)) {
         newByte = other->line[y][x]; // Copy from other file
         hiNib = ascii; // Always advance cursor to next byte
       } else if (ascii) {
         if (isprint(e.uChar.AsciiChar)) newByte = e.uChar.AsciiChar;
       } else { // hex
         if (isdigit(e.uChar.AsciiChar))
           newByte = e.uChar.AsciiChar - '0';
         else if (isxdigit(e.uChar.AsciiChar))
           newByte = toupper(e.uChar.AsciiChar) - 'A' + 10;
         if (newByte >= 0)
           if (hiNib)
             newByte = (newByte * 0x10) | (0x0F & line[y][x]);
           else
             newByte |= 0xF0 & line[y][x];
       } // end else hex
       if (newByte >= 0) {
         changed = true;
         setByte(x,y,newByte);
       } else
         break;
     } // end default and fall thru
     case VK_RIGHT:
      if (hiNib && !ascii)
        hiNib = false;
      else {
        hiNib = true;
        if (++x >= lineWidth) x = 0;
      }
      if (x || !hiNib)
        break;
      // else fall thru
     case VK_DOWN: if (++y >= numLines) y = 0;  break;

    } // end switch

  } // end forever

 done:
  if (changed) {
    promptWin.clear();
    promptWin.border();
    promptWin.put(30,1,"Save changes (Y/N):");
    promptWin.update();
    promptWin.setCursor(50,1);
    ConWindow::readKey(e);
    if (toupper(e.uChar.AsciiChar) != 'Y') {
      changed = false;
      moveTo(offset);           // Re-read buffer contents
    } else {
      file.seekp(offset);
      file.write(reinterpret_cast<char*>(buffer), bufContents);
    }
  }
  showPrompt();
  ConWindow::hideCursor();
  return changed;
#endif // FIXME
} // end FileDisplay::edit

//--------------------------------------------------------------------
void FileDisplay::setByte(short x, short y, Byte b)
{
  if (x + y*lineWidth >= bufContents) {
    if (x + y*lineWidth > bufContents) {
      short y1 = bufContents / lineWidth;
      short x1 = bufContents % lineWidth;
      while (y1 <= numLines) {
        while (x1 < lineWidth) {
          if ((x1 == x) && (y1 == y)) goto done;
          setByte(x1,y1,0);
          ++x1;
        }
        x1 = 0;
        ++y1;
      } // end while y1
    } // end if more than 1 byte past the end
   done:
    ++bufContents;
    line[y][x] = b ^ 1;         // Make sure it's different
  } // end if past the end

  if (line[y][x] != b) {
    line[y][x] = b;
    char str[3];
    sprintf(str, "%02X", b);
    win.setAttribs(cFileEdit);
    win.put(leftMar + 3*x + (x / 8), y+1, str);
    str[0] = displayTable[b];
    str[1] = '\0';
    win.put(leftMar2 + x + (x / 8), y+1, str);
    win.setAttribs(cFileWin);
    win.update();
  }
} // end FileDisplay::setByte

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
  file.clear();
  file.open(fileName, ios::in|ios::binary);
  writable = false;

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
void displayLockState()
{
  promptWin.putAttribs(63,1,
                       ((lockState == lockBottom) ? cLocked : cBackground),
                       8);
  promptWin.putAttribs(63,2,
                       ((lockState == lockTop)    ? cLocked : cBackground),
                       11);
} // end displayLockState

//--------------------------------------------------------------------
// Display prompt window for editing:
void showEditPrompt()
{
  promptWin.clear();
  promptWin.border();
  promptWin.put(9,1, "\x3C\x5E\x76\x3E move cursor        TAB hex\x3C\x3E"
                "ASCII       ESC done");
  promptWin.put(25,2, "RET copy byte from other file");
  promptWin.putAttribs( 9,1, cPromptKey, 4);
  promptWin.putAttribs(33,1, cPromptKey, 3);
  promptWin.putAttribs(54,1, cPromptKey, 3);
  promptWin.putAttribs(25,2, cPromptKey, 3);
  promptWin.update();
} // end showEditPrompt

//--------------------------------------------------------------------
// Display prompt window:

void showPrompt()
{
  promptWin.clear();
  promptWin.border();
  promptWin.put(1,1, "\x3E fwd 1 byte   \x76 fwd 1 line"
                "  RET next difference  ESC quit  T move top");
  promptWin.put(1,2, "\x3C back 1 byte  \x5E back 1 line   G goto position"
                "      Q quit  B move bottom");
  promptWin.putAttribs( 1,1, cPromptKey, 1);
  promptWin.putAttribs(16,1, cPromptKey, 1);
  promptWin.putAttribs(30,1, cPromptKey, 3);
  promptWin.putAttribs(51,1, cPromptKey, 3);
  promptWin.putAttribs(61,1, cPromptKey, 1);
  promptWin.putAttribs( 1,2, cPromptKey, 1);
  promptWin.putAttribs(16,2, cPromptKey, 1);
  promptWin.putAttribs(32,2, cPromptKey, 1);
  promptWin.putAttribs(53,2, cPromptKey, 1);
  promptWin.putAttribs(61,2, cPromptKey, 1);
  promptWin.update();
} // end showPrompt

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
  inWin.border();
  inWin.put((inWidth-4)/2,0, "\264Goto\303");
  inWin.setAttribs(cPromptWin);
  inWin.hide();

  promptWin.init(0,21, screenWidth,4, cBackground);
  showPrompt();

  file1.init(0,&diffs);
  file2.init(11,&diffs);

  return true;
} // end initialize

//--------------------------------------------------------------------
// Get a command from the keyboard:
//
// Returns:
//   Command code

Command getCommand()
{
  Command  cmd = cmNothing;

  while (cmd == cmNothing) {
    int e = promptWin.readKey();

    switch (toupper(e)) {
     case 0x0D:               // Enter
      cmd = cmNextDiff;
      break;

//FIX     case 0x05:                 // Ctrl+E
//FIX     case 'E':
//FIX      if (e.dwControlKeyState & (LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED))
//FIX        cmd = cmEditBottom;
//FIX      else
//FIX        cmd = cmEditTop;
//FIX      break;
//FIX
//FIX     case 'G':
//FIX      if (e.dwControlKeyState & (LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED))
//FIX        cmd = cmgGoto|cmgGotoBottom;
//FIX      else
//FIX        cmd = cmgGoto|cmgGotoBoth;
//FIX      break;
//FIX
//FIX     case 0x07:               // Ctrl+G
//FIX      cmd = cmgGoto|cmgGotoTop;
//FIX      break;

     case 0x1B:               // Esc
     case 0x03:               // Ctrl+C
     case 'Q':
      cmd = cmQuit;
      break;

     case 'A':  cmd = cmToggleASCII;  break;

     case 'B':  cmd = cmUseBottom;    break;
     case 'T':  cmd = cmUseTop;       break;

     case KEY_DOWN:   cmd = cmmMove|cmmMoveLine|cmmMoveForward;  break;
     case KEY_RIGHT:  cmd = cmmMove|cmmMoveByte|cmmMoveForward;  break;
     case KEY_NPAGE:  cmd = cmmMove|cmmMovePage|cmmMoveForward;  break;
     case KEY_LEFT:   cmd = cmmMove|cmmMoveByte;                 break;
     case KEY_UP:     cmd = cmmMove|cmmMoveLine;                 break;
     case KEY_PPAGE:  cmd = cmmMove|cmmMovePage;                 break;
     case KEY_HOME:   cmd = cmmMove|cmmMoveAll;                  break;
    } // end switch ASCII code
  } // end while no command

  if (cmd & cmmMove) {
    if (lockState != lockTop)    cmd |= cmmMoveTop;
    if (lockState != lockBottom) cmd |= cmmMoveBottom;
  } // end if move command

  return cmd;
} // end getCommand

//--------------------------------------------------------------------
// Get a file position:

void gotoPosition(Command cmd)
{
#if 0
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
#endif
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
  else if ((cmd & cmgGotoMask) == cmgGoto)
    gotoPosition(cmd);
  else if (cmd == cmNextDiff) {
    if (lockState) {
      lockState = lockNeither;
      displayLockState();
    }
    do {
      file1.move(bufSize);
      file2.move(bufSize);
    } while (!diffs.compute());
  } // end else if cmNextDiff
  else if (cmd == cmUseTop) {
    if (lockState == lockBottom)
      lockState = lockNeither;
    else
      lockState = lockBottom;
    displayLockState();
  }
  else if (cmd == cmUseBottom) {
    if (lockState == lockTop)
      lockState = lockNeither;
    else
      lockState = lockTop;
    displayLockState();
  }
  else if (cmd == cmToggleASCII) {
    displayTable = ((displayTable == asciiDisplayTable)
                    ? ebcdicDisplayTable
                    : asciiDisplayTable );
  }
  else if (cmd == cmEditTop)
    file1.edit(&file2);
  else if (cmd == cmEditBottom)
    file2.edit(&file1);

  // Make sure we haven't gone past the end of both files:
  while (diffs.compute() < 0) {
    file1.move(-steps[cmmMovePage]);
    file2.move(-steps[cmmMovePage]);
  }

  file1.display();
  file2.display();
} // end handleCmd

//====================================================================
// Initialization and option processing:
//====================================================================
// Display license information and exit:

bool license(GetOpt*, const GetOpt::Option*, const char*,
             GetOpt::Connection, const char*, int*)
{
  puts(titleString);
  puts("\n"
"This program is free software; you can redistribute it and/or\n"
"modify it under the terms of the GNU General Public License as\n"
"published by the Free Software Foundation; either version 2 of\n"
"the License, or (at your option) any later version.\n"
"\n"
"This program is distributed in the hope that it will be useful,\n"
"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
"GNU General Public License for more details.\n"
"\n"
"You should have received a copy of the GNU General Public License\n"
"along with this program; if not, write to the Free Software\n"
"Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA."
  );

  exit(0);
  return false;                 // Never happens
} // end license

//--------------------------------------------------------------------
// Display version & usage information and exit:
//
// Input:
//   showHelp:    True means display usage information
//   exitStatus:  Status code to pass to exit()

void usage(bool showHelp, int exitStatus)
{
  if (exitStatus > 1)
    cerr << "Try `" << program_name << " --help' for more information.\n";
  else {
    cout << titleString << endl;

    if (showHelp)
      cout << "Usage: " << program_name << " FILE1 FILE2\n\
Compare FILE1 and FILE2 byte by byte.\n\
\n\
Options:\n\
      --help               display this help information and exit\n\
      -L, --license        display license & warranty information and exit\n\
      -V, --version        display version information and exit\n";
  }

  exit(exitStatus);
} // end usage

bool usage(GetOpt* getopt, const GetOpt::Option* option,
           const char*, GetOpt::Connection, const char*, int*)
{
  usage(option->shortName == '?');
  return false;                 // Never happens
} // end usage

//--------------------------------------------------------------------
// Handle options:
//
// Input:
//   argc, argv:  The parameters passed to main
//
// Output:
//   argc, argv:
//     Modified to list only the non-option arguments
//     Note: argv[0] may not be the executable name

void processOptions(int& argc, char**& argv)
{
  static const GetOpt::Option options[] =
  {
    { '?', "help",       NULL, 0, &usage },
    { 'L', "license",    NULL, 0, &license },
    { 'V', "version",    NULL, 0, &usage },
    { 0 }
  };

  GetOpt getopt(options);
  int argi = getopt.process(argc, const_cast<const char**>(argv));
  if (getopt.error)
    usage(true, 1);

  if (argi >= argc)
    argc = 1;           // No arguments
  else {
    argc -= --argi;     // Reduce argc by number of arguments used
    argv += argi;       // And adjust argv[1] to the next argument
  }
} // end processOptions

//====================================================================
// Main Program:
//====================================================================
int main(int argc, char* argv[])
{
  if ((program_name = strrchr(argv[0], '\\')))
    // Isolate the filename:
    ++program_name;
  else
    program_name = argv[0];

  processOptions(argc, argv);

  if (argc != 3)
    usage(1);

  cout << "\
VBinDiff " PACKAGE_VERSION ", Copyright 1995-2005 Christopher J. Madsen\n\
VBinDiff comes with ABSOLUTELY NO WARRANTY; for details type `vbindiff -L'.\n";

  if (!initialize()) {
    cerr << '\n' << program_name << ": Unable to initialize windows\n";
    return 1;
  }

  ostringstream errMsg;

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
  inWin.close();
  promptWin.close();

  ConWindow::shutdown();

  string error(errMsg.str());
  if (error.length()) {
    cerr << '\n' << program_name << ": " << error << endl;
    return 1;
  }
  return 0;
} // end main
