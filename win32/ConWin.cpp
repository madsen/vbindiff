//--------------------------------------------------------------------
//
//   VBinDiff
//   Copyright 1997-2008 by Christopher J. Madsen
//
//   Support class for Win32 console mode applications
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

#include "ConWin.hpp"

void exitMsg(int status, const char* message); // From vbindiff.cpp

//====================================================================
// Colors:
//--------------------------------------------------------------------

#define F_BLACK 0
#define F_RED   FOREGROUND_RED
#define F_WHITE (FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE)
#define F_YELLOW (FOREGROUND_GREEN|FOREGROUND_RED)
#define B_BLUE  BACKGROUND_BLUE
#define B_WHITE (BACKGROUND_RED|BACKGROUND_GREEN|BACKGROUND_BLUE)

static const WORD colorStyle[] = {
  F_WHITE|B_BLUE,                       // cBackground
  F_WHITE|B_BLUE,                       // cPromptWin
  F_WHITE|B_BLUE|FOREGROUND_INTENSITY,  // cPromptKey
  F_WHITE|B_BLUE|FOREGROUND_INTENSITY,  // cPromptBdr
  F_BLACK|B_WHITE,                      // cCurrentMode
  F_BLACK|B_WHITE,                      // cFileName
  F_WHITE|B_BLUE,                       // cFileWin
  F_RED|B_BLUE|FOREGROUND_INTENSITY,    // cFileDiff
  F_YELLOW|B_BLUE|FOREGROUND_INTENSITY  // cFileEdit
};

//====================================================================
// Class ConWindow:
//--------------------------------------------------------------------
HANDLE  ConWindow::inBuf  = INVALID_HANDLE_VALUE;
HANDLE  ConWindow::scrBuf = INVALID_HANDLE_VALUE;

static DWORD  origInMode = 0;    // The original input mode

//////////////////////////////////////////////////////////////////////
// Static Member Functions:
//--------------------------------------------------------------------
// Start up the window system:
//
// Allocates a screen buffer and sets input mode:
//
// Returns:
//   true:   Everything set up properly
//   false:  Unable to initialize

bool ConWindow::startup()
{
  inBuf = GetStdHandle(STD_INPUT_HANDLE);
  if (inBuf == INVALID_HANDLE_VALUE)
    return false;

  scrBuf = CreateConsoleScreenBuffer(GENERIC_READ|GENERIC_WRITE,
                                     0, NULL, // No sharing
                                     CONSOLE_TEXTMODE_BUFFER, NULL);

  if (scrBuf == INVALID_HANDLE_VALUE)
    return false;

  if (!SetConsoleActiveScreenBuffer(scrBuf) ||
      !SetConsoleMode(scrBuf, 0) ||
      !GetConsoleMode(inBuf, &origInMode) ||
      !SetConsoleMode(inBuf, 0)) {
    CloseHandle(scrBuf);
    return false;
  }

  return true;
} // end ConWindow::startup

//--------------------------------------------------------------------
// Shut down the window system:
//
// Deallocate the screen buffer and restore the original input mode.

void ConWindow::shutdown()
{
  if (origInMode)
    SetConsoleMode(inBuf, origInMode);
  if (scrBuf != INVALID_HANDLE_VALUE)
    CloseHandle(scrBuf);
  scrBuf = INVALID_HANDLE_VALUE;
} // end ConWindow::shutdown

//--------------------------------------------------------------------
// Return the current screen size:

void ConWindow::getScreenSize(int& x, int& y)
{
  CONSOLE_SCREEN_BUFFER_INFO  info;

  if (GetConsoleScreenBufferInfo(scrBuf, &info)) {
    x = info.dwSize.X;
    y = info.dwSize.Y;
  } else {
    x = y = 0;
  }
} // end ConWindow::getScreenSize

//--------------------------------------------------------------------
// Make the cursor invisible:

void ConWindow::hideCursor()
{
  CONSOLE_CURSOR_INFO  info;

  if (GetConsoleCursorInfo(scrBuf, &info)) {
    info.bVisible = FALSE;
    SetConsoleCursorInfo(scrBuf, &info);
  }
} // end ConWindow::hideCursor

//--------------------------------------------------------------------
// Read the next key down event:
//
// Output:
//   event:  Contains a key down event

void ConWindow::readKey(KEY_EVENT_RECORD& event)
{
  INPUT_RECORD  e;

  for (;;) {
    DWORD  count = 0;
    while (!count)
      ReadConsoleInput(inBuf, &e, 1, &count);

    if ((e.EventType == KEY_EVENT) && e.Event.KeyEvent.bKeyDown) {
      event = e.Event.KeyEvent;
      return;
    }
  } // end forever
} // end ConWindow::readKey

//--------------------------------------------------------------------
// Curses-compatible readKey function:

int ConWindow::readKey()
{
  KEY_EVENT_RECORD  e;

  readKey(e);
  switch (e.wVirtualKeyCode) {
   case VK_ESCAPE:  return KEY_ESCAPE;
   case VK_TAB:     return KEY_TAB;
   case VK_BACK:    return KEY_BACKSPACE;
   case VK_RETURN:  return KEY_RETURN;
   case VK_DELETE:  return KEY_DC;
   case VK_INSERT:  return KEY_IC;
   case VK_HOME:    return KEY_HOME;
   case VK_END:     return KEY_END;
   case VK_UP:      return KEY_UP;
   case VK_DOWN:    return KEY_DOWN;
   case VK_LEFT:    return KEY_LEFT;
   case VK_RIGHT:   return KEY_RIGHT;
  }

  return e.uChar.AsciiChar;
} // end ConWindow::readKey

//--------------------------------------------------------------------
// Make the cursor visible:

void ConWindow::showCursor(bool insert)
{
  CONSOLE_CURSOR_INFO  info;

  if (GetConsoleCursorInfo(scrBuf, &info)) {
    info.bVisible = TRUE;
    info.dwSize = (insert ? 10 : 100);
    SetConsoleCursorInfo(scrBuf, &info);
  }
} // end ConWindow::showCursor

//////////////////////////////////////////////////////////////////////
// Member Functions:
//--------------------------------------------------------------------
// Constructor:

ConWindow::ConWindow()
: data(NULL)
{
} // end ConWindow::ConWindow

//--------------------------------------------------------------------
// Destructor:

ConWindow::~ConWindow()
{
  delete[] data;
} // end ConWindow::~ConWindow

//--------------------------------------------------------------------
// Initialize the window:
//
// Must be called only once, before any other functions are called.
// Allocates the data structures and clears the window buffer, but
// does not display anything.
//
// Input:
//   x,y:           The position of the window in the screen buffer
//   width,height:  The size of the window
//   attrib:        The default attributes for the window

void ConWindow::init(short x, short y, short width, short height, Style style)
{
  ASSERT(data == NULL);
  attribs = colorStyle[style];
  pos.X  = x;
  pos.Y  = y;

  resize(width, height);
} // end ConWindow::init

//--------------------------------------------------------------------
// Draw a box in the window:
//
// Input:
//   x,y:           The upper left corner of the box in the window
//   width,height:  The size of the box
//   tl,tr:         The characters for the top left & top right corners
//   bl,br:         The characters for the bottom left & right corners
//   horiz:         The character for horizontal lines
//   vert:          The character for vertical lines

void ConWindow::box(short x, short y, short width, short height,
                    char tl, char tr, char bl, char br, char horiz, char vert)
{
  ASSERT((height > 1) && (width > 1));

  PCHAR_INFO  c1 = data + x + size.X * y;
  PCHAR_INFO  c  = c1;
  PCHAR_INFO  c2 = c1 + width - 1;

  c1->Char.AsciiChar = tl;
  c1->Attributes = attribs;
  c2->Char.AsciiChar = tr;
  c2->Attributes = attribs;

  while (++c < c2) {
    c->Char.AsciiChar = horiz;
    c->Attributes = attribs;
  }

  c  =  c1 + size.X * (height-1);
  for (;;) {
    c1 += size.X;
    c2 += size.X;
    if (c1 == c) break;
    c1->Char.AsciiChar = c2->Char.AsciiChar = vert;
    c1->Attributes = c2->Attributes = attribs;
  }

  c1->Char.AsciiChar = bl;
  c1->Attributes = attribs;
  c2->Char.AsciiChar = br;
  c2->Attributes = attribs;

  while (++c1 < c2) {
    c1->Char.AsciiChar = horiz;
    c1->Attributes = attribs;
  }
} // end ConWindow::box

