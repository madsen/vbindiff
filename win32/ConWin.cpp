//--------------------------------------------------------------------
// $Id: ConWin.cpp 4585 2004-10-26 00:23:00Z cjm $
//--------------------------------------------------------------------
//
//   VBinDiff
//   Copyright 1997 by Christopher J. Madsen
//
//   Support class for console mode applications
//
//--------------------------------------------------------------------

#include "StdAfx.h"

#include "ConWin.hpp"

static const COORD ZeroC = {0, 0};

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

  if (!SetConsoleActiveScreenBuffer(scrBuf)) {
    CloseHandle(scrBuf);
    return false;
  }

  GetConsoleMode(inBuf, &origInMode);
  SetConsoleMode(inBuf, 0);
  SetConsoleMode(scrBuf, 0);

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
// Make the cursor visible:

void ConWindow::showCursor()
{
  CONSOLE_CURSOR_INFO  info;

  if (GetConsoleCursorInfo(scrBuf, &info)) {
    info.bVisible = TRUE;
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

void ConWindow::init(short x, short y, short width, short height, WORD attrib)
{
  ASSERT(data == NULL);
  attribs = attrib;
  pos.X  = x;
  pos.Y  = y;
  size.X = width;
  size.Y = height;

  data = new CHAR_INFO[size.X * size.Y];

  clear();
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
// Draw a box with a single line:
//
// Input:
//   x,y:           The upper left corner of the box in the window
//   width,height:  The size of the box

void ConWindow::boxSingle(short x, short y, short width, short height)
{
  box(x,y,width,height, '\332','\277', '\300','\331', '\304', '\263');
} // end ConWindow::boxSingle

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

void ConWindow::putAttribs(short x, short y, WORD color, short count)
{
  PCHAR_INFO  c = data + x + size.X * y;

  while (count--)
    (c++)->Attributes = color;
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

void ConWindow::update()
{
  SMALL_RECT r;
  r.Left   = pos.X;
  r.Top    = pos.Y;
  r.Right  = pos.X + size.X - 1;
  r.Bottom = pos.Y + size.Y - 1;

  WriteConsoleOutput(scrBuf, data, size, ZeroC, &r);
} // end ConWindow::update
