// -*- C++ -*-
// String creation functions.
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
#ifndef CH_T_STRING_H
#define CH_T_STRING_H 1

#include <string>
#if HAVE_SSTREAM
#include <sstream>
#elif HAVE_STRSTREAM
#include <strstream>
#endif

// set namespace to avoid possible clashes
CH_BEGIN_NAMESPACE

// This is necessary to make the creation of strings easier and
// allowing both stringstream and strstream to be used without
// cluttering up the code everywhere (the clutter is only needed
// here).

// this is a template function, T needs operator<<
// this functions takes what you have and turns it into a string
template<typename T>
CH_STD::string
t_string(const T& t)
{
#if HAVE_SSTREAM
  CH_STD::ostringstream t_stream;
  t_stream << t << CH_STD::ends;
  // FIXME: when you figure out how to use stringstreams
  CH_STD::string t_string(t_stream.str());
#elif HAVE_STRSTREAM // not HAVE_SSTREAM
  CH_STD::ostrstream t_stream;
  t_stream << t << CH_STD::ends;
  CH_STD::string t_string(t_stream.str()); // convert strstream to string
  delete []t_stream.str();	// required if you call str()
#else // not HAVE_SSTREAM nor HAVE_STRSTREAM
#error You must have either <sstream> or <strstream> C++ header files.
#endif // not HAVE_SSTREAM nor HAVE_STRSTREAM
  return t_string;
}

// This is a template function.  Both types, Prefix and Postfix, need
// operator<<.
// this function concatenates its arguments into a string
template<typename Prefix,typename Postfix>
CH_STD::string
concatenate(const Prefix& prefix, const Postfix& postfix)
{
  return t_string(prefix) + t_string(postfix);
}

CH_END_NAMESPACE

#endif // not CH_T_STRING_H

/* $Id: t_string.h,v 1.1.1.1 2004/11/25 20:24:06 banjo Exp $ */
