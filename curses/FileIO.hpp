//--------------------------------------------------------------------
// $Id: FileIO.hpp 4620 2005-03-25 19:59:42Z cjm $
//--------------------------------------------------------------------
//
//   Visual Binary Diff
//   Copyright 1997-2005 by Christopher J. Madsen
//
//   Support functions for Posix file I/O
//
//--------------------------------------------------------------------

#ifndef INCLUDED_FILEIO_HPP

#define INCLUDED_FILEIO_HPP

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

typedef int      File;
typedef fpos_t   FPos;
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
  const char* ptr = buffer;

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
inline FPos SeekFile(File file, FPos position, DWORD whence=SeekPos)
{
  return lseek(file, position, whence);
} // end SeekFile

#endif // INCLUDED_FILEIO_HPP

// Local Variables:
//     c-file-style: "cjm"
// End:
