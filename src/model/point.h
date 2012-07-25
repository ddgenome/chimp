// -*- C++ -*-
// Description of a single lattice point in the kmc surface.
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
#ifndef CH_MODEL_POINT_H
#define CH_MODEL_POINT_H 1

#include <deque>
#include <utility>
#include <vector>
#include "species.h"

// set namespace to avoid possible clashes
CH_BEGIN_NAMESPACE

// information needed for a singe lattice point
class lattice_point
{
public:
  // set up typedef's
  typedef CH_STD::vector<lattice_point*> seq;
  typedef seq::iterator seq_iter;
  typedef seq::const_iterator seq_citer;
  typedef CH_STD::deque<lattice_point*> deq;
  typedef deq::iterator deq_iter;
  typedef deq::const_iterator deq_citer;

private:
  model_species* type;		// type of species at this lattice point
  unsigned int row;		// row of surface of this point
  unsigned int column;		// column of surface of this point

private:
  // prevent copy construction and assignment
  lattice_point(const lattice_point&);
  lattice_point& operator=(const lattice_point&);
public:
  // ctor: give the model_species pointer and position in lattice
  lattice_point(model_species* type_, unsigned int row_, unsigned int column_);
  // dtor: do nothing
  ~lattice_point();

  // return pointer to the species which occupies this point
  model_species* get_species() const;
  // return the position in the lattice
  CH_STD::pair<unsigned int,unsigned int> get_position() const;
  // change the species at this lattice point, return old
  model_species* set_species(model_species* type_);
}; // end class lattice_point

CH_END_NAMESPACE

#endif // not CH_MODEL_POINT_H

/* $Id: point.h,v 1.1.1.1 2004/11/25 20:24:08 banjo Exp $ */
