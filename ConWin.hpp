//--------------------------------------------------------------------
// $Id: ConWin.hpp 4592 2005-03-12 17:11:36Z cjm $
//--------------------------------------------------------------------
//
//   VBinDiff
//   Copyright 1997 by Christopher J. Madsen
//
//   Support class for console mode applications
//
//--------------------------------------------------------------------

#ifndef __CONWIN_HPP

#define __CONWIN_HPP

#include <panel.h>
#undef border

enum Style {
  cBackground = 0,
  cPromptWin,
  cPromptKey,
  cPromptBdr,
  cLocked,
  cFileName,
  cFileWin,
  cFileDiff,
  cFileEdit
};

class ConWindow
{
 public:

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
  void setAttribs(Style color);
  void setCursor(short x, short y);
  void update() {};

  void hide() { hide_panel(pan); };
  void show() { show_panel(pan); };

  static void hideCursor() { curs_set(0); };
  static void showCursor() { curs_set(1); };
  static void shutdown();
  static bool startup();

}; // end ConWindow

#endif // __CONWIN_HPP
