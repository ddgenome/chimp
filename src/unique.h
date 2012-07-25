// -*- C++ -*-
// Class to create unique words for instances of classes.
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
#ifndef CH_UNIQUE_H
#define CH_UNIQUE_H 1

#include <map>
#include <string>
#include "counter.h"

// set namespace to avoid possible clashes
CH_BEGIN_NAMESPACE

// class to centralize the unique naming feature
// include in your class as a static variable
class unique
{
  CH_STD::map<CH_STD::string,counter> list; // list of words
  int index;			// unique index counter
  CH_STD::string prefix;	// class specific prefix
  bool multiple;		// multiple instances with same word allowed

private:
  // prevent copy contruction and assignment
  unique(const unique&);
  unique& operator=(const unique&);
  // create a unique word and return it
  CH_STD::string create();
public:
  // ctor: (default) supply prefix and optional uniqueness flag
  explicit unique(CH_STD::string prefix_ = "u", bool multiple_ = false);
  // dtor: do nothing
  ~unique();

  // test if word is already in map, return current count of that word
  int count(CH_STD::string word) const;
  // create a unique word, insert it, and return it
  CH_STD::string insert();
  // insert a new word into map, return current count of that word
  // return -1 if unable to insert the WORD
  int insert(CH_STD::string word);
  // remove word from list, return current count of that word
  // return -1 if WORD is not in list
  int remove(CH_STD::string word);
}; // end class unique

CH_END_NAMESPACE

#endif // not CH_UNIQUE_H

/* $Id: unique.h,v 1.1.1.1 2004/11/25 20:24:06 banjo Exp $ */
