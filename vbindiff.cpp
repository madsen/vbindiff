//--------------------------------------------------------------------
//
//   Visual Binary Diff
//   Copyright 1995-2008 by Christopher J. Madsen
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
//   along with this program.  If not, see <http://www.gnu.org/licenses/>.
//--------------------------------------------------------------------

#include "config.h"

#include <ctype.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include <algorithm>
#include <iostream>
#include <sstream>
#include <map>
#include <string>
#include <vector>
using namespace std;

#include "GetOpt/GetOpt.hpp"

#include "ConWin.hpp"
#include "FileIO.hpp"

const char titleString[] =
  "\nVBinDiff " PACKAGE_VERSION "\nCopyright 1995-2008 Christopher J. Madsen";

void exitMsg(int status, const char* message);
void usage(bool showHelp=true, int exitStatus=0);

//====================================================================
// Type definitions:

typedef unsigned char   Byte;
typedef unsigned short  Word;

typedef Byte  Command;

enum LockState { lockNeither = 0, lockTop, lockBottom };

//--------------------------------------------------------------------
// Strings:

typedef string  String;

typedef String::size_type      StrIdx;
typedef String::iterator       StrItr;
typedef String::const_iterator StrConstItr;

//--------------------------------------------------------------------
// Vectors:

typedef vector<String>          StrVec;
typedef StrVec::iterator        SVItr;
typedef StrVec::const_iterator  SVConstItr;

typedef StrVec::size_type  VecSize;

//--------------------------------------------------------------------
// Map:

typedef map<VecSize, String>    StrMap;
typedef StrMap::value_type      SMVal;
typedef StrMap::iterator        SMItr;
typedef StrMap::const_iterator  SMConstItr;

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
const Command  cmmMoveAll     = 0x03; // Move to beginning or end

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
const Command  cmFind         = 16; // Commands 16-19

const short  leftMar  = 13;     // Starting column of hex display
const short  leftMar2 = 63;     // Starting column of ASCII display

const int  lineWidth = 16;      // Number of bytes displayed per line

const int  promptHeight = 4;    // Height of prompt window
const int  inWidth = 10;        // Width of input window (excluding border)
const int  screenWidth = 80;

const int  maxPath = 260;

const VecSize maxHistory = 2000;

const char hexDigits[] = "0123456789ABCDEF";

#include "tables.h"             // ASCII and EBCDIC tables

//====================================================================
// Class Declarations:

void showEditPrompt();
void showPrompt();

class Difference;

union FileBuffer
{
  Byte  line[1][lineWidth];
  Byte  buffer[lineWidth];
}; // end FileBuffer

class FileDisplay
{
  friend class Difference;

 protected:
  int                bufContents;
  FileBuffer*        data;
  const Difference*  diffs;
  File               file;
  char               fileName[maxPath];
  FPos               offset;
  ConWindow          win;
  bool               writable;
  int                yPos;
 public:
  FileDisplay();
  ~FileDisplay();
  void         init(int y, const Difference* aDiff=NULL,
                    const char* aFileName=NULL);
  void         resize();
  void         shutDown();
  void         display();
  bool         edit(const FileDisplay* other);
  const Byte*  getBuffer() const { return data->buffer; };
  void         move(int step)    { moveTo(offset + step); };
  void         moveTo(FPos newOffset);
  bool         moveTo(const Byte* searchFor, int searchLen);
  void         moveToEnd(FileDisplay* other);
  bool         setFile(const char* aFileName);
 protected:
  void  setByte(short x, short y, Byte b);
}; // end FileDisplay

class Difference
{
  friend void FileDisplay::display();

 protected:
  FileBuffer*         data;
  const FileDisplay*  file1;
  const FileDisplay*  file2;
  int                 numDiffs;
 public:
  Difference(const FileDisplay* aFile1, const FileDisplay* aFile2);
  ~Difference();
  int  compute();
  int  getNumDiffs() const { return numDiffs; };
  void resize();
}; // end Difference

class InputManager
{
 private:
  char*        buf;             // The editing buffer
  const char*  restrict;        // If non-NULL, only allow these chars
  StrVec&      history;         // The history vector to use
  StrMap       historyOverlay;  // Overlay of modified history entries
  VecSize      historyPos;      // The current offset into history[]
  int          maxLen;          // The size of buf (not including NUL)
  int          len;             // The current length of the string
  int          i;               // The current cursor position
  bool         upcase;          // Force all characters to uppercase?
  bool         splitHex;        // Entering space-separated hex bytes?
  bool         insert;          // False for overstrike mode

 public:
  InputManager(char* aBuf, int aMaxLen, StrVec& aHistory);
  bool run();
  void setCharacters(const char* aRestriction) { restrict = aRestriction; };
  void setSplitHex(bool val) { splitHex = val; };
  void setUpcase(bool val)   { upcase = val; };

 private:
  bool normalize(int pos);
  void useHistory(int delta);
}; // end InputManager

//====================================================================
// Global Variables:

String       lastSearch;
StrVec       hexSearchHistory, textSearchHistory, positionHistory;
ConWindow    promptWin,inWin;
FileDisplay  file1, file2;
Difference   diffs(&file1, &file2);
const char*  displayTable = asciiDisplayTable;
const char*  program_name; // Name under which this program was invoked
LockState    lockState = lockNeither;
bool         singleFile = false;

int  numLines  = 9;       // Number of lines of each file to display
int  bufSize   = numLines * lineWidth;
int  linesBetween = 1;    // Number of lines of padding between files

// The number of bytes to move for each possible step size:
//   See cmmMoveByte, cmmMoveLine, cmmMovePage
int  steps[4] = {1, lineWidth, bufSize-lineWidth, 0};


//====================================================================
// Miscellaneous Functions:
//--------------------------------------------------------------------
// Beep the speaker:

#ifdef WIN32_CONSOLE // beep() is defined by ncurses
void beep()
{
  MessageBeep(-1);
} // end beep
#endif // WIN32_CONSOLE

//--------------------------------------------------------------------
// Convert a character to uppercase:
//
// The standard toupper(c) isn't guaranteed for arbitrary integers.

int safeUC(int c)
{
  return (c >= 0 && c <= UCHAR_MAX) ? toupper(c) : c;
} // end safeUC

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
: data(NULL),
  file1(aFile1),
  file2(aFile2)
{
} // end Difference::Difference

//--------------------------------------------------------------------
Difference::~Difference()
{
  delete [] reinterpret_cast<Byte*>(data);
} // end Difference::~Difference

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
  if (singleFile)
    // We return 1 so that cmNextDiff won't keep searching:
    return (file1->bufContents ? 1 : -1);

  memset(data->buffer, 0, bufSize); // Clear the difference table

  int  different = 0;

  const Byte*  buf1 = file1->data->buffer;
  const Byte*  buf2 = file2->data->buffer;

  int  size = min(file1->bufContents, file2->bufContents);

  int  i;
  for (i = 0; i < size; i++)
    if (*(buf1++) != *(buf2++)) {
      data->buffer[i] = true;
      ++different;
    }

  size = max(file1->bufContents, file2->bufContents);

  if (i < size) {
    // One buffer has more data than the other:
    different += size - i;
    for (; i < size; i++)
      data->buffer[i] = true;   // These bytes are only in 1 buffer
  } else if (!size)
    return -1;                  // Both buffers are empty

  numDiffs = different;

  return different;
} // end Difference::compute

//--------------------------------------------------------------------
void Difference::resize()
{
  if (singleFile) return;

  if (data)
    delete [] reinterpret_cast<Byte*>(data);

  data = reinterpret_cast<FileBuffer*>(new Byte[bufSize]);
} // end Difference::resize

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
  data(NULL),
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

  win.init(0,y, screenWidth, (numLines + 1 + ((y==0) ? linesBetween : 0)),
           cFileWin);

  resize();

  if (aFileName)
    setFile(aFileName);
} // end FileDisplay::init

//--------------------------------------------------------------------
// Destructor:

FileDisplay::~FileDisplay()
{
  shutDown();
  CloseFile(file);
  delete [] reinterpret_cast<Byte*>(data);
} // end FileDisplay::~FileDisplay

//--------------------------------------------------------------------
void FileDisplay::resize()
{
  if (data)
    delete [] reinterpret_cast<Byte*>(data);

  data = reinterpret_cast<FileBuffer*>(new Byte[bufSize]);

  // FIXME resize window
} // end FileDisplay::resize

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
  if (!fileName[0]) return;

  FPos  lineOffset = offset;

  short i,j,index,lineLength;
  char  buf[lineWidth + lineWidth/8];
  buf[sizeof(buf)-1] = '\0';

  char  buf2[screenWidth+1];
  buf2[screenWidth] = '\0';

  memset(buf, ' ', sizeof(buf)-1);

  for (i = 0; i < numLines; i++) {
//    cerr << i << '\n';
    char*  str = buf2;
    str +=
      sprintf(str, "%01X %04X %04X:",Word(lineOffset>>32),Word(lineOffset>>16),Word(lineOffset&0xFFFF));

    lineLength  = min(lineWidth, bufContents - i*lineWidth);

    for (j = 0, index = -1; j < lineLength; j++) {
      if (j % 8 == 0) {
        *(str++) = ' ';
        ++index;
      }
      str += sprintf(str, "%02X ", data->line[i][j]);

      buf[index++] = displayTable[data->line[i][j]];
    }
    memset(buf + index, ' ', sizeof(buf) - index - 1);
    memset(str, ' ', screenWidth - (str - buf2));

    win.put(0,i+1, buf2);
    win.put(leftMar2,i+1, buf);

    if (diffs)
      for (j = 0; j < lineWidth; j++)
        if (diffs->data->line[i][j]) {
          win.putAttribs(j*3 + leftMar  + (j>7),i+1, cFileDiff,2);
          win.putAttribs(j   + leftMar2 + (j>7),i+1, cFileDiff,1);
        }
    lineOffset += lineWidth;
  } // end for i up to numLines

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
  if (!bufContents && offset)
    return false;               // You must not be completely past EOF

  if (!writable) {
    File w = OpenFile(fileName, true);
    if (w == InvalidFile) return false;
    CloseFile(file);
    file = w;
    writable = true;
  }

  if (bufContents < bufSize)
    memset(data->buffer + bufContents, 0, bufSize - bufContents);

  short x = 0;
  short y = 0;
  bool  hiNib = true;
  bool  ascii = false;
  bool  changed = false;
  int   key;

  const Byte *const inputTable = ((displayTable == ebcdicDisplayTable)
                                  ? ascii2ebcdicTable
                                  : NULL); // No translation

  showEditPrompt();
  win.setCursor(leftMar,1);
  ConWindow::showCursor();

  for (;;) {
    win.setCursor((ascii ? leftMar2 + x : leftMar + 3*x + !hiNib) + (x / 8),
                  y+1);
    key = win.readKey();

    switch (key) {
     case KEY_ESCAPE: goto done;
     case KEY_TAB:
      hiNib = true;
      ascii = !ascii;
      break;

     case KEY_DELETE:
     case KEY_BACKSPACE:
     case KEY_LEFT:
      if (!hiNib)
        hiNib = true;
      else {
        if (!ascii) hiNib = false;
        if (--x < 0) x = lineWidth-1;
      }
      if (hiNib || (x < lineWidth-1))
        break;
      // else fall thru
     case KEY_UP:   if (--y < 0) y = numLines-1; break;

     default: {
       short newByte = -1;
       if ((key == KEY_RETURN) && other &&
           (other->bufContents > x + y*lineWidth)) {
         newByte = other->data->line[y][x]; // Copy from other file
         hiNib = ascii; // Always advance cursor to next byte
       } else if (ascii) {
         if (isprint(key)) newByte = (inputTable ? inputTable[key] : key);
       } else { // hex
         if (isdigit(key))
           newByte = key - '0';
         else if (isxdigit(key))
           newByte = safeUC(key) - 'A' + 10;
         if (newByte >= 0) {
           if (hiNib)
             newByte = (newByte * 0x10) | (0x0F & data->line[y][x]);
           else
             newByte |= 0xF0 & data->line[y][x];
         } // end if valid digit entered
       } // end else hex
       if (newByte >= 0) {
         changed = true;
         setByte(x,y,newByte);
       } else
         break;
     } // end default and fall thru
     case KEY_RIGHT:
      if (hiNib && !ascii)
        hiNib = false;
      else {
        hiNib = true;
        if (++x >= lineWidth) x = 0;
      }
      if (x || !hiNib)
        break;
      // else fall thru
     case KEY_DOWN: if (++y >= numLines) y = 0;  break;

    } // end switch

  } // end forever

 done:
  if (changed) {
    promptWin.clear();
    promptWin.border();
    promptWin.put(30,1,"Save changes (Y/N):");
    promptWin.update();
    promptWin.setCursor(50,1);
    key = promptWin.readKey();
    if (safeUC(key) != 'Y') {
      changed = false;
      moveTo(offset);           // Re-read buffer contents
    } else {
      SeekFile(file, offset);
      WriteFile(file, data->buffer, bufContents);
    }
  }
  showPrompt();
  ConWindow::hideCursor();
  return changed;
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
    data->line[y][x] = b ^ 1;         // Make sure it's different
  } // end if past the end

  if (data->line[y][x] != b) {
    data->line[y][x] = b;
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

void FileDisplay::moveTo(FPos newOffset)
{
  if (!fileName[0]) return;     // No file

  offset = newOffset;

  if (offset < 0)
    offset = 0;

  SeekFile(file, offset);
  bufContents = ReadFile(file, data->buffer, bufSize);
} // end FileDisplay::moveTo

//--------------------------------------------------------------------
// Change the file position by searching:
//
// Changes the file offset and updates the buffer.
// Does not update the display.
//
// Input:
//   searchFor:  The bytes to search for
//   searchLen:  The number of bytes in searchFor
//
// Returns:
//   true:   The search was successful
//   false:  Search unsuccessful, file not moved

bool FileDisplay::moveTo(const Byte* searchFor, int searchLen)
{
  if (!fileName[0]) return true; // No file, pretend success

  // Using algorithm based on QuickSearch:
  //   http://www-igm.univ-mlv.fr/~lecroq/string/node19.htm

  // Compute offset table:
  int i;
  int moveOver[256];

  for (i = 0; i < 256; ++i)
    moveOver[i] = searchLen + 1;
  for (i = 0; i < searchLen; ++i)
    moveOver[searchFor[i]] = searchLen - i;

  // Prepare the search buffer:

  const int
    blockSize  = 8 * 1024,
    moveLength = searchLen,
    restartAt  = blockSize - moveLength,
    fullStop   = blockSize * 2 - moveLength;

  Byte *const  searchBuf = new Byte[2 * blockSize];

  Byte *const  copyTo         = searchBuf + restartAt;
  const Byte *const copyFrom  = searchBuf + fullStop;

  char *const  readAt = reinterpret_cast<char*>(searchBuf) + blockSize;

  FPos  newPos = offset + 1;

  SeekFile(file, newPos);
  Size bytesRead = ReadFile(file, searchBuf, blockSize * 2);
  int stopAt = bytesRead - moveLength;

  // Start the search:
  i = 0;
  for (;;) {
    if (stopAt < fullStop) ++stopAt;

    while (i < stopAt) {
      if (memcmp(searchFor, searchBuf + i, searchLen) == 0)
        goto done;

      i += moveOver[searchBuf[i + searchLen]]; // shift
    } // end while more buffer to search

    if (stopAt != fullStop) {
      i = -1;
      goto done;
    } // Nothing more to read

    newPos += blockSize;
    i -= blockSize;
    memcpy(copyTo, copyFrom, moveLength);
    bytesRead = ReadFile(file, readAt, blockSize);
    stopAt = bytesRead + blockSize - moveLength;
  } // end forever

 done:
  delete [] searchBuf;

  if (i < 0) return false;      // No match

  moveTo(newPos + i);

  return true;
} // end FileDisplay::moveTo

//--------------------------------------------------------------------
// Move to the end of the file:
//
// Input:
//   other:  If non NULL, move both files to the end of the shorter file

void FileDisplay::moveToEnd(FileDisplay* other)
{
  if (!fileName[0]) return;     // No file

  FPos  end = SeekFile(file, 0, SeekEnd);
  FPos  diff = 0;

  if (other) {
    // If the files aren't currently at the same position,
    // we want to keep them offset by the same amount:
    diff = other->offset - offset;

    end = min(end, SeekFile(other->file, 0, SeekEnd) - diff);
  } // end if moving other file too

  end -= steps[cmmMovePage];
  end -= end % 0x10;

  moveTo(end);
  if (other) other->moveTo(end + diff);
} // end FileDisplay::moveToEnd

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
//   False:  Unable to open file (call ErrorMsg for error message)

bool FileDisplay::setFile(const char* aFileName)
{
  strncpy(fileName, aFileName, maxPath);
  fileName[maxPath-1] = '\0';

  win.put(0,0, fileName);
  win.putAttribs(0,0, cFileName, screenWidth);
  win.update();                 // FIXME

  bufContents = 0;
  file = OpenFile(fileName);
  writable = false;

  if (file == InvalidFile)
    return false;

  offset = 0;
  bufContents = ReadFile(file, data->buffer, bufSize);

  return true;
} // end FileDisplay::setFile

//====================================================================
// Main Program:
//--------------------------------------------------------------------
void calcScreenLayout(bool resize = true)
{
  int  screenX, screenY;

  ConWindow::getScreenSize(screenX, screenY);

  if (screenX < screenWidth) {
    ostringstream  err;
    err << "The screen must be at least "
        << screenWidth << " characters wide.";
    exitMsg(2, err.str().c_str());
  }

  if (screenY < promptHeight + 4) {
    ostringstream  err;
    err << "The screen must be at least "
        << (promptHeight + 4) << " lines high.";
    exitMsg(2, err.str().c_str());
  }

  numLines = screenY - promptHeight - (singleFile ? 1 : 2);

  if (singleFile)
    linesBetween = 0;
  else {
    linesBetween = numLines % 2;
    numLines = (numLines - linesBetween) / 2;
  }

  bufSize = numLines * lineWidth;

  steps[cmmMovePage] = bufSize-lineWidth;

  // FIXME resize existing windows
} // end calcScreenLayout

//--------------------------------------------------------------------
void displayCharacterSet()
{
  const bool isASCII = (displayTable == asciiDisplayTable);

  promptWin.putAttribs(3,2, (isASCII ? cCurrentMode : cBackground), 5);
  promptWin.putAttribs(9,2, (isASCII ? cBackground : cCurrentMode), 6);

  promptWin.update();
} // end displayCharacterSet

//--------------------------------------------------------------------
void displayLockState()
{
#ifndef WIN32_CONSOLE     // The Win32 version uses Ctrl & Alt instead
  if (singleFile) return;

  promptWin.putAttribs(63,1,
                       ((lockState == lockBottom) ? cCurrentMode : cBackground),
                       8);
  promptWin.putAttribs(63,2,
                       ((lockState == lockTop)    ? cCurrentMode : cBackground),
                       11);
#endif
} // end displayLockState

//--------------------------------------------------------------------
// Print a message to stderr and exit:
//
// Input:
//   status:   The exit status to use
//   message:  The message to print

void exitMsg(int status, const char* message)
{
  ConWindow::shutdown();

  cerr << endl << message << endl;
  exit(status);
} // end exitMsg

//--------------------------------------------------------------------
// Normalize the string in the input window:
//
// Does nothing unless splitHex mode is active.
//
// Input:
//   pos:  The position of the cursor in buf
//
// Returns:
//   true:   The input buffer was changed
//   false:  No changes were necessary

bool InputManager::normalize(int pos)
{
  if (!splitHex) return false;

  // Change D_ to 0D:
  if (pos && buf[pos] == ' ' && buf[pos-1] != ' ') {
    buf[pos] = buf[pos-1];
    buf[pos-1] = '0';
    if (pos == len) len += 2;
    return true;
  }

  // Change _D to 0D:
  if (pos < len && buf[pos] == ' ' && buf[pos+1] != ' ') {
    buf[pos] = '0';
    return true;
  }

  return false;                 // No changes necessary
} // end InputManager::normalize

//--------------------------------------------------------------------
// Get a string using inWin:
//
// Input:
//   buf:       The buffer where the string will be stored
//   maxLen:    The maximum number of chars to accept (not including NUL byte)
//   history:   The history vector to use
//   restrict:  If not NULL, accept only chars in this string
//   upcase:    If true, convert all chars with safeUC

void getString(char* buf, int maxLen, StrVec& history,
               const char* restrict=NULL,
               bool upcase=false, bool splitHex=false)
{
  InputManager  manager(buf, maxLen, history);

  manager.setCharacters(restrict);
  manager.setSplitHex(splitHex);
  manager.setUpcase(upcase);

  manager.run();
} // end getString

//--------------------------------------------------------------------
// Construct the InputManager object:
//
// Input:
//   aBuf:      The buffer where the string will be stored
//   aMaxLen:   The maximum number of chars to accept (not including NUL byte)
//   aHistory:  The history vector to use

InputManager::InputManager(char* aBuf, int aMaxLen, StrVec& aHistory)
: buf(aBuf),
  restrict(NULL),
  history(aHistory),
  historyPos(aHistory.size()),
  maxLen(aMaxLen),
  len(0),
  i(0),
  upcase(false),
  splitHex(false),
  insert(true)
{
} // end InputManager

//--------------------------------------------------------------------
// Run the main loop to get an input string:
//
// Returns:
//   true:   Enter was pressed
//   false:  Escape was pressed

bool InputManager::run()
{
  inWin.setCursor(2,1);

  bool  inWinShown = false;
  bool  done   = false;
  bool  aborted = true;

  ConWindow::showCursor(insert);

  memset(buf, ' ', maxLen);
  buf[maxLen] = '\0';

  // We need to be able to display complete bytes:
  if (splitHex && (maxLen % 3 == 1)) --maxLen;

  // Main input loop:
  while (!done) {
    inWin.put(2,1,buf);
    if (inWinShown) inWin.update(1); // Only update inside the box
    else { inWin.update();   inWinShown = true; } // Show the input window
    inWin.setCursor(2+i,1);
    int key = inWin.readKey();
    if (upcase) key = safeUC(key);

    switch (key) {
     case KEY_ESCAPE:  buf[0] = '\0';  done = true;  break; // ESC

     case KEY_RETURN:           // Enter
      normalize(i);
      buf[len] = '\0';
      done = true;
      aborted = false;
      break;

     case KEY_BACKSPACE:
     case KEY_DELETE:           // Backspace on most Unix terminals
     case 0x08:                 // Backspace (Ctrl-H)
      if (!i) continue; // Can't back up if we're at the beginning already
      if (splitHex) {
        if ((i % 3) == 0) {
          // At the beginning of a byte; erase last digit of previous byte:
          if (i == len) len -= 2;
          i -= 2;
          buf[i] = ' ';
        } else if (i < len && buf[i] != ' ') {
          // On the second digit; erase the first digit:
          buf[--i] = ' ';
        } else {
          // On a blank second digit; delete the entire byte:
          buf[--i] = ' ';
          memmove(buf + i, buf + i + 3, maxLen - i - 3);
          len -= 3;
          if (len < i) len = i;
        }
      } else { // not splitHex mode
        memmove(buf + i - 1, buf + i, maxLen - i);
        buf[maxLen-1] = ' ';
        --len;  --i;
      } // end else not splitHex mode
      break;

     case 0x04:                 // Ctrl-D
     case KEY_DC:
      if (i >= len) continue;
      if (splitHex) {
        i -= i%3;
        memmove(buf + i, buf + i + 3, maxLen - i - 3);
        len -= 3;
        if (len < i) len = i;
      } else {
        memmove(buf + i, buf + i + 1, maxLen - i - 1);
        buf[maxLen-1] = ' ';
        --len;
      } // end else not splitHex mode
      break;

     case KEY_IC:
      insert = !insert;
      ConWindow::showCursor(insert);
      break;

     case 0x02:                 // Ctrl-B
     case KEY_LEFT:
      if (i) {
        --i;
        if (splitHex) {
          normalize(i+1);
          if (i % 3 == 2) --i;
        }
      }
      break;

     case 0x06:                 // Ctrl-F
     case KEY_RIGHT:
      if (i < len) {
        ++i;
        if (splitHex) {
          normalize(i-1);
          if ((i < maxLen) && (i % 3 == 2)) ++i;
        }
      }
      break;

     case 0x0B:                 // Ctrl-K
      if (len > i) {
        memset(buf + i, ' ', len - i);
        len = i;
      }
      break;

     case 0x01:                 // Ctrl-A
     case KEY_HOME:
      normalize(i);
      i = 0;
      break;

     case 0x05:                 // Ctrl-E
     case KEY_END:
      if (splitHex && (i < len))
        normalize(i);
      i = len;
      break;

     case 0x10:                 // Ctrl-P
     case KEY_UP:
      if (historyPos == 0) beep();
      else                 useHistory(-1);
      break;

     case 0x0E:                 // Ctrl-N
     case KEY_DOWN:
      if (historyPos == history.size()) beep();
      else                              useHistory(+1);
      break;

     default:
      if (isprint(key) && (!restrict || strchr(restrict, key))) {
        if (insert) {
          if (splitHex) {
            if (buf[i] == ' ') {
              if (i >= maxLen) continue;
            } else {
              if (len >= maxLen) continue;
              i -= i % 3;
              memmove(buf + i + 3, buf + i, maxLen - i - 3);
              buf[i+1] = ' ';
              len += 3;
            }
          } // end if splitHex mode
          else {
            if (len >= maxLen) continue;
            memmove(buf + i + 1, buf + i, maxLen - i - 1);
            ++len;
          } // end else not splitHex mode
        } else { // overstrike mode
          if (i >= maxLen) continue;
        } // end else overstrike mode
        buf[i++] = key;
        if (splitHex && (i < maxLen) && (i % 3 == 2))
          ++i;
        if (i > len) len = i;
      } // end if is acceptable character to insert
    } // end switch key
  } // end while not done

  // Hide the input window & cursor:
  ConWindow::hideCursor();
  inWin.hide();

  // Record the result in the history:
  if (!aborted && len) {
    String  newValue(buf);

    SVItr  exists = find(history.begin(), history.end(), newValue);
    if (exists != history.end())
      // Already in history.  Move it to the end:
      rotate(exists, exists + 1, history.end());
    else if (history.size() >= maxHistory) {
      // History is full.  Replace the first entry & move it to the end:
      history.front().swap(newValue);
      rotate(history.begin(), history.begin() + 1, history.end());
    } else
      // Just append to history:
      history.push_back(newValue);
  } // end if we have a value to store in the history

  return !aborted;
} // end run

//--------------------------------------------------------------------
// Switch the current input line with one from the history:
//
// Input:
//   delta:  The number to add to historyPos (-1 previous, +1 next)

void InputManager::useHistory(int delta)
{
  // Clean up the current string if necessary:
  normalize(i);

  // Update the history overlay if necessary:
  //   We always store the initial value, because it doesn't
  //   correspond to a valid entry in history.
  if (len || historyPos == history.size())
    historyOverlay[historyPos].assign(buf, len);

  // Look for an entry in the overlay:
  SMItr itr = historyOverlay.find(historyPos += delta);

  String& s = ((itr == historyOverlay.end())
               ? history[historyPos] : itr->second);

  // Store the new string in the buffer:
  memset(buf, ' ', maxLen);
  i = len = min(static_cast<VecSize>(maxLen), s.length());
  memcpy(buf, s.c_str(), len);
} // end useHistory

//--------------------------------------------------------------------
// Convert hex string to bytes:
//
// Input:
//   buf:  Must contain a well-formed string of hex characters
//         (each byte must be separated by spaces)
//
// Output:
//   buf:  Contains the translated bytes
//
// Returns:
//   The number of bytes in buf

int packHex(Byte* buf)
{
  unsigned long val;

  char* in  = reinterpret_cast<char*>(buf);
  Byte* out = buf;

  while (*in) {
    if (*in == ' ')
      ++in;
    else {
      val = strtoul(in, &in, 16);
      *(out++) = Byte(val);
    }
  }

  return out - buf;
} // end packHex

//--------------------------------------------------------------------
// Position the input window:
//
// Input:
//   cmd:    Indicates where the window should be positioned
//   width:  The width of the window
//   title:  The title for the window

void positionInWin(Command cmd, short width, const char* title)
{
  inWin.resize(width, 3);
  inWin.move((screenWidth-width)/2,
             ((!singleFile && (cmd & cmgGotoBottom))
              ? ((cmd & cmgGotoTop)
                 ? numLines + linesBetween                   // Moving both
                 : numLines + numLines/2 + 1 + linesBetween) // Moving bottom
              : numLines/2));                                // Moving top

  inWin.border();
  inWin.put((width-strlen(title))/2,0, title);
} // end positionInWin

//--------------------------------------------------------------------
// Display prompt window for editing:

void showEditPrompt()
{
  promptWin.clear();
  promptWin.border();
  promptWin.put(3,1, "Arrow keys move cursor        TAB hex\x3C\x3E"
                "ASCII       ESC done");
  if (displayTable == ebcdicDisplayTable)
    promptWin.put(42,1, "EBCDIC");

  promptWin.putAttribs( 3,1, cPromptKey, 10);
  promptWin.putAttribs(33,1, cPromptKey, 3);
  promptWin.putAttribs(54,1, cPromptKey, 3);

  if (!singleFile) {
    promptWin.put(25,2, "RET copy byte from other file");
    promptWin.putAttribs(25,2, cPromptKey, 3);
  }
  promptWin.update();
} // end showEditPrompt

//--------------------------------------------------------------------
// Display prompt window:

void showPrompt()
{
  promptWin.clear();
  promptWin.border();

#ifdef WIN32_CONSOLE
  promptWin.put(1,1, "Arrow keys move  F find      "
                "RET next difference  ESC quit  ALT  freeze top");
  promptWin.put(1,2, "C ASCII/EBCDIC   E edit file   "
                "G goto position      Q quit  CTRL freeze bottom");
  const short
    topBotLength = 4,
    topLength    = 15;
#else // curses
  promptWin.put(1,1, "Arrow keys move  F find      "
                "RET next difference  ESC quit  T move top");
  promptWin.put(1,2, "C ASCII/EBCDIC   E edit file   "
                "G goto position      Q quit  B move bottom");
  const short
    topBotLength = 1,
    topLength    = 10;
#endif

  promptWin.putAttribs( 1,1, cPromptKey, 10);
  promptWin.putAttribs(18,1, cPromptKey, 1);
  promptWin.putAttribs(30,1, cPromptKey, 3);
  promptWin.putAttribs(51,1, cPromptKey, 3);
  promptWin.putAttribs( 1,2, cPromptKey, 1);
  promptWin.putAttribs(18,2, cPromptKey, 1);
  promptWin.putAttribs(32,2, cPromptKey, 1);
  promptWin.putAttribs(53,2, cPromptKey, 1);
  if (singleFile) {
    // Erase "move top" & "move bottom":
    promptWin.putChar(61,1, ' ', topLength);
    promptWin.putChar(61,2, ' ', topLength + 3);
  } else {
    promptWin.putAttribs(61,1, cPromptKey, topBotLength);
    promptWin.putAttribs(61,2, cPromptKey, topBotLength);
  }
  displayLockState();
  displayCharacterSet();        // Calls promptWin.update()
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

  calcScreenLayout(false);

  inWin.init(0,0, inWidth+2,3, cPromptBdr);
  inWin.border();
  inWin.put((inWidth-4)/2,0, " Goto ");
  inWin.setAttribs(cPromptWin);
  inWin.hide();

  int y;
  if (singleFile) y = numLines + 1;
  else            y = numLines * 2 + linesBetween + 2;

  promptWin.init(0,y, screenWidth,promptHeight, cBackground);
  showPrompt();

  if (!singleFile) diffs.resize();

  file1.init(0, (singleFile ? NULL : &diffs));

  if (!singleFile) file2.init(numLines + linesBetween + 1, &diffs);

  return true;
} // end initialize

//--------------------------------------------------------------------
// Get a command from the keyboard:
//
// Returns:
//   Command code

#ifdef WIN32_CONSOLE
Command getCommand()
{
  KEY_EVENT_RECORD e;
  Command  cmd = cmNothing;

  while (cmd == cmNothing) {
    ConWindow::readKey(e);

    switch (safeUC(e.uChar.AsciiChar)) {
     case KEY_RETURN:           // Enter
     case ' ':                  // Space
      cmd = cmNextDiff;
      break;

     case 0x05:                 // Ctrl+E
     case 'E':
      if (e.dwControlKeyState & (LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED))
        cmd = cmEditBottom;
      else
        cmd = cmEditTop;
      break;

     case 'F':
      if (e.dwControlKeyState & (LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED))
        cmd = cmFind|cmgGotoBottom;
      else
        cmd = cmFind|cmgGotoBoth;
      break;

     case 0x06:               // Ctrl+F
      cmd = cmFind|cmgGotoTop;
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

     case KEY_ESCAPE:         // Esc
     case 0x03:               // Ctrl+C
     case 'Q':
      cmd = cmQuit;
      break;

     case 'C':  cmd = cmToggleASCII;  break;

     default:                 // Try extended codes
      switch (e.wVirtualKeyCode) {
       case VK_DOWN:   cmd = cmmMove|cmmMoveLine|cmmMoveForward;  break;
       case VK_RIGHT:  cmd = cmmMove|cmmMoveByte|cmmMoveForward;  break;
       case VK_NEXT:   cmd = cmmMove|cmmMovePage|cmmMoveForward;  break;
       case VK_END:    cmd = cmmMove|cmmMoveAll|cmmMoveForward;   break;
       case VK_LEFT:   cmd = cmmMove|cmmMoveByte;                 break;
       case VK_UP:     cmd = cmmMove|cmmMoveLine;                 break;
       case VK_PRIOR:  cmd = cmmMove|cmmMovePage;                 break;
       case VK_HOME:   cmd = cmmMove|cmmMoveAll;                  break;
      } // end switch virtual key code
      break;
    } // end switch ASCII code
  } // end while no command

  if (cmd & cmmMove) {
    if ((e.dwControlKeyState & (LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED)) == 0)
      cmd |= cmmMoveTop;
    if ((e.dwControlKeyState & (LEFT_CTRL_PRESSED|RIGHT_CTRL_PRESSED)) == 0)
      cmd |= cmmMoveBottom;
  } // end if move command

  return cmd;
} // end getCommand

#else // using curses interface
Command getCommand()
{
  Command  cmd = cmNothing;

  while (cmd == cmNothing) {
    int e = promptWin.readKey();

    switch (safeUC(e)) {
     case KEY_RETURN:           // Enter
     case ' ':                  // Space
      cmd = cmNextDiff;
      break;

     case 'E':
      if (lockState == lockTop)
        cmd = cmEditBottom;
      else
        cmd = cmEditTop;
      break;

     case 'F':
      cmd = cmFind;
      if (lockState != lockTop)    cmd |= cmgGotoTop;
      if (lockState != lockBottom) cmd |= cmgGotoBottom;
      break;

     case 'G':
      cmd = cmgGoto;
      if (lockState != lockTop)    cmd |= cmgGotoTop;
      if (lockState != lockBottom) cmd |= cmgGotoBottom;
      break;

     case KEY_ESCAPE:
     case 0x03:               // Ctrl+C
     case 'Q':
      cmd = cmQuit;
      break;

     case 'C':  cmd = cmToggleASCII;  break;

     case 'B':  if (!singleFile) cmd = cmUseBottom;              break;
     case 'T':  if (!singleFile) cmd = cmUseTop;                 break;

     case KEY_DOWN:   cmd = cmmMove|cmmMoveLine|cmmMoveForward;  break;
     case KEY_RIGHT:  cmd = cmmMove|cmmMoveByte|cmmMoveForward;  break;
     case KEY_NPAGE:  cmd = cmmMove|cmmMovePage|cmmMoveForward;  break;
     case KEY_END:    cmd = cmmMove|cmmMoveAll|cmmMoveForward;   break;
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
#endif  // end else curses interface

//--------------------------------------------------------------------
// Get a file position and move there:

void gotoPosition(Command cmd)
{
  positionInWin(cmd, inWidth+3, " Goto ");

  const int  maxLen = inWidth-1;
  char  buf[maxLen+1];

  getString(buf, maxLen, positionHistory, hexDigits, true);

  if (!buf[0])
    return;

  FPos  pos = _strtoui64(buf, NULL, 16);

  if (cmd & cmgGotoTop)
    file1.moveTo(pos);
  if (cmd & cmgGotoBottom)
    file2.moveTo(pos);
} // end gotoPosition

//--------------------------------------------------------------------
// Search for text or bytes in the files:

void searchFiles(Command cmd)
{
  const bool havePrev = !lastSearch.empty();

  positionInWin(cmd, (havePrev ? 47 : 32), " Find ");

  inWin.put(2, 1,"H Hex search   T Text search");
  inWin.putAttribs( 2,1, cPromptKey, 1);
  inWin.putAttribs(17,1, cPromptKey, 1);
  if (havePrev) {
    inWin.put(33, 1,"N Next match");
    inWin.putAttribs(33,1, cPromptKey, 1);
  }
  inWin.update();
  int key = safeUC(inWin.readKey());

  bool hex = false;

  if (key == KEY_ESCAPE) {
    inWin.hide();
    return;
  } else if (key == 'H')
    hex = true;

  if (key == 'N' && havePrev) {
    inWin.hide();
  } else {
    positionInWin(cmd, screenWidth, (hex ? " Find Hex Bytes" : " Find Text "));

    const int  maxLen = screenWidth-4;
    Byte  buf[maxLen+1];
    int   searchLen;

    if (hex) {
      getString(reinterpret_cast<char*>(buf), maxLen, hexSearchHistory, hexDigits, true, true);
      searchLen = packHex(buf);
    } else {
      getString(reinterpret_cast<char*>(buf), maxLen, textSearchHistory);

      searchLen = strlen(reinterpret_cast<char*>(buf));
      if (displayTable == ebcdicDisplayTable) {
        for (int i = 0; i < searchLen; ++i)
          buf[i] = ascii2ebcdicTable[buf[i]];
      } // end if in EBCDIC mode
    } // end else text search

    if (!searchLen) return;

    lastSearch.assign(reinterpret_cast<char*>(buf), searchLen);
  } // end else need to read search string

  bool problem = false;
  const Byte *const  searchPattern =
    reinterpret_cast<const Byte*>(lastSearch.c_str());

  if ((cmd & cmgGotoTop) &&
      !file1.moveTo(searchPattern, lastSearch.length()))
    problem = true;
  if ((cmd & cmgGotoBottom) &&
      !file2.moveTo(searchPattern, lastSearch.length()))
    problem = true;

  if (problem) beep();
} // end searchFiles

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

    if ((cmd & cmmMoveForward) && !step) {
      if (cmd & cmmMoveTop)
        file1.moveToEnd((!singleFile && (cmd & cmmMoveBottom)) ? &file2 : NULL);
      else
        file2.moveToEnd(NULL);
    } else {
      if (cmd & cmmMoveTop) {
        if (step)
          file1.move(step);
        else
          file1.moveTo(0);
      } // end if moving top file

      if (cmd & cmmMoveBottom) {
        if (step)
          file2.move(step);
        else
          file2.moveTo(0);
      } // end if moving bottom file
    } // end else not moving to end
  } // end if move
  else if ((cmd & cmgGotoMask) == cmgGoto)
    gotoPosition(cmd);
  else if ((cmd & cmgGotoMask) == cmFind)
    searchFiles(cmd);
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
    displayCharacterSet();
  }
  else if (cmd == cmEditTop)
    file1.edit(singleFile ? NULL : &file2);
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
"along with this program; if not, see <http://www.gnu.org/licenses/>."
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
      cout << "Usage: " << program_name << " FILE1 [FILE2]\n\
Compare FILE1 and FILE2 byte by byte.\n\
If FILE2 is omitted, just display FILE1.\n\
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

  if (argc < 2 || argc > 3)
    usage(1);

  cout << "\
VBinDiff " PACKAGE_VERSION ", Copyright 1995-2008 Christopher J. Madsen\n\
VBinDiff comes with ABSOLUTELY NO WARRANTY; for details type `vbindiff -L'.\n";

  singleFile = (argc == 2);
  if (!initialize()) {
    cerr << '\n' << program_name << ": Unable to initialize windows\n";
    return 1;
  }

  {
    ostringstream errMsg;

    if (!file1.setFile(argv[1])) {
      const char* errStr = ErrorMsg();
      errMsg << "Unable to open " << argv[1] << ": " << errStr;
    }
    else if (!singleFile && !file2.setFile(argv[2])) {
      const char* errStr = ErrorMsg();
      errMsg << "Unable to open " << argv[2] << ": " << errStr;
    }
    string error(errMsg.str());
    if (error.length())
      exitMsg(1, error.c_str());
  } // end block around errMsg

  diffs.compute();

  file1.display();
  file2.display();

  Command  cmd;
  while ((cmd = getCommand()) != cmQuit)
    handleCmd(cmd);

  file1.shutDown();
  file2.shutDown();
  inWin.close();
  promptWin.close();

  ConWindow::shutdown();

  return 0;
} // end main

//--------------------------------------------------------------------
// Local Variables:
//     c-file-style: "cjm"
// End:
