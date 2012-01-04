/* Hacked by Rusty to make it standalone, remove options. */
/* `dir', `vdir' and `ls' directory listing programs for GNU.
   Copyright (C) 1985, 1988, 1990-1991, 1995-2010 Free Software Foundation,
   Inc.

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

/* If ls_mode is LS_MULTI_COL,
   the multi-column format is the default regardless
   of the type of output device.
   This is for the `dir' program.

   If ls_mode is LS_LONG_FORMAT,
   the long format is the default regardless of the
   type of output device.
   This is for the `vdir' program.

   If ls_mode is LS_LS,
   the output format depends on whether the output
   device is a terminal.
   This is for the `ls' program.  */

/* Written by Richard Stallman and David MacKenzie.  */

/* Color support by Peter Anvin <Peter.Anvin@linux.org> and Dennis
   Flaherty <dennisf@denix.elk.miles.com> based on original patches by
   Greg Lee <lee@uhunix.uhcc.hawaii.edu>.  */

#include <sys/types.h>
#include <stdio.h>
#include <assert.h>
#include <setjmp.h>
#include <grp.h>
#include <pwd.h>
#include <getopt.h>
#include <signal.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <time.h>

#ifndef MAX
# define MAX(a, b) ((a) < (b) ? (b) : (a))
#endif
#ifndef MIN
# define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

# define ST_NBLOCKS(statbuf) ((statbuf).st_blocks)

/* Bound on length of the string representing an integer type or expression T.
   Subtract 1 for the sign bit if t is signed; log10 (2.0) < 146/485;
   add 1 for integer division truncation; add 1 more for a minus sign
   if needed.  */
#define INT_STRLEN_BOUND(t) \
  ((sizeof (t) * CHAR_BIT - 1) * 146 / 485 + 2)

#define LONGEST_HUMAN_READABLE INT_STRLEN_BOUND(uintmax_t)

/* Bound on buffer size needed to represent an integer type or expression T,
   including the terminating null.  */
# define INT_BUFSIZE_BOUND(t) (INT_STRLEN_BOUND (t) + 1)
# define SAME_INODE(Stat_buf_1, Stat_buf_2) \
   ((Stat_buf_1).st_ino == (Stat_buf_2).st_ino \
    && (Stat_buf_1).st_dev == (Stat_buf_2).st_dev)

# define TYPE_SIGNED_MAGNITUDE(t) ((t) ~ (t) 0 < (t) -1)

/* True if the arithmetic type T is signed.  */
# define TYPE_SIGNED(t) (! ((t) 0 < (t) -1))

/* The maximum and minimum values for the integer type T.  These
   macros have undefined behavior if T is signed and has padding bits.
   If this is a problem for you, please let us know how to fix it for
   your host.  */
# define TYPE_MINIMUM(t) \
   ((t) (! TYPE_SIGNED (t) \
         ? (t) 0 \
         : TYPE_SIGNED_MAGNITUDE (t) \
         ? ~ (t) 0 \
         : ~ (t) 0 << (sizeof (t) * CHAR_BIT - 1)))

enum
{
  NOT_AN_INODE_NUMBER = 0
};

# define xalloc_oversized(n, s) \
    ((size_t) (sizeof (ptrdiff_t) <= sizeof (size_t) ? -1 : -2) / (s) < (n))

static void *
xmalloc (size_t n)
{
  void *p = malloc (n);
  if (!p && n != 0)
    abort();
  return p;
}

static void *
xnmalloc (size_t n, size_t s)
{
  if (xalloc_oversized (n, s))
    abort();
  return xmalloc (n * s);
}

static void *
xmemdup (void const *p, size_t s)
{
  return memcpy (xmalloc (s), p, s);
}

/* Clone STRING.  */

static char *
xstrdup (char const *string)
{
  return xmemdup (string, strlen (string) + 1);
}

static void *
xrealloc (void *p, size_t n)
{
  p = realloc (p, n);
  if (!p && n != 0)
    abort();
  return p;
}

static void *
xnrealloc (void *p, size_t n, size_t s)
{
  if (xalloc_oversized (n, s))
    abort();
  return xrealloc (p, n * s);
}

/* Bound on length of the string representing an integer type or expression T.
   Subtract 1 for the sign bit if t is signed; log10 (2.0) < 146/485;
   add 1 for integer division truncation; add 1 more for a minus sign
   if needed.  */
#define INT_STRLEN_BOUND(t) \
  ((sizeof (t) * CHAR_BIT - 1) * 146 / 485 + 2)

static char *
umaxtostr (uintmax_t i, char *buf)
{
  char *p = buf + INT_STRLEN_BOUND (uintmax_t);
  *p = 0;

    {
      do
        *--p = '0' + i % 10;
      while ((i /= 10) != 0);
    }

  return p;
}

typedef int (*comparison_function) (void const *, void const *);

static void mpsort_with_tmp (void const **, size_t,
                             void const **, comparison_function);

/* Sort a vector BASE containing N pointers, placing the sorted array
   into TMP.  Compare pointers with CMP.  N must be at least 2.  */

static void
mpsort_into_tmp (void const **base, size_t n,
                 void const **tmp,
                 comparison_function cmp)
{
  size_t n1 = n / 2;
  size_t n2 = n - n1;
  size_t a = 0;
  size_t alim = n1;
  size_t b = n1;
  size_t blim = n;
  void const *ba;
  void const *bb;

  mpsort_with_tmp (base + n1, n2, tmp, cmp);
  mpsort_with_tmp (base, n1, tmp, cmp);

  ba = base[a];
  bb = base[b];

  for (;;)
    if (cmp (ba, bb) <= 0)
      {
        *tmp++ = ba;
        a++;
        if (a == alim)
          {
            a = b;
            alim = blim;
            break;
          }
        ba = base[a];
      }
    else
      {
        *tmp++ = bb;
        b++;
        if (b == blim)
          break;
        bb = base[b];
      }

  memcpy (tmp, base + a, (alim - a) * sizeof *base);
}

/* Sort a vector BASE containing N pointers, in place.  Use TMP
   (containing N / 2 pointers) for temporary storage.  Compare
   pointers with CMP.  */

static void
mpsort_with_tmp (void const **base, size_t n,
                 void const **tmp,
                 comparison_function cmp)
{
  if (n <= 2)
    {
      if (n == 2)
        {
          void const *p0 = base[0];
          void const *p1 = base[1];
          if (! (cmp (p0, p1) <= 0))
            {
              base[0] = p1;
              base[1] = p0;
            }
        }
    }
  else
    {
      size_t n1 = n / 2;
      size_t n2 = n - n1;
      size_t i;
      size_t t = 0;
      size_t tlim = n1;
      size_t b = n1;
      size_t blim = n;
      void const *bb;
      void const *tt;

      mpsort_with_tmp (base + n1, n2, tmp, cmp);

      if (n1 < 2)
        tmp[0] = base[0];
      else
        mpsort_into_tmp (base, n1, tmp, cmp);

      tt = tmp[t];
      bb = base[b];

      for (i = 0; ; )
        if (cmp (tt, bb) <= 0)
          {
            base[i++] = tt;
            t++;
            if (t == tlim)
              break;
            tt = tmp[t];
          }
        else
          {
            base[i++] = bb;
            b++;
            if (b == blim)
              {
                memcpy (base + i, tmp + t, (tlim - t) * sizeof *base);
                break;
              }
            bb = base[b];
          }
    }
}

