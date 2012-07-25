// Methods for the creation and manipulating of the kmc lattice.
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

#include "lattice.h"
#include "t_string.h"

// set namespace to avoid possible clashes
CH_BEGIN_NAMESPACE

// class lattice methods
// ctor: optional lattice size
// ctor: default size_ = 0U
lattice::lattice(unsigned int size_)
  : size(size_), points(), initialized(false)
{}

// ctor: copy
lattice::lattice(const lattice& original)
  : size(original.size), points(), initialized(false)
{}

// dtor: delete the lattice_points
lattice::~lattice()
{
  // loop through and delete the pointers
  for (lattice_point::seq_iter it(points.begin()); it != points.end(); ++it)
    {
      delete *it;
      *it = 0;
    }
}

// lattice class private methods
// create the surface, using size, fill points with empty site
void
lattice::make_surface(model_species* empty_site)
{
  // reserve the space we need for the ``surface''
  points.reserve(size * size);
  // we have to do this double loop structure to set the row and column
  // values in the lattice_point's
  for (unsigned int i(0U); i < size; ++i)
    {
      for (unsigned int j(0U); j < size; ++j)
	{
	  // create and insert lattice_point
	  points.push_back(new lattice_point(empty_site, i, j));
	}
    }
  return;
}

// lattice class public methods
// return the size of one side of the lattice
unsigned int
lattice::get_size() const
{
  return size;
}

// change the size of the lattice, return old value
unsigned int
lattice::set_size(unsigned int size_)
  throw (bad_request)
{
  if (initialized)
    {
      throw bad_request(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			"lattice::set_size(): the lattice surface has already "
			"been created, so it cannot be resized");
    }
  unsigned int old(size);
  size = size_;
  return old;
}

// create the surface and initilize it, if necessary
void
lattice::initialize(model_species* empty_site)
  throw (bad_pointer)
{
  // see if we need to set up a surface
  if (size > 0U)
    {
      // make sure empty site exists
      if (empty_site == 0)
	{
	  throw bad_pointer(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			    ":lattice::initialize(): surface has non-zero "
			    "number of sites, but an empty site was not "
			    "included in the model");
	}
      // create a surface of empty sites
      make_surface(empty_site);
    }
  // initialization complete
  initialized = true;
  return;
}

// return pointer to species at the given point
lattice_point*
lattice::get_point(unsigned int row, unsigned int column) const
  throw (bad_request, bad_value)
{
  if (!initialized)
    {
      throw bad_request(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			"lattice::get_point(): lattice has not been initialized "
			"so a point can not be requested");
    }
  if (row >= size)
    {
      throw bad_value(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		      "lattice::get_point(): requested lattice point has row ("
		      + t_string(row) + ") greater than the total number of "
		      "rows (" + t_string(size) + ")");
    }
  if (column >= size)
    {
      throw bad_value(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		      "lattice::get_point(): requested lattice point has "
		      "column (" + t_string(row) + ") greater than the total "
		      "number of columns (" + t_string(size) + ")");
    }
  // get the appropriate lattice_point
  return points[row * size + column];
}

// create string representation of a picture of the surface
// default width = 8U
CH_STD::string
lattice::stringify(unsigned int width) const
  throw (bad_request, bad_value)
{
  CH_STD::string surface_string;
  // double loop through the list of points
  for (unsigned int i(0U); i < size; ++i)
    {
      for (unsigned int j(0U); j < size; ++j)
	{
	  // get the species name
	  CH_STD::string name(get_point(i, j)->get_species()->get_name());
	  // resize the name to the proper width using spaces
	  name.resize(width, ' ');
	  // add species name to the string (and blank space)
	  surface_string.append(name);
	}
      // add new line after each row
      // NOTE: would like to push_back(char) here, but it is not implemented
      surface_string.append("\n");
    }
  return surface_string;
}

CH_END_NAMESPACE

/* $Id: lattice.cc,v 1.1.1.1 2004/11/25 20:24:08 banjo Exp $ */
