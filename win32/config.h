//--------------------------------------------------------------------
// $Id: config.h 4749 2008-06-10 05:07:14Z cjm $
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

// Stop complaining about lengthy names of template expansions:
#pragma warning(disable: 4786)

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
