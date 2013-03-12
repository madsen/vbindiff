//--------------------------------------------------------------------
//
//   Visual Binary Diff
//   Copyright 1997-2005 by Christopher J. Madsen
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

#ifndef INCLUDED_CONWIN_HPP

#define INCLUDED_CONWIN_HPP

#define KEY_ESCAPE 0x1B
#define KEY_TAB    0x09
#define KEY_DC     0x7F
#define KEY_DELETE 0x107F       // Not used
#define KEY_IC     0513
#define KEY_RETURN 0x0D

#define KEY_DOWN        0402            /* down-arrow key */
#define KEY_UP          0403            /* up-arrow key */
#define KEY_LEFT        0404            /* left-arrow key */
#define KEY_RIGHT       0405            /* right-arrow key */
#define KEY_HOME        0406            /* home key */
#define KEY_END         0550            /* end key */
#define KEY_BACKSPACE   0407            /* backspace key */


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
 public:
  static HANDLE inBuf;
  static HANDLE scrBuf;

 protected:
  COORD  size;
  COORD  pos;
  WORD   attribs;
  PCHAR_INFO  data;

 public:
  ConWindow();
  ~ConWindow();
  void init(short x, short y, short width, short height, Style style);
  void close() {};

  void border();
  void clear();
  void move(short x, short y) { pos.X = x; pos.Y = y; };
///void put(short x, short y, const String& s);
  void put(short x, short y, const char* s);
  void putAttribs(short x, short y, Style color, short count);
  void putChar(short x, short y, char c, short count);
  int  readKey();
  void resize(short width, short height);
  void setAttribs(Style color);
  void setCursor(short x, short y);
  void update(unsigned short margin=0);

  void hide() {};
  void show() {};

  static void getScreenSize(int& x, int& y);
  static void hideCursor();
  static void readKey(KEY_EVENT_RECORD& event);
  static void showCursor(bool insert=true);
  static void shutdown();
  static bool startup();

 protected:
  void box(short x, short y, short width, short height,
           char tl, char tr, char bl, char br, char horiz, char vert);
}; // end ConWindow

#endif // INCLUDED_CONWIN_HPP

// Local Variables:
// cjm-related-file: "ConWin.cpp"
//     c-file-style: "cjm"
// End:
