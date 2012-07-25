// Methods to manipulate a single lattice point on a kmc surface.
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

#include "point.h"

// set namespace to avoid possible clashes
CH_BEGIN_NAMESPACE

// class lattice_point methods
// ctor: give species pointer and position in lattice
lattice_point::lattice_point(model_species* type_, unsigned int row_,
			     unsigned int column_)
  : type(type_), row(row_), column(column_)
{}

// dtor: do nothing
lattice_point::~lattice_point()
{}

// lattice_point class public methods
// return pointer to the species which occupies this point
model_species*
lattice_point::get_species() const
{
  return type;
}

// return the position in the lattice
CH_STD::pair<unsigned int,unsigned int>
lattice_point::get_position() const
{
  return CH_STD::make_pair(row, column);
}

// change the species at this lattice point, return old
model_species*
lattice_point::set_species(model_species* type_)
{
  model_species* old(type);
  type = type_;
  return old;
}

CH_END_NAMESPACE

/* $Id: point.cc,v 1.1.1.1 2004/11/25 20:24:08 banjo Exp $ */
