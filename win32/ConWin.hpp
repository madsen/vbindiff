//--------------------------------------------------------------------
// $Id: ConWin.hpp 4617 2005-03-23 23:04:29Z cjm $
//--------------------------------------------------------------------
//
//   Visual Binary Diff
//   Copyright 1997-2005 by Christopher J. Madsen
//
//   Support class for console mode applications
//
//--------------------------------------------------------------------

#ifndef INCLUDED_CONWIN_HPP

#define INCLUDED_CONWIN_HPP

#define KEY_ESCAPE 0x1B
#define KEY_TAB    0x09
#define KEY_DC     0x107F       // Not used
#define KEY_DELETE 0x7F
#define KEY_RETURN 0x0D

#define KEY_DOWN        0402            /* down-arrow key */
#define KEY_UP          0403            /* up-arrow key */
#define KEY_LEFT        0404            /* left-arrow key */
#define KEY_RIGHT       0405            /* right-arrow key */
#define KEY_HOME        0406            /* home key */
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
  void update();

  void hide() {};
  void show() {};

  static void getScreenSize(int& x, int& y);
  static void hideCursor();
  static void readKey(KEY_EVENT_RECORD& event);
  static void showCursor();
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
