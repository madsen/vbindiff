#ifndef _WINRESRC_H
#define _WINRESRC_H

#if __GNUC__ >= 3
#pragma GCC system_header
#endif

#ifndef WINVER
#define WINVER 0x0500
#endif

#ifndef _WIN32_IE
#define _WIN32_IE 0x0501
#endif

#ifndef _WIN32_WINDOWS
#define _WIN32_WINDOWS 0x0410
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif

#include <winuser.h>
#include <commctrl.h>
#include <dde.h>
#include <winnt.h>
#include <dlgs.h>
#include <winver.h>

#endif // _WINRESRC_H
