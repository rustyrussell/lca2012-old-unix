/* Hacked by Rusty to make it standalone, remove options and error output */
/* cat -- concatenate files and print on the standard output.
   Copyright (C) 1988, 1990-1991, 1995-2010 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* Differences from the Unix cat:
   * Always unbuffered, -u is ignored.
   * Usually much faster than other versions of cat, the difference
   is especially apparent when using the -v option.

   By tege@sics.se, Torbjorn Granlund, advised by rms, Richard Stallman.  */
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <limits.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#ifndef O_BINARY
# define O_BINARY 0
# define O_TEXT 0
#endif

#ifdef EINTR
# define IS_EINTR(x) ((x) == EINTR)
#else
# define IS_EINTR(x) 0
#endif

/* Redirection and wildcarding when done by the utility itself.
   Generally a noop, but used in particular for native VMS. */
#ifndef initialize_main
# define initialize_main(ac, av)
#endif

#if !defined DEV_BSIZE && defined BSIZE
# define DEV_BSIZE BSIZE
#endif
#if !defined DEV_BSIZE && defined BBSIZE /* SGI */
# define DEV_BSIZE BBSIZE
#endif
#ifndef DEV_BSIZE
# define DEV_BSIZE 4096
#endif
#ifndef SIZE_MAX
# define SIZE_MAX ((size_t) -1)
#endif

# define ST_BLKSIZE(statbuf) ((0 < (statbuf).st_blksize \
                               && (statbuf).st_blksize <= SIZE_MAX / 8 + 1) \
                              ? (statbuf).st_blksize : DEV_BSIZE)

#ifndef MAX
# define MAX(a, b) ((a) < (b) ? (b) : (a))
#endif

# define STREQ(a, b) (strcmp (a, b) == 0)

enum { IO_BUFSIZE = 32*1024 };
static size_t
io_blksize (struct stat sb)
{
  return MAX (IO_BUFSIZE, ST_BLKSIZE (sb));
}

/* Name of input file.  May be "-".  */
static char const *infile;

/* Descriptor on which input file is open.  */
static int input_desc;

#define PAGE_SIZE 4096
/* The input buffer.  */
static char inbuf[4096 + PAGE_SIZE - 1];

/* Compute the next line number.  */

/* Read(write) up to COUNT bytes at BUF from(to) descriptor FD, retrying if
   interrupted.  Return the actual number of bytes read(written), zero for EOF,
   or SAFE_READ_ERROR(SAFE_WRITE_ERROR) upon error.  */
size_t
safe_read (int fd, void *buf, size_t count)
{
  /* Work around a bug in Tru64 5.1.  Attempting to read more than
     INT_MAX bytes fails with errno == EINVAL.  See
     <http://lists.gnu.org/archive/html/bug-gnu-utils/2002-04/msg00010.html>.
     When decreasing COUNT, keep it block-aligned.  */
  enum { BUGGY_READ_MAXIMUM = INT_MAX & ~8191 };

  for (;;)
    {
      ssize_t result = read(fd, buf, count);

      if (0 <= result)
        return result;
      else if (IS_EINTR (errno))
        continue;
      else if (errno == EINVAL && BUGGY_READ_MAXIMUM < count)
        count = BUGGY_READ_MAXIMUM;
      else
        return result;
    }
}

size_t
safe_write (int fd, const void *buf, size_t count)
{
  /* Work around a bug in Tru64 5.1.  Attempting to read more than
     INT_MAX bytes fails with errno == EINVAL.  See
     <http://lists.gnu.org/archive/html/bug-gnu-utils/2002-04/msg00010.html>.
     When decreasing COUNT, keep it block-aligned.  */
  enum { BUGGY_READ_MAXIMUM = INT_MAX & ~8191 };

  for (;;)
    {
      ssize_t result = write(fd, buf, count);

      if (0 <= result)
        return result;
      else if (IS_EINTR (errno))
        continue;
      else if (errno == EINVAL && BUGGY_READ_MAXIMUM < count)
        count = BUGGY_READ_MAXIMUM;
      else
        return result;
    }
}

# define ZERO_BYTE_TRANSFER_ERRNO ENOSPC

size_t
full_write (int fd, const void *buf, size_t count)
{
  size_t total = 0;
  const char *ptr = (const char *) buf;

  while (count > 0)
    {
      size_t n_rw = safe_write (fd, ptr, count);
      if (n_rw == (size_t) -1)
        break;
      if (n_rw == 0)
        {
          errno = ZERO_BYTE_TRANSFER_ERRNO;
          break;
        }
      total += n_rw;
      ptr += n_rw;
      count -= n_rw;
    }

  return total;
}

#define SAFE_READ_ERROR ((size_t) -1)

static void *
ptr_align (void const *ptr, size_t alignment)
{
  char const *p0 = ptr;
  char const *p1 = p0 + alignment - 1;
  return (void *) (p1 - (size_t) p1 % alignment);
}

/* Plain cat.  Copies the file behind `input_desc' to STDOUT_FILENO.
   Return true if successful.  */

static bool
simple_cat (
     /* Pointer to the buffer, used by reads and writes.  */
     char *buf,

     /* Number of characters preferably read or written by each read and write
        call.  */
     size_t bufsize)
{
  /* Actual number of characters read, and therefore written.  */
  size_t n_read;

  /* Loop until the end of the file.  */

  for (;;)
    {
      /* Read a block of input.  */

      n_read = safe_read (input_desc, buf, bufsize);
      if (n_read == SAFE_READ_ERROR)
        {
          return false;
        }

      /* End of this file?  */

      if (n_read == 0)
        return true;

      /* Write this block out.  */

      {
        /* The following is ok, since we know that 0 < n_read.  */
        size_t n = n_read;
        if (full_write (STDOUT_FILENO, buf, n) != n)
          return false;
      }
    }
}

int
main (int argc, char **argv)
{
  /* Optimal size of i/o operations of output.  */
  size_t outsize;

  /* Optimal size of i/o operations of input.  */
  size_t insize;

  bool ok = true;

  /* Index in argv to processed argument.  */
  int argind;

  /* Nonzero if we have ever read standard input.  */
  bool have_read_stdin = false;

  struct stat stat_buf;

  /* Variables that are set according to the specified options.  */
  int file_open_mode = O_RDONLY;

  initialize_main (&argc, &argv);

  /* Get device, i-node number, and optimal blocksize of output.  */

  if (fstat (STDOUT_FILENO, &stat_buf) < 0)
    abort();

  outsize = io_blksize (stat_buf);
  /* Input file can be output file for non-regular files.
     fstat on pipes returns S_IFSOCK on some systems, S_IFIFO
     on others, so the checking should not be done for those types,
     and to allow things like cat < /dev/tty > /dev/tty, checking
     is not done for device files either.  */

  file_open_mode |= O_BINARY;
  /* Check if any of the input files are the same as the output file.  */

  /* Main loop.  */

  infile = "-";
  argind = 1;

  do
    {
      if (argind < argc)
        infile = argv[argind];

      if (STREQ (infile, "-"))
        {
          have_read_stdin = true;
          input_desc = STDIN_FILENO;
        }
      else
        {
          input_desc = open (infile, file_open_mode);
          if (input_desc < 0)
            {
              ok = false;
              continue;
            }
        }

      if (fstat (input_desc, &stat_buf) < 0)
        {
          ok = false;
          goto contin;
        }
      insize = io_blksize (stat_buf);

          insize = MAX (insize, outsize);
	  if (insize > 4096)
	    insize = 4096;
          ok &= simple_cat (ptr_align (inbuf, PAGE_SIZE), insize);

    contin:
      if (!STREQ (infile, "-") && close (input_desc) < 0)
        {
          ok = false;
        }
    }
  while (++argind < argc);

  if (have_read_stdin && close (STDIN_FILENO) < 0)
    return false;

  exit (ok ? EXIT_SUCCESS : EXIT_FAILURE);
}
