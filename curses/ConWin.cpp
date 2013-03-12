//--------------------------------------------------------------------
//
//   Visual Binary Diff
//   Copyright 1997-2005 by Christopher J. Madsen
//
//   Support class for curses applications
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

#include <stdlib.h>

#include "ConWin.hpp"

void exitMsg(int status, const char* message); // From vbindiff.cpp

enum ColorPair {
  pairWhiteBlue= 1,
  pairWhiteBlack,
  pairRedBlue,
  pairYellowBlue
};

static const ColorPair colorStyle[] = {
  pairWhiteBlue,   // cBackground
  pairWhiteBlue,   // cPromptWin
  pairWhiteBlue,   // cPromptKey
  pairWhiteBlue,   // cPromptBdr
  pairWhiteBlack,  // cCurrentMode
  pairWhiteBlack,  // cFileName
  pairWhiteBlue,   // cFileWin
  pairRedBlue,     // cFileDiff
  pairYellowBlue   // cFileEdit
};

static const attr_t attribStyle[] = {
              COLOR_PAIR(colorStyle[ cBackground ]),
              COLOR_PAIR(colorStyle[ cPromptWin  ]),
  A_BOLD    | COLOR_PAIR(colorStyle[ cPromptKey  ]),
  A_BOLD    | COLOR_PAIR(colorStyle[ cPromptBdr  ]),
  A_REVERSE | COLOR_PAIR(colorStyle[ cCurrentMode]),
  A_REVERSE | COLOR_PAIR(colorStyle[ cFileName   ]),
              COLOR_PAIR(colorStyle[ cFileWin    ]),
  A_BOLD    | COLOR_PAIR(colorStyle[ cFileDiff   ]),
  A_BOLD    | COLOR_PAIR(colorStyle[ cFileEdit   ])
};

//====================================================================
// Class ConWindow:
//--------------------------------------------------------------------
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
  if (!initscr()) return false; // initialize the curses library
  atexit(ConWindow::shutdown);  // just in case

  keypad(stdscr, true);         // enable keyboard mapping
  nonl();           // tell curses not to do NL->CR/NL on output
  cbreak();         // take input chars one at a time, no wait for \n
  noecho();         // do not echo input

  if (has_colors()) {
    start_color();

    init_pair(pairWhiteBlue,  COLOR_WHITE,  COLOR_BLUE);
    init_pair(pairWhiteBlack, COLOR_WHITE,  COLOR_BLACK);
    init_pair(pairRedBlue,    COLOR_RED,    COLOR_BLUE);
    init_pair(pairYellowBlue, COLOR_YELLOW, COLOR_BLUE);
  } // end if terminal has color

  return true;
} // end ConWindow::startup

//--------------------------------------------------------------------
// Shut down the window system:
//
// Deallocate the screen buffer and restore the original input mode.

void ConWindow::shutdown()
{
  if (!isendwin()) {
    showCursor();
    endwin();
  }
} // end ConWindow::shutdown

//////////////////////////////////////////////////////////////////////
// Member Functions:
//--------------------------------------------------------------------
// Constructor:

ConWindow::ConWindow()
: pan(NULL),
  win(NULL)
{
} // end ConWindow::ConWindow

//--------------------------------------------------------------------
// Destructor:

ConWindow::~ConWindow()
{
  close();
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

void ConWindow::init(short x, short y, short width, short height, Style attrib)
{
  if ((win = newwin(height, width, y, x)) == 0)
    exitMsg(99, "Internal error: Failed to create window");

  if ((pan = new_panel(win)) == 0)
    exitMsg(99, "Internal error: Failed to create panel");

  wbkgdset(win, attribStyle[attrib] | ' ');

  keypad(win, TRUE);            // enable keyboard mapping

  clear();
} // end ConWindow::init

//--------------------------------------------------------------------
void ConWindow::close()
{
  if (pan) {
    del_panel(pan);
    pan = NULL;
  }

  if (win) {
    delwin(win);
    win = NULL;
  }
} // end ConWindow::close

//--------------------------------------------------------------------
// Write a string using the current attributes:
//
// Input:
//   x,y:  The start of the string in the window
//   s:    The string to write

//void ConWindow::put(short x, short y, const char* s)

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
  mvwchgat(win, y, x, count, attribStyle[color], colorStyle[color], NULL);
  touchwin(win);
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
  wmove(win, y, x);

  while (count--) {
    waddch(win, c);
  }
} // end ConWindow::putAttribs

//--------------------------------------------------------------------
// Read the next key down event:
//
// Output:
//   event:  Contains a key down event

int ConWindow::readKey()
{
  top_panel(pan);
  update_panels();
  doupdate();

  return wgetch(win);
} // end ConWindow::readKey

//--------------------------------------------------------------------
void ConWindow::resize(short width, short height)
{
  if (wresize(win, height, width) != OK)
    exitMsg(99, "Internal error: Failed to resize window");

  replace_panel(pan, win);

  clear();
} // end ConWindow::resize

//--------------------------------------------------------------------
void ConWindow::setAttribs(Style color)
{
  wattrset(win, attribStyle[color]);
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
//  ASSERT((x>=0)&&(x<size.X)&&(y>=0)&&(y<size.Y));

  wmove(win, y, x);
} // end ConWindow::setCursor

//--------------------------------------------------------------------
// Local Variables:
//     c-file-style: "cjm"
// End:
