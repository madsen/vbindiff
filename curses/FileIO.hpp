//--------------------------------------------------------------------
//
//   Visual Binary Diff
//   Copyright 1997-2005 by Christopher J. Madsen
//
//   Support functions for Posix file I/O
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

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

typedef int      File;
typedef off_t    FPos;
typedef ssize_t  Size;

const int
  SeekEnd = SEEK_END,
  SeekPos = SEEK_SET;

const File InvalidFile = -1;

//--------------------------------------------------------------------
inline const char* ErrorMsg()
{
  return strerror(errno);
} // end ErrorMsg

//--------------------------------------------------------------------
inline File OpenFile(const char* path, bool writable=false)
{
  return open(path, (writable ? O_RDWR : O_RDONLY));
} // end OpenFile

//--------------------------------------------------------------------
inline void CloseFile(File file)
{
  close(file);
} // end CloseFile

//--------------------------------------------------------------------
bool WriteFile(File file, const void* buffer, Size count)
{
  const char* ptr = reinterpret_cast<const char*>(buffer);

  while (count > 0) {
    Size bytesWritten = write(file, ptr, count);
    if (bytesWritten < 1) {
      if (errno == EINTR)
        bytesWritten = 0;
      else
        return false;
    }

    ptr   += bytesWritten;
    count -= bytesWritten;
  } // end while more to write

  return true;
} // end WriteFile

//--------------------------------------------------------------------
inline Size ReadFile(File file, void* buffer, Size count)
{
  return read(file, buffer, count);
} // end ReadFile

//--------------------------------------------------------------------
inline FPos SeekFile(File file, FPos position, int whence=SeekPos)
{
  return lseek(file, position, whence);
} // end SeekFile

#endif // INCLUDED_FILEIO_HPP

// Local Variables:
//     c-file-style: "cjm"
// End:
