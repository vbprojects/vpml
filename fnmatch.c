/* Copyright (C) 1992 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.  */

/* Modified slightly by Brian Berliner <berliner@sun.com> and
   Jim Blandy <jimb@cyclic.com> for CVS use */
/* Modified slightly by j.f. dockes for recoll use */

// #include "autoconfig.h"
#ifdef _WIN32
#include <ctype.h>
static inline int fold_fn_char(int c)
{
    /* Only ASCII for now... */
    if (c == '\\')
        return '/';
    if (c > 0 && c <= 127)
        return tolower(c);
    return c;
}
#define FOLD_FN_CHAR(c) fold_fn_char(c)
#endif /* _WIN32 */

/* Some file systems are case-insensitive.  If FOLD_FN_CHAR is
   #defined, it maps the character C onto its "canonical" form.  In a
   case-insensitive system, it would map all alphanumeric characters
   to lower case.  Under Windows NT, / and \ are both path component
   separators, so FOLD_FN_CHAR would map them both to /.  */
#ifndef FOLD_FN_CHAR
#define FOLD_FN_CHAR(c) (c)
#endif

#include <errno.h>
#include "fnmatch.h"


/* Match STRING against the filename pattern PATTERN, returning zero if
   it matches, nonzero if not.  */
int fnmatch (const char *pattern, const char *string, int flags)
{
  register const char *p = pattern, *n = string;
  register char c;

  if ((flags & ~__FNM_FLAGS) != 0)
    {
      errno = EINVAL;
      return -1;
    }

  while ((c = *p++) != '\0')
    {
      switch (c)
    {
    case '?':
      if (*n == '\0')
        return FNM_NOMATCH;
      else if ((flags & FNM_PATHNAME) && *n == '/')
        return FNM_NOMATCH;
      else if ((flags & FNM_PERIOD) && *n == '.' &&
           (n == string || ((flags & FNM_PATHNAME) && n[-1] == '/')))
        return FNM_NOMATCH;
      break;
      
    case '\\':
      if (!(flags & FNM_NOESCAPE))
        c = *p++;
      if (*n != c)
        return FNM_NOMATCH;
      break;
      
    case '*':
      if ((flags & FNM_PERIOD) && *n == '.' &&
          (n == string || ((flags & FNM_PATHNAME) && n[-1] == '/')))
        return FNM_NOMATCH;
      
      for (c = *p++; c == '?' || c == '*'; c = *p++, ++n)
        if (((flags & FNM_PATHNAME) && *n == '/') ||
        (c == '?' && *n == '\0'))
          return FNM_NOMATCH;
      
      if (c == '\0')
        return 0;
      
      {
        char c1 = (!(flags & FNM_NOESCAPE) && c == '\\') ? *p : c;
        for (--p; *n != '\0'; ++n)
          if ((c == '[' || *n == c1) &&
          fnmatch(p, n, flags & ~FNM_PERIOD) == 0)
        return 0;
        return FNM_NOMATCH;
      }
      
    case '[':
      {
        /* Nonzero if the sense of the character class is inverted.  */
        register int not;
        
        if (*n == '\0')
          return FNM_NOMATCH;
        
        if ((flags & FNM_PERIOD) && *n == '.' &&
        (n == string || ((flags & FNM_PATHNAME) && n[-1] == '/')))
          return FNM_NOMATCH;
        
        not = (*p == '!' || *p == '^');
        if (not)
          ++p;
        
        c = *p++;
        for (;;)
          {
        register char cstart = c, cend = c;
        
        if (!(flags & FNM_NOESCAPE) && c == '\\')
          cstart = cend = *p++;
        
        if (c == '\0')
          /* [ (unterminated) loses.  */
          return FNM_NOMATCH;
        
        c = *p++;
        
        if ((flags & FNM_PATHNAME) && c == '/')
          /* [/] can never match.  */
          return FNM_NOMATCH;
        
        if (c == '-' && *p != ']')
          {
            cend = *p++;
            if (!(flags & FNM_NOESCAPE) && cend == '\\')
              cend = *p++;
            if (cend == '\0')
              return FNM_NOMATCH;
            c = *p++;
          }
        
        if (*n >= cstart && *n <= cend)
          goto matched;
        
        if (c == ']')
          break;
          }
        if (!not)
          return FNM_NOMATCH;
        break;
        
      matched:;
        /* Skip the rest of the [...] that already matched.  */
        while (c != ']')
          {
        if (c == '\0')
          /* [... (unterminated) loses.  */
          return FNM_NOMATCH;
        
        c = *p++;
        if (!(flags & FNM_NOESCAPE) && c == '\\')
          /* 1003.2d11 is unclear if this is right.  %%% */
          ++p;
          }
        if (not)
          return FNM_NOMATCH;
      }
      break;
      
    default:
      if (FOLD_FN_CHAR (c) != FOLD_FN_CHAR (*n))
        return FNM_NOMATCH;
    }
      
      ++n;
    }

  if (*n == '\0')
    return 0;

  return FNM_NOMATCH;
}
