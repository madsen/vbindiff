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

#ifndef INCLUDED_CONWIN_HPP

#define INCLUDED_CONWIN_HPP

#include <panel.h>
#undef border                 // It interferes with my member function

#define KEY_ESCAPE 0x1B
#define KEY_TAB    0x09
#define KEY_DELETE 0x7F
#define KEY_RETURN 0x0D

enum Style {
  cBackground = 0,
  cPromptWin,
  cPromptKey,
  cPromptBdr,
  cCurrentMode,
  cFileName,
  cFileWin,
  cFileDiff,
  cFileEdit
};

class ConWindow
{
 protected:
  PANEL   *pan;
  WINDOW  *win;

 public:
  ConWindow();
  ~ConWindow();
  void init(short x, short y, short width, short height, Style style);
  void close();

  void border() { ::box(win, 0, 0); };
  void clear()  {   werase(win);    };
  void move(short x, short y) { move_panel(pan, y, x); };
///void put(short x, short y, const String& s);
  void put(short x, short y, const char* s) { mvwaddstr(win, y, x, s); };
  void putAttribs(short x, short y, Style color, short count);
  void putChar(short x, short y, char c, short count);
  int  readKey();
  void resize(short width, short height);
  void setAttribs(Style color);
  void setCursor(short x, short y);
  void update(unsigned short margin=0) {};

  void hide() { hide_panel(pan); };
  void show() { show_panel(pan); };

  static void getScreenSize(int& x, int& y) { getmaxyx(curscr, y, x); };
  static void hideCursor()                  { curs_set(0);            };
  static void showCursor(bool insert=true)  { curs_set(insert ? 1 : 2); };
  static void shutdown();
  static bool startup();
}; // end ConWindow

#endif // INCLUDED_CONWIN_HPP

//--------------------------------------------------------------------
// Local Variables:
//            mode: c++
//    c-file-style: "cjm"
// End:
