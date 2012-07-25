// Methods for counting instances of things.
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

#include "counter.h"

// set namespace to avoid possible clashes
CH_BEGIN_NAMESPACE

// counter methods
// note: these methods are all very small, but not critical to performance
//       so I put them in here rather than inline them.
// ctor: (default) initialize counter to zero
counter::counter()
  : i(0)
{}

// ctor: copy
counter::counter(const counter& original)
  : i(original.i)
{}

// assignment
counter&
counter::operator=(const counter& right)
{
  // check for self-assignment
  if (this == &right) return *this;
  i = right.i;
  return *this;
}

// dtor: do nothing
counter::~counter()
{}

// return I
int
counter::get_count() const
{
  return i;
}

// pre-increment operator
int
counter::operator++()
{
  return ++i;
}

// post-increment operator
int
counter::operator++(int)
{
  return i++;
}

// pre-decrement operator
int
counter::operator--()
{
  return --i;
}

// post-increment operator
int
counter::operator--(int)
{
  return i--;
}

CH_END_NAMESPACE

/* $Id: counter.cc,v 1.1.1.1 2004/11/25 20:24:05 banjo Exp $ */
