// Methods to create unique words for instances of classes.
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

#include "t_string.h"
#include "unique.h"

// set namespace to avoid possible clashes
CH_BEGIN_NAMESPACE

// unique methods
// ctor: supply prefix and uniqueness flag
// ctor: defaults prefix_ = "u", multiple_ = false
unique::unique(CH_STD::string prefix_, bool multiple_)
  : list(), index(0), prefix(prefix_), multiple(multiple_)
{}

// dtor: do nothing
unique::~unique()
{}

// unique private methods
// create a word that is not currently in the list
CH_STD::string
unique::create()
{
  CH_STD::string word;
  // loop until count for word is zero
  while (list.count(word = concatenate(prefix, index++)))
    ; // empty loop
  return word;
}

// unique public methods
// test if word is in list, return count
int
unique::count(CH_STD::string word) const
{
  CH_STD::map<CH_STD::string,counter>::const_iterator it;
  if ((it = list.find(word)) != list.end())
    {
      return it->second.get_count();
    }
  return 0; // does not exist
}

// create a unique word, insert it into the list, and return it
CH_STD::string
unique::insert()
{
  CH_STD::string word(create());
  insert(word);	// it is unique so no need to test return value
  return word;
}

// insert a new word into map, return current count of that word
// return -1 if unable to insert word because it exists already and
// multiple = false
int
unique::insert(CH_STD::string word)
{
  // if multiple instances aren't allowed and it already exists
  if (!multiple && list.count(word))
    {
      return -1; // error
    }
  // element is automatically created if it does not exist.
  return ++list[word];
}

// decrement count of WORD, remove if approriate, return current count
// return -1 if WORD is not in list
int
unique::remove(CH_STD::string word)
{
  // make sure _word is in list
  if (!list.count(word))
    {
      return -1; // error: removing word that doesn't exist
    }
  int n(--list[word]);
  if (n == 0)
    {
      list.erase(word);
    }
  return n;
}

CH_END_NAMESPACE

/* $Id: unique.cc,v 1.1.1.1 2004/11/25 20:24:06 banjo Exp $ */
