// -*- C++ -*-
// File for counting instances of objecs, names, etc.
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
#ifndef CH_COUNTER_H
#define CH_COUNTER_H 1

// set namespace to avoid possible clashes
CH_BEGIN_NAMESPACE

class counter
{
  int i;			// current count

public:
  // ctor: (default) initialize i to zero
  counter();
  // ctor: copy
  counter(const counter& original);
  // assignment
  counter& operator=(const counter& right);
  // dtor: do nothing
  ~counter();

  // return I
  int get_count() const;
  // pre-increment operator
  int operator++();
  // post-increment operator
  int operator++(int);
  // pre-decrement operator
  int operator--();
  // post-increment operator
  int operator--(int);
}; // end class counter

CH_END_NAMESPACE

#endif // not CH_COUNTER_H

/* $Id: counter.h,v 1.1.1.1 2004/11/25 20:24:05 banjo Exp $ */
