//--------------------------------------------------------------------
// $Id: ConWin.hpp 4585 2004-10-26 00:23:00Z cjm $
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

#define F_BLACK 0
#define F_RED   FOREGROUND_RED
#define F_WHITE (FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE)
#define F_YELLOW (FOREGROUND_GREEN|FOREGROUND_RED)
#define B_BLUE  BACKGROUND_BLUE
#define B_WHITE (BACKGROUND_RED|BACKGROUND_GREEN|BACKGROUND_BLUE)


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
  void init(short x, short y, short width, short height, WORD attrib);

  void boxSingle(short x, short y, short width, short height);
  void clear();
  void move(short x, short y) { pos.X = x; pos.Y = y; };
///void put(short x, short y, const String& s);
  void put(short x, short y, const char* s);
  void putAttribs(short x, short y, WORD color, short count);
  void putChar(short x, short y, char c, short count);
  void setAttribs(WORD color) { attribs = color; };
  void setCursor(short x, short y);
  void update();

  static void hideCursor();
  static void readKey(KEY_EVENT_RECORD& event);
  static void showCursor();
  static void shutdown();
  static bool startup();

 protected:
  void box(short x, short y, short width, short height,
           char tl, char tr, char bl, char br, char horiz, char vert);
}; // end ConWindow

#endif // __CONWIN_HPP