/* Sort a vector BASE containing N pointers, in place.  BASE must
   contain enough storage to hold N + N / 2 vectors; the trailing
   vectors are used for temporaries.  Compare pointers with CMP.  */

static void
mpsort (void const **base, size_t n, comparison_function cmp)
{
  mpsort_with_tmp (base, n, base + n, cmp);
}

#define AUTHORS \
  proper_name ("Richard M. Stallman"), \
  proper_name ("David MacKenzie")

/* Unix-based readdir implementations have historically returned a dirent.d_ino
   value that is sometimes not equal to the stat-obtained st_ino value for
   that same entry.  This error occurs for a readdir entry that refers
   to a mount point.  readdir's error is to return the inode number of
   the underlying directory -- one that typically cannot be stat'ed, as
   long as a file system is mounted on that directory.  RELIABLE_D_INO
   encapsulates whether we can use the more efficient approach of relying
   on readdir-supplied d_ino values, or whether we must incur the cost of
   calling stat or lstat to obtain each guaranteed-valid inode number.  */

#ifndef READDIR_LIES_ABOUT_MOUNTPOINT_D_INO
# define READDIR_LIES_ABOUT_MOUNTPOINT_D_INO 1
#endif

#if READDIR_LIES_ABOUT_MOUNTPOINT_D_INO
# define RELIABLE_D_INO(dp) NOT_AN_INODE_NUMBER
#else
# define RELIABLE_D_INO(dp) D_INO (dp)
#endif

enum filetype
  {
    unknown,
    chardev,
    directory,
    blockdev,
    normal,
    arg_directory
  };
static char const filetype_letter[] = "?cdb-d";

struct fileinfo
  {
    /* The file name.  */
    char *name;

    struct stat stat;

    enum filetype filetype;

    bool stat_ok;
  };

static size_t quote_name (FILE *out, const char *name,
                          size_t *width);
static int decode_switches (int argc, char **argv);
static bool file_ignored (char const *name);
static uintmax_t gobble_file (char const *name, enum filetype type,
			      ino_t inode, bool command_line_arg,
			      char const *dirname);
static void attach (char *dest, const char *dirname, const char *name);
static void clear_files (void);
static void extract_dirs_from_files (void);
static void print_current_files (void);
static void print_dir (char const *name);
static size_t print_file_name_and_frills (const struct fileinfo *f);
static int format_user_width (uid_t u);
static int format_group_width (gid_t g);
static void print_long_format (const struct fileinfo *f);
static size_t print_name_with_quoting (const struct fileinfo *f);
static void queue_directory (char const *name);
static void sort_files (void);

/* The table of files in the current directory:

   `cwd_file' points to a vector of `struct fileinfo', one per file.
   `cwd_n_alloc' is the number of elements space has been allocated for.
   `cwd_n_used' is the number actually in use.  */

/* Address of block containing the files that are described.  */
static struct fileinfo *cwd_file;

/* Length of block that `cwd_file' points to, measured in files.  */
static size_t cwd_n_alloc;

/* Index of first unused slot in `cwd_file'.  */
static size_t cwd_n_used;

/* Vector of pointers to files, in proper sorted order, and the number
   of entries allocated for it.  */
static void **sorted_file;
static size_t sorted_file_alloc;


/* Record of one pending directory waiting to be listed.  */

struct pending
  {
    char *name;
    bool command_line_arg;
    struct pending *next;
  };

static struct pending *pending_dirs;

/* Current time in seconds and nanoseconds since 1970, updated as
   needed when deciding whether a file is recent.  */

static struct timespec current_time;

/* The number of columns to use for columns containing inode numbers,
   block sizes, link counts, owners, groups, authors, major device
   numbers, minor device numbers, and file sizes, respectively.  */

static int inode_number_width;
static int block_size_width;
static int nlink_width;
static int owner_width;
static int group_width;
static int major_device_number_width;
static int minor_device_number_width;
static int file_size_width;

/* Option flags */

/* long_format for lots of info, one per line.
   one_per_line for just names, one per line.
   many_per_line for just names, many per line, sorted vertically.
   horizontal for just names, many per line, sorted horizontally.
   with_commas for just names, many per line, separated by commas.

   -l (and other options that imply -l), -1, -C, -x and -m control
   this parameter.  */

enum format
  {
    long_format,		/* -l and other options that imply -l */
    one_per_line,		/* -1 */
  };

static enum format format;

/* Type of time to print or sort by.  Controlled by -c and -u.
   The values of each item of this enum are important since they are
   used as indices in the sort functions array (see sort_files()).  */

enum time_type
  {
    time_mtime,			/* default */
    time_atime,			/* -u */
    time_numtypes		/* the number of elements of this enum */
  };

static enum time_type time_type;

/* The file characteristic to sort by.  Controlled by -t, -S, -U, -X, -v.
   The values of each item of this enum are important since they are
   used as indices in the sort functions array (see sort_files()).  */

enum sort_type
  {
    sort_none = -1,		/* -U */
    sort_name,			/* default */
    sort_time,			/* -t */
  };

static enum sort_type sort_type;

/* Direction of sort.
   false means highest first if numeric,
   lowest first if alphabetic;
   these are the defaults.
   true means the opposite order in each case.  -r  */

static bool sort_reverse;

/* True means to display owner information.  -g turns this off.  */

static bool print_owner = true;

/* True means to display group information.  -G and -o turn this off.  */

static bool print_group = true;

/* True means mention the size in blocks of each file.  -s  */

static bool print_block_size;

/* True means mention the inode number of each file.  -i  */
static bool print_inode;

/* True means when an argument is a directory name, display info
   on it itself.  -d  */

static bool immediate_dirs;

/* Which files to ignore.  */

static enum
{
  /* Ignore files whose names start with `.'.  */
  IGNORE_DEFAULT,

  /* Ignore nothing.  */
  IGNORE_MINIMAL
} ignore_mode;

/* If true, the file listing format requires that stat be called on
   each file.  */

static bool format_needs_stat;

/* An arbitrary limit on the number of bytes in a printed time stamp.
   This is set to a relatively small value to avoid the need to worry
   about denial-of-service attacks on servers that run "ls" on behalf
   of remote clients.  1000 bytes should be enough for any practical
   time stamp format.  */

enum { TIME_STAMP_LEN_MAXIMUM = MAX (1000, INT_STRLEN_BOUND (time_t)) };

/* strftime formats for non-recent and recent files, respectively, in
   -l output.  */

static char const *long_time_format[2] =
  {
    /* strftime format for non-recent files (older than 6 months), in
       -l output.  This should contain the year, month and day (at
       least), in an order that is understood by people in your
       locale's territory.  Please try to keep the number of used
       screen columns small, because many people work in windows with
       only 80 columns.  But make this as wide as the other string
       below, for recent files.  */
    /* TRANSLATORS: ls output needs to be aligned for ease of reading,
       so be wary of using variable width fields from the locale.
       Note %b is handled specially by ls and aligned correctly.
       Note also that specifying a width as in %5b is erroneous as strftime
       will count bytes rather than characters in multibyte locales.  */
    "%b %e  %Y",
    /* strftime format for recent files (younger than 6 months), in -l
       output.  This should contain the month, day and time (at
       least), in an order that is understood by people in your
       locale's territory.  Please try to keep the number of used
       screen columns small, because many people work in windows with
       only 80 columns.  But make this as wide as the other string
       above, for non-recent files.  */
    /* TRANSLATORS: ls output needs to be aligned for ease of reading,
       so be wary of using variable width fields from the locale.
       Note %b is handled specially by ls and aligned correctly.
       Note also that specifying a width as in %5b is erroneous as strftime
       will count bytes rather than characters in multibyte locales.  */
    "%b %e %H:%M"
  };

/* Desired exit status.  */

static int exit_status;

/* Exit statuses.  */
enum
  {
    /* "ls" had a minor problem.  E.g., while processing a directory,
       ls obtained the name of an entry via readdir, yet was later
       unable to stat that name.  This happens when listing a directory
       in which entries are actively being removed or renamed.  */
    LS_MINOR_PROBLEM = 1,

    /* "ls" had more serious trouble (e.g., memory exhausted, invalid
       option or failure to stat a command line argument.  */
    LS_FAILURE = 2
  };

struct dev_ino
{
  ino_t st_ino;
  dev_t st_dev;
};
static void
free_pending_ent (struct pending *p)
{
  free (p->name);
  free (p);
}

int
main (int argc, char **argv)
{
  int i;
  struct pending *thispend;
  int n_files;

  exit_status = EXIT_SUCCESS;
  pending_dirs = NULL;

  current_time.tv_sec = TYPE_MINIMUM (time_t);
  current_time.tv_nsec = -1;

  i = decode_switches (argc, argv);

  format_needs_stat = sort_type == sort_time
    || format == long_format
    || print_block_size;

  cwd_n_alloc = 100;
  cwd_file = xnmalloc (cwd_n_alloc, sizeof *cwd_file);
  cwd_n_used = 0;

  clear_files ();

  n_files = argc - i;

  if (n_files <= 0)
    {
      if (immediate_dirs)
        gobble_file (".", directory, NOT_AN_INODE_NUMBER, true, "");
      else
        queue_directory (".");
    }
  else
    do
      gobble_file (argv[i++], unknown, NOT_AN_INODE_NUMBER, true, "");
    while (i < argc);

  if (cwd_n_used)
    {
      sort_files ();
      if (!immediate_dirs)
        extract_dirs_from_files ();
      /* `cwd_n_used' might be zero now.  */
    }

  /* In the following if/else blocks, it is sufficient to test `pending_dirs'
     (and not pending_dirs->name) because there may be no markers in the queue
     at this point.  A marker may be enqueued when extract_dirs_from_files is
     called with a non-empty string or via print_dir.  */
  if (cwd_n_used)
    {
      print_current_files ();
      if (pending_dirs)
        putchar ('\n');
    }

  while (pending_dirs)
    {
      thispend = pending_dirs;
      pending_dirs = pending_dirs->next;

      print_dir (thispend->name);

      free_pending_ent (thispend);
    }

  exit (exit_status);
}

/* Set all the option flags according to the switches specified.
   Return the index of the first non-option argument.  */

static int
decode_switches (int argc, char **argv)
{
  /* Record whether there is an option specifying sort type.  */
  bool sort_type_specified = false;

  format = one_per_line;
  time_type = time_mtime;
  sort_type = sort_name;
  sort_reverse = false;
  print_block_size = false;
  print_inode = false;
  immediate_dirs = false;
  ignore_mode = IGNORE_DEFAULT;

  for (;;)
    {
      int c = getopt (argc, argv, "adfgilrstu");
      if (c == -1)
        break;

      switch (c)
        {
        case 'a':
          ignore_mode = IGNORE_MINIMAL;
          break;

        case 'd':
          immediate_dirs = true;
          break;

        case 'f':
          /* Same as enabling -a -U and disabling -l -s.  */
          ignore_mode = IGNORE_MINIMAL;
          sort_type = sort_none;
          sort_type_specified = true;
          /* disable -l */
          if (format == long_format)
            format = one_per_line;
          print_block_size = false;	/* disable -s */
          break;

        case 'g':
          format = long_format;
          print_owner = false;
          break;

        case 'i':
          print_inode = true;
          break;

        case 'l':
          format = long_format;
          break;

        case 'r':
          sort_reverse = true;
          break;

        case 's':
          print_block_size = true;
          break;

        case 't':
          sort_type = sort_time;
          sort_type_specified = true;
          break;

        case 'u':
          time_type = time_atime;
          break;

        default:
          break;
        }
    }

  /* If -c or -u is specified and not -l (or any other option that implies -l),
     and no sort-type was specified, then sort by the ctime (-c) or atime (-u).
     The behavior of ls when using either -c or -u but with neither -l nor -t
     appears to be unspecified by POSIX.  So, with GNU ls, `-u' alone means
     sort by atime (this is the one that's not specified by the POSIX spec),
     -lu means show atime and sort by name, -lut means show atime and sort
     by atime.  */

  if ((time_type == time_atime)
      && !sort_type_specified && format != long_format)
    {
      sort_type = sort_time;
    }

  return optind;
}
/* Set the exit status to report a failure.  If SERIOUS, it is a
   serious failure; otherwise, it is merely a minor problem.  */

static void
set_exit_status (bool serious)
{
  if (serious)
    exit_status = LS_FAILURE;
  else if (exit_status == EXIT_SUCCESS)
    exit_status = LS_MINOR_PROBLEM;
}

/* Assuming a failure is serious if SERIOUS, use the printf-style
   MESSAGE to report the failure to access a file named FILE.  Assume
   errno is set appropriately for the failure.  */

static void
file_failure (bool serious, char const *message, char const *file)
{
  fprintf(stderr, message, file);
  set_exit_status (serious);
}

/* Request that the directory named NAME have its contents listed later.
   If REALNAME is nonzero, it will be used instead of NAME when the
   directory name is printed.  This allows symbolic links to directories
   to be treated as regular directories but still be listed under their
   real names.  NAME == NULL is used to insert a marker entry for the
   directory named in REALNAME.
   If NAME is non-NULL, we use its dev/ino information to save
   a call to stat -- when doing a recursive (-R) traversal.
   COMMAND_LINE_ARG means this directory was mentioned on the command line.  */

static void
queue_directory (char const *name)
{
  struct pending *new = xmalloc (sizeof *new);
  new->name = name ? xstrdup (name) : NULL;
  new->next = pending_dirs;
  pending_dirs = new;
}

/* Read directory NAME, and list the files in it.
   COMMAND_LINE_ARG means this directory was mentioned on the command line.  */

static void
print_dir (char const *name)
{
  DIR *dirp;
  struct dirent *next;
  uintmax_t total_blocks = 0;

  errno = 0;
  dirp = opendir (name);
  if (!dirp)
    {
      file_failure (true, "cannot open directory %s", name);
      return;
    }

  /* Read the directory entries, and insert the subfiles into the `cwd_file'
     table.  */

  clear_files ();

  while (1)
    {
      /* Set errno to zero so we can distinguish between a readdir failure
         and when readdir simply finds that there are no more entries.  */
      errno = 0;
      next = readdir (dirp);
      if (next)
        {
          if (! file_ignored (next->d_name))
            {
              enum filetype type = unknown;

#if HAVE_STRUCT_DIRENT_D_TYPE
              switch (next->d_type)
                {
                case DT_BLK:  type = blockdev;		break;
                case DT_CHR:  type = chardev;		break;
                case DT_DIR:  type = directory;		break;
                case DT_FIFO: type = fifo;		break;
                case DT_LNK:  type = symbolic_link;	break;
                case DT_REG:  type = normal;		break;
                case DT_SOCK: type = sock;		break;
# ifdef DT_WHT
                case DT_WHT:  type = whiteout;		break;
# endif
                }
#endif
              total_blocks += gobble_file (next->d_name, type,
                                           RELIABLE_D_INO (next),
                                           false, name);

              /* In this narrow case, print out each name right away, so
                 ls uses constant memory while processing the entries of
                 this directory.  Useful when there are many (millions)
                 of entries in a directory.  */
              if (format == one_per_line && sort_type == sort_none
                      && !print_block_size)
                {
                  /* We must call sort_files in spite of
                     "sort_type == sort_none" for its initialization
                     of the sorted_file vector.  */
                  sort_files ();
                  print_current_files ();
                  clear_files ();
                }
            }
        }
      else if (errno != 0)
        {
          file_failure (true, "reading directory %s", name);
          if (errno != EOVERFLOW)
            break;
        }
      else
        break;
    }

  if (closedir (dirp) != 0)
    {
      file_failure (true, "closing directory %s", name);
      /* Don't return; print whatever we got.  */
    }

  /* Sort the directory contents.  */
  sort_files ();

  /* If any member files are subdirectories, perhaps they should have their
     contents listed rather than being mentioned here as files.  */

  if (format == long_format || print_block_size)
    {
      fputs("total", stdout);
      putchar(' ');
      printf("%llu", (unsigned long long)total_blocks);
      putchar('\n');
    }

  if (cwd_n_used)
    print_current_files ();
}

/* Return true if FILE should be ignored.  */

static bool
file_ignored (char const *name)
{
  return (ignore_mode == IGNORE_DEFAULT && name[0] == '.');
}


/* Enter and remove entries in the table `cwd_file'.  */

/* Empty the table of files.  */

static void
clear_files (void)
{
  size_t i;

  for (i = 0; i < cwd_n_used; i++)
    {
      struct fileinfo *f = sorted_file[i];
      free (f->name);
    }

  cwd_n_used = 0;
  inode_number_width = 0;
  block_size_width = 0;
  nlink_width = 0;
  owner_width = 0;
  group_width = 0;
  major_device_number_width = 0;
  minor_device_number_width = 0;
  file_size_width = 0;
}

/* Add a file to the current table of files.
   Verify that the file exists, and print an error message if it does not.
   Return the number of blocks that the file occupies.  */

static uintmax_t
gobble_file (char const *name, enum filetype type, ino_t inode,
             bool command_line_arg, char const *dirname)
{
  uintmax_t blocks = 0;
  struct fileinfo *f;

  /* An inode value prior to gobble_file necessarily came from readdir,
     which is not used for command line arguments.  */
  assert (! command_line_arg || inode == NOT_AN_INODE_NUMBER);

  if (cwd_n_used == cwd_n_alloc)
    {
      cwd_file = xnrealloc (cwd_file, cwd_n_alloc, 2 * sizeof *cwd_file);
      cwd_n_alloc *= 2;
    }

  f = &cwd_file[cwd_n_used];
  memset (f, '\0', sizeof *f);
  f->stat.st_ino = inode;
  f->filetype = type;

  if (command_line_arg
      || format_needs_stat
      /* Command line dereferences are already taken care of by the above
         assertion that the inode number is not yet known.  */
      || (print_inode && inode == NOT_AN_INODE_NUMBER))
    {
      /* Absolute name of this file.  */
      char *absolute_name;
      int err;

      if (name[0] == '/' || dirname[0] == 0)
        absolute_name = (char *) name;
      else
        {
          absolute_name = alloca (strlen (name) + strlen (dirname) + 2);
          attach (absolute_name, dirname, name);
        }

      err = lstat (absolute_name, &f->stat);

      if (err != 0)
        {
          /* Failure to stat a command line argument leads to
             an exit status of 2.  For other files, stat failure
             provokes an exit status of 1.  */
          file_failure (command_line_arg,
                        "cannot access %s", absolute_name);
          if (command_line_arg)
            return 0;

          f->name = xstrdup (name);
          cwd_n_used++;

          return 0;
        }

      f->stat_ok = true;

      if (S_ISDIR (f->stat.st_mode))
        {
          if (command_line_arg && !immediate_dirs)
            f->filetype = arg_directory;
          else
            f->filetype = directory;
        }
      else
        f->filetype = normal;

      blocks = ST_NBLOCKS (f->stat);
      if (format == long_format || print_block_size)
        {
          char b[INT_BUFSIZE_BOUND (uintmax_t)];
	  int len = strlen (umaxtostr (blocks, b));
          if (block_size_width < len)
            block_size_width = len;
        }

      if (format == long_format)
        {
          if (print_owner)
            {
              int len = format_user_width (f->stat.st_uid);
              if (owner_width < len)
                owner_width = len;
            }

          if (print_group)
            {
              int len = format_group_width (f->stat.st_gid);
              if (group_width < len)
                group_width = len;
            }
        }

      if (format == long_format)
        {
          char b[INT_BUFSIZE_BOUND (uintmax_t)];
          int b_len = strlen (umaxtostr (f->stat.st_nlink, b));
          if (nlink_width < b_len)
            nlink_width = b_len;

          if (S_ISCHR (f->stat.st_mode) || S_ISBLK (f->stat.st_mode))
            {
              char buf[INT_BUFSIZE_BOUND (uintmax_t)];
              int len = strlen (umaxtostr (major (f->stat.st_rdev), buf));
              if (major_device_number_width < len)
                major_device_number_width = len;
              len = strlen (umaxtostr (minor (f->stat.st_rdev), buf));
              if (minor_device_number_width < len)
                minor_device_number_width = len;
              len = major_device_number_width + 2 + minor_device_number_width;
              if (file_size_width < len)
                file_size_width = len;
            }
          else
            {
              char buf[LONGEST_HUMAN_READABLE + 1];
              uintmax_t size = f->stat.st_size;
              int len = strlen (umaxtostr (size, buf));
              if (file_size_width < len)
                file_size_width = len;
            }
        }
    }

  if (print_inode)
    {
      char buf[INT_BUFSIZE_BOUND (uintmax_t)];
      int len = strlen (umaxtostr (f->stat.st_ino, buf));
      if (inode_number_width < len)
        inode_number_width = len;
    }

  f->name = xstrdup (name);
  cwd_n_used++;

  return blocks;
}

/* Return true if F refers to a directory.  */
static bool
is_directory (const struct fileinfo *f)
{
  return f->filetype == directory || f->filetype == arg_directory;
}

# define ISSLASH(C) ((C) == '/')
#   define FILE_SYSTEM_PREFIX_LEN(Filename) 0

/* Return the address of the last file name component of NAME.  If
   NAME has no relative file name components because it is a file
   system root, return the empty string.  */

static char *
last_component (char const *name)
{
  char const *base = name + FILE_SYSTEM_PREFIX_LEN (name);
  char const *p;
  bool saw_slash = false;

  while (ISSLASH (*base))
    base++;

  for (p = base; *p; p++)
    {
      if (ISSLASH (*p))
        saw_slash = true;
      else if (saw_slash)
        {
          base = p;
          saw_slash = false;
        }
    }

  return (char *) base;
}

static bool
dot_or_dotdot (char const *file_name)
{
  if (file_name[0] == '.')
    {
      char sep = file_name[(file_name[1] == '.') + 1];
      return (! sep || ISSLASH (sep));
    }
  else
    return false;
}

/* Return true if the last component of NAME is `.' or `..'
   This is so we don't try to recurse on `././././. ...' */

static bool
basename_is_dot_or_dotdot (const char *name)
{
  char const *base = last_component (name);
  return dot_or_dotdot (base);
}

#if ! HAVE_MEMPCPY && ! defined mempcpy
# define mempcpy(D, S, N) ((void *) ((char *) memcpy (D, S, N) + (N)))
#endif

/* Return the longest suffix of F that is a relative file name.
   If it has no such suffix, return the empty string.  */

static char const *
longest_relative_suffix (char const *f)
{
  for (f += FILE_SYSTEM_PREFIX_LEN (f); ISSLASH (*f); f++)
    continue;
  return f;
}

static size_t
base_len (char const *name)
{
  size_t len;

  for (len = strlen (name);  1 < len && ISSLASH (name[len - 1]);  len--)
    continue;

  return len;
}

/* Concatenate two file name components, DIR and ABASE, in
   newly-allocated storage and return the result.
   The resulting file name F is such that the commands "ls F" and "(cd
   DIR; ls BASE)" refer to the same file, where BASE is ABASE with any
   file system prefixes and leading separators removed.
   Arrange for a directory separator if necessary between DIR and BASE
   in the result, removing any redundant separators.
   In any case, if BASE_IN_RESULT is non-NULL, set
   *BASE_IN_RESULT to point to the copy of ABASE in the returned
   concatenation.  However, if ABASE begins with more than one slash,
   set *BASE_IN_RESULT to point to the sole corresponding slash that
   is copied into the result buffer.

   Return NULL if malloc fails.  */

# define DIRECTORY_SEPARATOR '/'

#define IS_ABSOLUTE_FILE_NAME(F) (ISSLASH ((F)[0]))

static char *
mfile_name_concat (char const *dir, char const *abase, char **base_in_result)
{
  char const *dirbase = last_component (dir);
  size_t dirbaselen = base_len (dirbase);
  size_t dirlen = dirbase - dir + dirbaselen;
  size_t needs_separator = (dirbaselen && ! ISSLASH (dirbase[dirbaselen - 1]));

  char const *base = longest_relative_suffix (abase);
  size_t baselen = strlen (base);

  char *p_concat = malloc (dirlen + needs_separator + baselen + 1);
  char *p;

  if (p_concat == NULL)
    return NULL;

  p = mempcpy (p_concat, dir, dirlen);
  *p = DIRECTORY_SEPARATOR;
  p += needs_separator;

  if (base_in_result)
    *base_in_result = p - IS_ABSOLUTE_FILE_NAME (abase);

  p = mempcpy (p, base, baselen);
  *p = '\0';

  return p_concat;
}

static char *
file_name_concat (char const *dir, char const *abase, char **base_in_result)
{
  char *p = mfile_name_concat (dir, abase, base_in_result);
  if (p == NULL)
    abort ();
  return p;
}

/* Remove any entries from CWD_FILE that are for directories,
   and queue them to be listed as directories instead.
   DIRNAME is the prefix to prepend to each dirname
   to make it correct relative to ls's working dir;
   if it is null, no prefix is needed and "." and ".." should not be ignored.
   If COMMAND_LINE_ARG is true, this directory was mentioned at the top level,
   This is desirable when processing directories recursively.  */

static void
extract_dirs_from_files (void)
{
  size_t i;
  size_t j;
  bool ignore_dot_and_dot_dot = false;

  /* Queue the directories last one first, because queueing reverses the
     order.  */
  for (i = cwd_n_used; i-- != 0; )
    {
      struct fileinfo *f = sorted_file[i];

      if (is_directory (f)
          && (! ignore_dot_and_dot_dot
              || ! basename_is_dot_or_dotdot (f->name)))
        {
          if (f->name[0] == '/')
            queue_directory (f->name);
          else
            {
              char *name = file_name_concat (NULL, f->name, NULL);
              queue_directory (name);
              free (name);
            }
          if (f->filetype == arg_directory)
            free (f->name);
        }
    }

  /* Now delete the directories from the table, compacting all the remaining
     entries.  */

  for (i = 0, j = 0; i < cwd_n_used; i++)
    {
      struct fileinfo *f = sorted_file[i];
      sorted_file[j] = f;
      j += (f->filetype != arg_directory);
    }
  cwd_n_used = j;
}

/* Comparison routines for sorting the files.  */

typedef void const *V;
typedef int (*qsortFunc)(V a, V b);

/* Define the 8 different sort function variants required for each sortkey.
   KEY_NAME is a token describing the sort key, e.g., ctime, atime, size.
   KEY_CMP_FUNC is a function to compare records based on that key, e.g.,
   ctime_cmp, atime_cmp, size_cmp.  Append KEY_NAME to the string,
   '[rev_][x]str{cmp|coll}[_df]_', to create each function name.  */
#define DEFINE_SORT_FUNCTIONS(key_name, key_cmp_func)			\
  /* direct, non-dirfirst versions */					\
  static int strcmp_##key_name (V a, V b)				\
  { return key_cmp_func (a, b, strcmp); }				\
                                                                        \
  /* reverse, non-dirfirst versions */					\
  static int rev_strcmp_##key_name (V a, V b)				\
  { return key_cmp_func (b, a, strcmp); }

static int
timespec_cmp (struct timespec a, struct timespec b)
{
  return (a.tv_sec < b.tv_sec ? -1
          : a.tv_sec > b.tv_sec ? 1
          : a.tv_nsec < b.tv_nsec ? -1
          : a.tv_nsec > b.tv_nsec ? 1
          : 0);
}

static inline struct timespec
get_stat_mtime (struct stat const *st)
{
  struct timespec t;
  t.tv_sec = st->st_mtime;
  t.tv_nsec = 0;
  return t;
}

static int
cmp_mtime (struct fileinfo const *a, struct fileinfo const *b,
           int (*cmp) (char const *, char const *))
{
  int diff = timespec_cmp (get_stat_mtime (&b->stat),
                           get_stat_mtime (&a->stat));
  return diff ? diff : cmp (a->name, b->name);
}

static inline struct timespec
get_stat_atime (struct stat const *st)
{
  struct timespec t;
  t.tv_sec = st->st_atime;
  t.tv_nsec = 0;
  return t;
}

static int
cmp_atime (struct fileinfo const *a, struct fileinfo const *b,
           int (*cmp) (char const *, char const *))
{
  int diff = timespec_cmp (get_stat_atime (&b->stat),
                           get_stat_atime (&a->stat));
  return diff ? diff : cmp (a->name, b->name);
}

static int
cmp_name (struct fileinfo const *a, struct fileinfo const *b,
          int (*cmp) (char const *, char const *))
{
  return cmp (a->name, b->name);
}

DEFINE_SORT_FUNCTIONS (mtime, cmp_mtime)
DEFINE_SORT_FUNCTIONS (atime, cmp_atime)
DEFINE_SORT_FUNCTIONS (name, cmp_name)

/* We have 2^3 different variants for each sortkey function
   (for 3 independent sort modes).
   The function pointers stored in this array must be dereferenced as:

    sort_variants[sort_key][use_strcmp][reverse][dirs_first]

   Note that the order in which sortkeys are listed in the function pointer
   array below is defined by the order of the elements in the time_type and
   sort_type enums!  */

#define LIST_SORTFUNCTION_VARIANTS(key_name)                        \
  { strcmp_##key_name, rev_strcmp_##key_name }

static qsortFunc const sort_functions[][2] =
  {
    LIST_SORTFUNCTION_VARIANTS (name),
    /* last are time sort functions */
    LIST_SORTFUNCTION_VARIANTS (mtime),
    LIST_SORTFUNCTION_VARIANTS (atime)
  };

/* Set up SORTED_FILE to point to the in-use entries in CWD_FILE, in order.  */

static void
initialize_ordering_vector (void)
{
  size_t i;
  for (i = 0; i < cwd_n_used; i++)
    sorted_file[i] = &cwd_file[i];
}

/* Sort the files now in the table.  */

static void
sort_files (void)
{
  if (sorted_file_alloc < cwd_n_used + cwd_n_used / 2)
    {
      free (sorted_file);
      sorted_file = xnmalloc (cwd_n_used, 3 * sizeof *sorted_file);
      sorted_file_alloc = 3 * cwd_n_used;
    }

  initialize_ordering_vector ();

  if (sort_type == sort_none)
    return;

  /* When sort_type == sort_time, use time_type as subindex.  */
  mpsort ((void const **) sorted_file, cwd_n_used,
          sort_functions[sort_type + (sort_type == sort_time ? time_type : 0)]
                        [sort_reverse]);
}

/* List all the files now in the table.  */

static void
print_current_files (void)
{
  size_t i;

  switch (format)
    {
    case one_per_line:
      for (i = 0; i < cwd_n_used; i++)
	{
	  print_file_name_and_frills (sorted_file[i]);
	  putchar ('\n');
	}
      break;

    case long_format:
      for (i = 0; i < cwd_n_used; i++)
        {
          print_long_format (sorted_file[i]);
          putchar ('\n');
        }
      break;
    }
}

static size_t
align_nstrftime (char *buf, size_t size, char const *fmt, struct tm const *tm,
                 int __utc, int __ns)
{
  size_t ret = strftime (buf, size, fmt, tm);
  return ret;
}

/* Return the expected number of columns in a long-format time stamp,
   or zero if it cannot be calculated.  */

static int
long_time_expected_width (void)
{
  static int width = -1;

  if (width < 0)
    {
      time_t epoch = 0;
      struct tm const *tm = localtime (&epoch);
      char buf[TIME_STAMP_LEN_MAXIMUM + 1];

      /* In case you're wondering if localtime can fail with an input time_t
         value of 0, let's just say it's very unlikely, but not inconceivable.
         The TZ environment variable would have to specify a time zone that
         is 2**31-1900 years or more ahead of UTC.  This could happen only on
         a 64-bit system that blindly accepts e.g., TZ=UTC+20000000000000.
         However, this is not possible with Solaris 10 or glibc-2.3.5, since
         their implementations limit the offset to 167:59 and 24:00, resp.  */
      if (tm)
        {
          size_t len =
            align_nstrftime (buf, sizeof buf, long_time_format[0], tm, 0, 0);
          if (len != 0)
            width = strlen(buf);
        }

      if (width < 0)
        width = 0;
    }

  return width;
}

/* Print the user or group name NAME, with numeric id ID, using a
   print width of WIDTH columns.  */

static void
format_user_or_group (char const *name, unsigned long int id, int width)
{
  size_t len;

  if (name)
    {
      int width_gap = width - strlen (name);
      int pad = MAX (0, width_gap);
      fputs (name, stdout);
      len = strlen (name) + pad;

      do
        putchar (' ');
      while (pad--);
    }
  else
    {
      printf ("%*lu ", width, id);
      len = width;
    }
}

struct userid
{
  union
    {
      uid_t u;
      gid_t g;
    } id;
  struct userid *next;
  char name[0];
};

/* FIXME: provide a function to free any malloc'd storage and reset lists,
   so that an application can use code like this just before exiting:
   #ifdef lint
     idcache_clear ();
   #endif
*/

static struct userid *user_alist;

/* Use the same struct as for userids.  */
static struct userid *group_alist;

/* Translate UID to a login name, with cache, or NULL if unresolved.  */

static char *
getuser (uid_t uid)
{
  struct userid *tail;
  struct userid *match = NULL;

  for (tail = user_alist; tail; tail = tail->next)
    {
      if (tail->id.u == uid)
        {
          match = tail;
          break;
        }
    }

  if (match == NULL)
    {
      struct passwd *pwent = getpwuid (uid);
      char const *name = pwent ? pwent->pw_name : "";
      match = xmalloc (offsetof (struct userid, name) + strlen (name) + 1);
      match->id.u = uid;
      strcpy (match->name, name);

      /* Add to the head of the list, so most recently used is first.  */
      match->next = user_alist;
      user_alist = match;
    }

  return match->name[0] ? match->name : NULL;
}

/* Print the name or id of the user with id U, using a print width of
   WIDTH.  */

static void
format_user (uid_t u, int width, bool stat_ok)
{
  format_user_or_group (! stat_ok ? "?" :
                        getuser (u), u, width);
}

/* Likewise, for groups.  */

/* Translate GID to a group name, with cache, or NULL if unresolved.  */

static char *
getgroup (gid_t gid)
{
  struct userid *tail;
  struct userid *match = NULL;

  for (tail = group_alist; tail; tail = tail->next)
    {
      if (tail->id.g == gid)
        {
          match = tail;
          break;
        }
    }

  if (match == NULL)
    {
      struct group *grent = getgrgid (gid);
      char const *name = grent ? grent->gr_name : "";
      match = xmalloc (offsetof (struct userid, name) + strlen (name) + 1);
      match->id.g = gid;
      strcpy (match->name, name);

      /* Add to the head of the list, so most recently used is first.  */
      match->next = group_alist;
      group_alist = match;
    }

  return match->name[0] ? match->name : NULL;
}

static void
format_group (gid_t g, int width, bool stat_ok)
{
  format_user_or_group (! stat_ok ? "?" :
                        getgroup (g), g, width);
}

/* Return the number of columns that format_user_or_group will print.  */

static int
format_user_or_group_width (char const *name, unsigned long int id)
{
  if (name)
    {
      int len = strlen (name);
      return MAX (0, len);
    }
  else
    {
      char buf[INT_BUFSIZE_BOUND (unsigned long int)];
      sprintf (buf, "%lu", id);
      return strlen (buf);
    }
}

/* Return the number of columns that format_user will print.  */

static int
format_user_width (uid_t u)
{
  return format_user_or_group_width (getuser (u), u);
}

/* Likewise, for groups.  */

static int
format_group_width (gid_t g)
{
  return format_user_or_group_width (getgroup (g), g);
}

/* Return a pointer to a formatted version of F->stat.st_ino,
   possibly using buffer, BUF, of length BUFLEN, which must be at least
   INT_BUFSIZE_BOUND (uintmax_t) bytes.  */
static char *
format_inode (char *buf, size_t buflen, const struct fileinfo *f)
{
  assert (INT_BUFSIZE_BOUND (uintmax_t) <= buflen);
  return (f->stat_ok && f->stat.st_ino != NOT_AN_INODE_NUMBER
          ? umaxtostr (f->stat.st_ino, buf)
          : (char *) "?");
}

static char
ftypelet (mode_t bits)
{
  /* These are the most common, so test for them first.  */
  if (S_ISREG (bits))
    return '-';
  if (S_ISDIR (bits))
    return 'd';

  /* Other letters standardized by POSIX 1003.1-2004.  */
  if (S_ISBLK (bits))
    return 'b';
  if (S_ISCHR (bits))
    return 'c';
  return '?';
}

static void
strmode (mode_t mode, char *str)
{
  str[0] = ftypelet (mode);
  str[1] = mode & S_IRUSR ? 'r' : '-';
  str[2] = mode & S_IWUSR ? 'w' : '-';
  str[3] = (mode & S_ISUID
            ? (mode & S_IXUSR ? 's' : 'S')
            : (mode & S_IXUSR ? 'x' : '-'));
  str[4] = mode & S_IRGRP ? 'r' : '-';
  str[5] = mode & S_IWGRP ? 'w' : '-';
  str[6] = (mode & S_ISGID
            ? (mode & S_IXGRP ? 's' : 'S')
            : (mode & S_IXGRP ? 'x' : '-'));
  str[7] = mode & S_IROTH ? 'r' : '-';
  str[8] = mode & S_IWOTH ? 'w' : '-';
  str[9] = (mode & S_ISVTX
            ? (mode & S_IXOTH ? 't' : 'T')
            : (mode & S_IXOTH ? 'x' : '-'));
  str[10] = ' ';
  str[11] = '\0';
}

static void
filemodestring (struct stat const *statp, char *str)
{
  strmode (statp->st_mode, str);
}

/* Print information about F in long format.  */
static void
print_long_format (const struct fileinfo *f)
{
  char modebuf[12];
  char buf
    [LONGEST_HUMAN_READABLE + 1		/* inode */
     + LONGEST_HUMAN_READABLE + 1	/* size in blocks */
     + sizeof (modebuf) - 1 + 1		/* mode string */
     + INT_BUFSIZE_BOUND (uintmax_t)	/* st_nlink */
     + LONGEST_HUMAN_READABLE + 2	/* major device number */
     + LONGEST_HUMAN_READABLE + 1	/* minor device number */
     + TIME_STAMP_LEN_MAXIMUM + 1	/* max length of time/date */
     ];
  size_t s;
  char *p;
  struct timespec when_timespec;
  struct tm *when_local;

  /* Compute the mode string, except remove the trailing space if no
     file in this directory has an ACL or SELinux security context.  */
  if (f->stat_ok)
    filemodestring (&f->stat, modebuf);
  else
    {
      modebuf[0] = filetype_letter[f->filetype];
      memset (modebuf + 1, '?', 10);
      modebuf[11] = '\0';
    }
  modebuf[10] = '\0';

  switch (time_type)
    {
    case time_mtime:
      when_timespec = get_stat_mtime (&f->stat);
      break;
    case time_atime:
      when_timespec = get_stat_atime (&f->stat);
      break;
    default:
      abort ();
    }

  p = buf;

  if (print_inode)
    {
      char hbuf[INT_BUFSIZE_BOUND (uintmax_t)];
      sprintf (p, "%*s ", inode_number_width,
               format_inode (hbuf, sizeof hbuf, f));
      /* Increment by strlen (p) here, rather than by inode_number_width + 1.
         The latter is wrong when inode_number_width is zero.  */
      p += strlen (p);
    }

  if (print_block_size)
    {
      char hbuf[LONGEST_HUMAN_READABLE + 1];
      char const *blocks =
        (! f->stat_ok
         ? "?"
         : umaxtostr (ST_NBLOCKS (f->stat), hbuf));
      int pad;
      for (pad = block_size_width - strlen (blocks); 0 < pad; pad--)
        *p++ = ' ';
      while ((*p++ = *blocks++))
        continue;
      p[-1] = ' ';
    }

  /* The last byte of the mode string is the POSIX
     "optional alternate access method flag".  */
  {
    char hbuf[INT_BUFSIZE_BOUND (uintmax_t)];
    sprintf (p, "%s %*s ", modebuf, nlink_width,
             ! f->stat_ok ? "?" : umaxtostr (f->stat.st_nlink, hbuf));
  }
  /* Increment by strlen (p) here, rather than by, e.g.,
     sizeof modebuf - 2 + any_has_acl + 1 + nlink_width + 1.
     The latter is wrong when nlink_width is zero.  */
  p += strlen (p);

  if (print_owner || print_group)
    {
      fputs (buf, stdout);

      if (print_owner)
        format_user (f->stat.st_uid, owner_width, f->stat_ok);

      if (print_group)
        format_group (f->stat.st_gid, group_width, f->stat_ok);

      p = buf;
    }

  if (f->stat_ok
      && (S_ISCHR (f->stat.st_mode) || S_ISBLK (f->stat.st_mode)))
    {
      char majorbuf[INT_BUFSIZE_BOUND (uintmax_t)];
      char minorbuf[INT_BUFSIZE_BOUND (uintmax_t)];
      int blanks_width = (file_size_width
                          - (major_device_number_width + 2
                             + minor_device_number_width));
      sprintf (p, "%*s, %*s ",
               major_device_number_width + MAX (0, blanks_width),
               umaxtostr (major (f->stat.st_rdev), majorbuf),
               minor_device_number_width,
               umaxtostr (minor (f->stat.st_rdev), minorbuf));
      p += file_size_width + 1;
    }
  else
    {
      char hbuf[LONGEST_HUMAN_READABLE + 1];
      char const *size =
        (! f->stat_ok
         ? "?"
         : umaxtostr (f->stat.st_size, hbuf));
      int pad;
      for (pad = file_size_width - strlen (size); 0 < pad; pad--)
        *p++ = ' ';
      while ((*p++ = *size++))
        continue;
      p[-1] = ' ';
    }

  when_local = localtime (&when_timespec.tv_sec);
  s = 0;
  *p = '\1';

  if (f->stat_ok && when_local)
    {
      struct timespec six_months_ago;
      bool recent;
      char const *fmt;

      /* If the file appears to be in the future, update the current
         time, in case the file happens to have been modified since
         the last time we checked the clock.  */
      if (timespec_cmp (current_time, when_timespec) < 0)
        {
          /* Note that gettime may call gettimeofday which, on some non-
             compliant systems, clobbers the buffer used for localtime's result.
             But it's ok here, because we use a gettimeofday wrapper that
             saves and restores the buffer around the gettimeofday call.  */
          clock_gettime (CLOCK_REALTIME, &current_time);
        }

      /* Consider a time to be recent if it is within the past six
         months.  A Gregorian year has 365.2425 * 24 * 60 * 60 ==
         31556952 seconds on the average.  Write this value as an
         integer constant to avoid floating point hassles.  */
      six_months_ago.tv_sec = current_time.tv_sec - 31556952 / 2;
      six_months_ago.tv_nsec = current_time.tv_nsec;

      recent = (timespec_cmp (six_months_ago, when_timespec) < 0
                && (timespec_cmp (when_timespec, current_time) < 0));
      fmt = long_time_format[recent];

      /* We assume here that all time zones are offset from UTC by a
         whole number of seconds.  */
      s = align_nstrftime (p, TIME_STAMP_LEN_MAXIMUM + 1, fmt,
                           when_local, 0, when_timespec.tv_nsec);
    }

  if (s || !*p)
    {
      p += s;
      *p++ = ' ';

      /* NUL-terminate the string -- fputs (via DIRED_FPUTS) requires it.  */
      *p = '\0';
    }
  else
    {
      /* The time cannot be converted using the desired format, so
         print it as a huge integer number of seconds.  */
      char hbuf[INT_BUFSIZE_BOUND (intmax_t)];
      sprintf (p, "%*s ", long_time_expected_width (),
               (! f->stat_ok
                ? "?"
                : umaxtostr (when_timespec.tv_sec, hbuf)));
      /* FIXME: (maybe) We discarded when_timespec.tv_nsec. */
      p += strlen (p);
    }

  fputs (buf, stdout);
  print_name_with_quoting (f);
}

/* Output to OUT a quoted representation of the file name NAME,
   using OPTIONS to control quoting.  Produce no output if OUT is NULL.
   Store the number of screen columns occupied by NAME's quoted
   representation into WIDTH, if non-NULL.  Return the number of bytes
   produced.  */

static size_t
quote_name (FILE *out, const char *name, size_t *width)
{
  if (out != NULL)
    fputs (name, out);
  if (width != NULL)
    *width = strlen(name);
  return strlen(name);
}

static size_t
print_name_with_quoting (const struct fileinfo *f)
{
  size_t width = quote_name (stdout, f->name, NULL);
  return width;
}

/* Print the file name of `f' with appropriate quoting.
   Also print file size, inode number, and filetype indicator character,
   as requested by switches.  */

static size_t
print_file_name_and_frills (const struct fileinfo *f)
{
  char buf[MAX (LONGEST_HUMAN_READABLE + 1, INT_BUFSIZE_BOUND (uintmax_t))];

  if (print_inode)
    printf ("%*s ", inode_number_width,
            format_inode (buf, sizeof buf, f));

  if (print_block_size)
    printf ("%*s ", block_size_width,
            ! f->stat_ok ? "?"
            : umaxtostr(ST_NBLOCKS (f->stat), buf));

  size_t width = print_name_with_quoting (f);

  return width;
}

/* Put DIRNAME/NAME into DEST, handling `.' and `/' properly.  */
/* FIXME: maybe remove this function someday.  See about using a
   non-malloc'ing version of file_name_concat.  */

static void
attach (char *dest, const char *dirname, const char *name)
{
  const char *dirnamep = dirname;

  /* Copy dirname if it is not ".".  */
  if (dirname[0] != '.' || dirname[1] != 0)
    {
      while (*dirnamep)
        *dest++ = *dirnamep++;
      /* Add '/' if `dirname' doesn't already end with it.  */
      if (dirnamep > dirname && dirnamep[-1] != '/')
        *dest++ = '/';
    }
  while (*name)
    *dest++ = *name++;
  *dest = 0;
}
