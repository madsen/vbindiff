//--------------------------------------------------------------------
//
//   Visual Binary Diff
//   Copyright 1997-2005 by Christopher J. Madsen
//
//   Support functions for Win32 file I/O
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

#ifndef INCLUDED_FILEIO_HPP

#define INCLUDED_FILEIO_HPP

typedef HANDLE   File;
typedef __int64  FPos;
typedef int      Size;

const DWORD
  SeekEnd = FILE_END,
  SeekPos = FILE_BEGIN;

const File InvalidFile = INVALID_HANDLE_VALUE;

#ifndef INVALID_SET_FILE_POINTER
#define INVALID_SET_FILE_POINTER ((DWORD)0xFFFFFFFF)
#endif

//--------------------------------------------------------------------
LPCTSTR ErrorMsg()
{
  static TCHAR  buf[512];
  FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | 65, NULL, GetLastError(), 0,
                buf, sizeof(buf), NULL);

  return buf;
} // end ErrorMsg

//--------------------------------------------------------------------
inline File OpenFile(const char* path, bool writable=false)
{
  return CreateFile(path, (writable ? GENERIC_READ|GENERIC_WRITE : GENERIC_READ),
                    FILE_SHARE_READ|FILE_SHARE_WRITE,
                    NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
} // end OpenFile

//--------------------------------------------------------------------
inline void CloseFile(File file)
{
  CloseHandle(file);
} // end CloseFile

//--------------------------------------------------------------------
inline bool WriteFile(File file, const void* buffer, Size count)
{
  DWORD bytesWritten;

  return (WriteFile(file, buffer, count, &bytesWritten, NULL) != 0
          && bytesWritten == count);
} // end WriteFile

//--------------------------------------------------------------------
Size ReadFile(File file, void* buffer, Size count)
{
  DWORD  bytesRead;

  if (!ReadFile(file, buffer, count, &bytesRead, NULL)) return -1;

  return bytesRead;
} // end ReadFile

//--------------------------------------------------------------------
FPos SeekFile(File file, FPos position, DWORD whence=SeekPos)
{
   LARGE_INTEGER li;

   li.QuadPart = position;

   li.LowPart = SetFilePointer(file, li.LowPart, &li.HighPart, whence);

   if ((li.LowPart == INVALID_SET_FILE_POINTER) && (GetLastError() != NO_ERROR))
     li.QuadPart = -1;

   return li.QuadPart;
} // end SeekFile

#endif // INCLUDED_FILEIO_HPP

// Local Variables:
//     c-file-style: "cjm"
// End:
