// String comparison methods.
// Copyright (C) 2004 David Dooling <banjo@users.sourceforge.net>
//
// This file is part of CHIMP.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.
//
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <cctype>		// tolower()
#include "compare.h"

// set namespace to avoid possible clashes
CH_BEGIN_NAMESPACE

// this function does a case-insensitive comparison
// returns 0 if same, >0 if s1 comes after s2, and <0 if s1 comes before s2
// defaults max = 0 (compare complete strings)
int
icompare(const CH_STD::string& s1, const CH_STD::string& s2, int max)
{
  // set up safe maximum if max <= 0
  if (max <= 0)
    {
      max = s1.length() + s2.length() + 5;
    }
  // use iterators to test characters one by one
  CH_STD::string::const_iterator p1(s1.begin());
  CH_STD::string::const_iterator p2(s2.begin());
  for (int i = 0; p1 != s1.end() && p2 != s2.end(); ++p1, ++p2)
    {
      // get lower case chars and compare
      char c1(tolower(*p1));
      char c2(tolower(*p2));
      if (c1 != c2)
	{
	  return (c1 < c2) ? -1 : 1;
	}
      if (++i == max)
	{
	  return 0;
	}
    }
  // they match up to end of one of the strings, use length
  return s1.size() - s2.size();
}

CH_END_NAMESPACE

/* $Id: compare.cc,v 1.1.1.1 2004/11/25 20:24:05 banjo Exp $ */
