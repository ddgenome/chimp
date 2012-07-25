// Methods for maintaining debugging information.
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

#include "debug.h"
#include <iostream>
#include "file.h"
#include "t_string.h"

// set namespace to avoid possible clashes
CH_BEGIN_NAMESPACE

// instantiate singleton instance
debug debug::single;

// debug class methods
// ctor: default (private) set up defaults
debug::debug()
  : level(1U), file_stream()
{}

// dtor: close the stream if it is a file
debug::~debug()
{
  // see if file stream is open
  if (file_stream.is_open())
    {
      // close it
      file_stream.close();
    }
}

// debug public methods
// return singleton instantiation of debug
debug&
debug::get()
{
  return single;
}

// return debug level
unsigned int
debug::get_level() const
{
  return level;
}

// set the level, return old one
unsigned int
debug::set_level(unsigned int level_)
{
  unsigned int old(level);
  level = level_;
  return old;
}

// return reference to the output stream
CH_STD::ostream&
debug::get_stream()
{
  // see if a file is being used
  if (file_stream.is_open())
    {
      return file_stream;
    }
  // else return standard error
  return CH_STD::cerr;
}

// set the output to a file
void
debug::set_output(const CH_STD::string& file_name)
  throw (bad_file)
{
  // open the file, destroying the contents
  file_stream.open(file_name.c_str());
  // make sure it is ok
  if (!file_stream)
    {
      // find out why
      file_stat fail(file_name);
      throw bad_file(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		     ":debug::set_output(): could not open file "
		     + file_name + ": " + fail.why_no_write());
    }
  return;
}

CH_END_NAMESPACE

/* $Id: debug.cc,v 1.1.1.1 2004/11/25 20:24:05 banjo Exp $ */
