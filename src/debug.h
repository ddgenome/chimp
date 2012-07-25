// -*- C++ -*-
// Debugging level information.
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
#ifndef CH_DEBUG_H
#define CH_DEBUG_H 1

#include <fstream>
#include <string>
#include "except.h"

// set namespace to avoid possible clashes
CH_BEGIN_NAMESPACE

// class to control debugging information output
class debug
{
  static debug single;		// singleton instantiation of debug
  unsigned int level;		// level of debugging information output
  CH_STD::ofstream file_stream; // file to where output should go

private:
  // make all ctors  private
  // ctor: (default) set default values
  debug();
  // disallow copy construction and assignment
  debug(const debug&);
  debug& operator=(const debug&);
public:
  // close file_stream if necessary
  ~debug();

  // return singleton instantiation of debug
  static debug& get();
  // return debug level
  unsigned int get_level() const;
  // set the level, return old one
  unsigned int set_level(unsigned int level_);
  // return reference to the output stream
  CH_STD::ostream& get_stream();
  // set the output to a file
  void set_output(const CH_STD::string& file_name)
    throw (bad_file);
}; // end class debug

CH_END_NAMESPACE

#endif // not CH_TEMPLATE_H

/* $Id: debug.h,v 1.1.1.1 2004/11/25 20:24:05 banjo Exp $ */