//--------------------------------------------------------------------
// Draw a box with a single line around the window's border:

void ConWindow::border()
{
  box(0,0,size.X,size.Y, '\332','\277', '\300','\331', '\304', '\263');
} // end ConWindow::border

//--------------------------------------------------------------------
// Clear the window:

void ConWindow::clear()
{
  PCHAR_INFO  c = data;
  for (int i = size.X * size.Y; i > 0; ++c, --i) {
    c->Char.AsciiChar = ' ';
    c->Attributes = attribs;
  }
} // end ConWindow::clear

//--------------------------------------------------------------------
// Write a string using the current attributes:
//
// Input:
//   x,y:  The start of the string in the window
//   s:    The string to write

void ConWindow::put(short x, short y, const char* s)
{
  PCHAR_INFO  out = data + x + size.X * y;

  while (*s) {
    out->Char.AsciiChar = *s;
    out->Attributes = attribs;
    ++out;
    ++s;
  }
} // end ConWindow::put

///void ConWindow::put(short x, short y, const String& s)
///{
///  PCHAR_INFO  out = data + x + size.X * y;
///  StrConstItr  c = s.begin();
///
///  while (c != s.end()) {
///    out->Char.AsciiChar = *c;
///    out->Attributes = attribs;
///    ++out;
///    ++c;
///  }
///} // end ConWindow::put

//--------------------------------------------------------------------
// Change the attributes of characters in the window:
//
// Input:
//   x,y:    The position in the window to start changing attributes
//   color:  The attribute to set
//   count:  The number of characters to change

void ConWindow::putAttribs(short x, short y, Style color, short count)
{
  PCHAR_INFO  c = data + x + size.X * y;

  while (count--)
    (c++)->Attributes = colorStyle[color];
} // end ConWindow::putAttribs

//--------------------------------------------------------------------
// Write a character using the current attributes:
//
// Input:
//   x,y:    The position in the window to start writing
//   c:      The character to write
//   count:  The number of characters to write

void ConWindow::putChar(short x, short y, char c, short count)
{
  PCHAR_INFO  ci = data + x + size.X * y;

  while (count--) {
    ci->Char.AsciiChar = c;
    (ci++)->Attributes = attribs;
  }
} // end ConWindow::putAttribs

//--------------------------------------------------------------------
void ConWindow::resize(short width, short height)
{
  if ((size.X != width) || (size.Y != height)) {
    size.X = width;
    size.Y = height;

    delete [] data;
    data = new CHAR_INFO[size.X * size.Y];
  } // end if new size

  clear();
} // end ConWindow::resize

//--------------------------------------------------------------------
void ConWindow::setAttribs(Style color)
{
  attribs = colorStyle[color];
} // end ConWindow::setAttribs

//--------------------------------------------------------------------
// Position the cursor in the window:
//
// There is only one cursor, and each window does not maintain its own
// cursor position.
//
// Input:
//   x,y:    The position in the window for the cursor

void ConWindow::setCursor(short x, short y)
{
  ASSERT((x>=0)&&(x<size.X)&&(y>=0)&&(y<size.Y));
  COORD c;
  c.X = x + pos.X;
  c.Y = y + pos.Y;
  SetConsoleCursorPosition(scrBuf, c);
} // end ConWindow::setCursor

//--------------------------------------------------------------------
// Display the window on the screen:
//
// This is the only function that actually displays anything.
//
// Input:
//   margin:  Exclude this many characters around the edge (default 0)

void ConWindow::update(unsigned short margin)
{
  SMALL_RECT r;
  r.Left   = pos.X + margin;
  r.Top    = pos.Y + margin;
  r.Right  = pos.X + size.X - 1 - margin;
  r.Bottom = pos.Y + size.Y - 1 - margin;

  const COORD offset = {margin, margin};

  WriteConsoleOutput(scrBuf, data, size, offset, &r);
} // end ConWindow::update

//--------------------------------------------------------------------
// Local Variables:
// cjm-related-file: "ConWin.hpp"
//     c-file-style: "cjm"
// End:
