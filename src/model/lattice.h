// -*- C++ -*-
// Class for the creation and maintenance of the kmc lattice.
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
#ifndef CH_MODEL_LATTICE_H
#define CH_MODEL_LATTICE_H 1

#include <vector>
#include "except.h"
#include "mechanism.h"
#include "point.h"

// set namespace to avoid possible clashes
CH_BEGIN_NAMESPACE

// kinetic Monte Carlo lattice class
class lattice
{
  unsigned int size;		// square root of the number of lattice points
  lattice_point::seq points;	// sequence of pointers to lattice points
  bool initialized;		// whether surface has been initialized

private:
  // prevent assignment
  lattice& operator=(const lattice&);
  // create the surface, using size, fill points with empty site
  void make_surface(model_species* empty_site);
public:
  // ctor: (default) optional size
  explicit lattice(unsigned int size_ = 0U);
  // ctor: copy
  explicit lattice(const lattice& original);
  // dtor: delete the lattice_points
  ~lattice();

  // return the size of one side of the lattice
  unsigned int get_size() const;
  // change the size of the lattice, return old value
  unsigned int set_size(unsigned int size_)
    throw (bad_request); // this
  // initialize the surface, if necessary
  void initialize(model_species* empty_site)
    throw (bad_pointer); // this
  // return pointer to species at the given point
  lattice_point* get_point(unsigned int row, unsigned int column) const
    throw (bad_request, bad_value); // this
  // create string representation of a picture of the surface
  CH_STD::string stringify(unsigned int width = 8U) const
    throw (bad_request, bad_value); // get_point()
}; // end class lattice

CH_END_NAMESPACE

#endif // not CH_MODEL_LATTICE_H

/* $Id: lattice.h,v 1.1.1.1 2004/11/25 20:24:08 banjo Exp $ */
