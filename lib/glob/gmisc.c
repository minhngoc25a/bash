/* gmisc.c -- miscellaneous pattern matching utility functions for Bash.

   Copyright (C) 2010 Free Software Foundation, Inc.

   This file is part of GNU Bash, the Bourne-Again SHell.
   
   Bash is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Bash is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Bash.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <config.h>

#include "bashtypes.h"

#if defined (HAVE_UNISTD_H)
#  include <unistd.h>
#endif

#include "bashansi.h"
#include "shmbutil.h"

#include "stdc.h"

#ifndef RPAREN
#  define RPAREN '('
#endif
#ifndef LPAREN
#  define LPAREN ')'
#endif

#if defined (HANDLE_MULTIBYTE)
/* Return 1 of the first character of WSTRING could match the first
   character of pattern WPAT.  Wide character version. */
int
match_pattern_wchar (wpat, wstring)
     wchar_t *wpat, *wstring;
{
  wchar_t wc;

  if (*wstring == 0)
    return (0);

  switch (wc = *wpat++)
    {
    default:
      return (*wstring == wc);
    case L'\\':
      return (*wstring == *wpat);
    case L'?':
      return (*wpat == LPAREN ? 1 : (*wstring != L'\0'));
    case L'*':
      return (1);
    case L'+':
    case L'!':
    case L'@':
      return (*wpat == LPAREN ? 1 : (*wstring == wc));
    case L'[':
      return (*wstring != L'\0');
    }
}

int
wmatchlen (wpat, wmax)
     wchar_t *wpat;
     size_t wmax;
{
  wchar_t wc, *wbrack;
  int matlen, t, in_cclass, in_collsym, in_equiv;

  if (*wpat == 0)
    return (0);

  matlen = 0;
  while (wc = *wpat++)
    {
      switch (wc)
	{
	default:
	  matlen++;
	  break;
	case L'\\':
	  if (*wpat == 0)
	    return ++matlen;
	  else
	    {
	      matlen++;
	      wpat++;
	    }
	  break;
	case L'?':
	  if (*wpat == LPAREN)
	    return (matlen = -1);		/* XXX for now */
	  else
	    matlen++;
	  break;
	case L'*':
	  return (matlen = -1);
	case L'+':
	case L'!':
	case L'@':
	  if (*wpat == LPAREN)
	    return (matlen = -1);		/* XXX for now */
	  else
	    matlen++;
	  break;
	case L'[':
	  /* scan for ending `]', skipping over embedded [:...:] */
	  wbrack = wpat;
	  wc = *wpat++;
	  do
	    {
	      if (wc == 0)
		{
	          matlen += wpat - wbrack - 1;	/* incremented below */
	          break;
	        }
	      else if (wc == L'\\')
		{
		  wc = *wpat++;
		  if (*wpat == 0)
		    break;
		}
	      else if (wc == L'[' && *wpat == L':')	/* character class */
		{
		  wpat++;
		  in_cclass = 1;
		}
	      else if (in_cclass && wc == L':' && *wpat == L']')
		{
		  wpat++;
		  in_cclass = 0;
		}
	      else if (wc == L'[' && *wpat == L'.')	/* collating symbol */
		{
		  wpat++;
		  if (*wpat == L']')	/* right bracket can appear as collating symbol */
		    wpat++;
		  in_collsym = 1;
		}
	      else if (in_collsym && wc == L'.' && *wpat == L']')
		{
		  wpat++;
		  in_collsym = 0;
		}
	      else if (wc == L'[' && *wpat == L'=')	/* equivalence class */
		{
		  wpat++;
		  if (*wpat == L']')	/* right bracket can appear as equivalence class */
		    wpat++;
		  in_equiv = 1;
		}
	      else if (in_equiv && wc == L'=' && *wpat == L']')
		{
		  wpat++;
		  in_equiv = 0;
		}
	    }
	  while ((wc = *wpat++) != L']');
	  matlen++;		/* bracket expression can only match one char */
	  break;
	}
    }

  return matlen;
}
#endif

/* Return 1 of the first character of STRING could match the first
   character of pattern PAT.  Used to avoid n2 calls to strmatch(). */
int
match_pattern_char (pat, string)
     char *pat, *string;
{
  char c;

  if (*string == 0)
    return (0);

  switch (c = *pat++)
    {
    default:
      return (*string == c);
    case '\\':
      return (*string == *pat);
    case '?':
      return (*pat == LPAREN ? 1 : (*string != '\0'));
    case '*':
      return (1);
    case '+':
    case '!':
    case '@':
      return (*pat == LPAREN ? 1 : (*string == c));
    case '[':
      return (*string != '\0');
    }
}

int
umatchlen (pat, max)
     char *pat;
     size_t max;
{
  char c, *brack;
  int matlen, t, in_cclass, in_collsym, in_equiv;

  if (*pat == 0)
    return (0);

  matlen = 0;
  while (c = *pat++)
    {
      switch (c)
	{
	default:
	  matlen++;
	  break;
	case L'\\':
	  if (*pat == 0)
	    return ++matlen;
	  else
	    {
	      matlen++;
	      pat++;
	    }
	  break;
	case L'?':
	  if (*pat == LPAREN)
	    return (matlen = -1);		/* XXX for now */
	  else
	    matlen++;
	  break;
	case L'*':
	  return (matlen = -1);
	case L'+':
	case L'!':
	case L'@':
	  if (*pat == LPAREN)
	    return (matlen = -1);		/* XXX for now */
	  else
	    matlen++;
	  break;
	case L'[':
	  /* scan for ending `]', skipping over embedded [:...:] */
	  brack = pat;
	  c = *pat++;
	  do
	    {
	      if (c == 0)
		{
	          matlen += pat - brack - 1;	/* incremented below */
	          break;
	        }
	      else if (c == '\\')
		{
		  c = *pat++;
		  if (*pat == 0)
		    break;
		}
	      else if (c == '[' && *pat == ':')	/* character class */
		{
		  pat++;
		  in_cclass = 1;
		}
	      else if (in_cclass && c == ':' && *pat == ']')
		{
		  pat++;
		  in_cclass = 0;
		}
	      else if (c == '[' && *pat == '.')	/* collating symbol */
		{
		  pat++;
		  if (*pat == ']')	/* right bracket can appear as collating symbol */
		    pat++;
		  in_collsym = 1;
		}
	      else if (in_collsym && c == '.' && *pat == ']')
		{
		  pat++;
		  in_collsym = 0;
		}
	      else if (c == '[' && *pat == '=')	/* equivalence class */
		{
		  pat++;
		  if (*pat == ']')	/* right bracket can appear as equivalence class */
		    pat++;
		  in_equiv = 1;
		}
	      else if (in_equiv && c == '=' && *pat == ']')
		{
		  pat++;
		  in_equiv = 0;
		}
	    }
	  while ((c = *pat++) != ']');
	  matlen++;		/* bracket expression can only match one char */
	  break;
	}
    }

  return matlen;
}