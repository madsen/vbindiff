// winres.h

#if __GNUC__ >= 3
#pragma GCC system_header
#endif

#define VS_VERSION_INFO     1

#ifndef WINVER
#define WINVER 0x0501   // default to Windows XP
#endif

#include <winresrc.h>

// Remove possible incorrect value
#ifdef IDC_STATIC
#undef IDC_STATIC
#endif
#define IDC_STATIC      (-1)

