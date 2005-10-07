//--------------------------------------------------------------------
// $Id: config.h 4644 2005-10-07 21:43:10Z cjm $
//--------------------------------------------------------------------
//
//   Visual Binary Diff
//   Copyright 1997-2005 by Christopher J. Madsen
//
//   Include file for standard system include files,
//   or project specific include files that are used frequently,
//   but are changed infrequently
//
//--------------------------------------------------------------------

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <crtdbg.h>             // _ASSERT

#define ASSERT _ASSERTE

#define WIN32_CONSOLE 1

#include "version.h"

/////////////////////////////////////////////////////////////////////////////
// Local Variables:
// mode: c++
// c-file-style: "cjm"
// End:
