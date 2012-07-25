// Methods to create and analyze reaction ensembles.
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

#include "ensemble.h"
#include <algorithm>		// sort()

// set namespace to avoid possible clashes
CH_BEGIN_NAMESPACE

// ensemble class methods
// ctor: sort the given list of species and insert into the sequence
ensemble::ensemble(const model_species::seq& speciess)
  : sorted_species(), coordination(0U)
{
  // reserve the maximum size we would need
  sorted_species.reserve(speciess.size());
  // loop through the species, only inserting the surface species
  for (model_species::seq_citer it(speciess.begin()); it != speciess.end();
       ++it)
    {
      unsigned int coord((*it)->get_surface_coordination());
      // see if it is a surface species
      if (coord > 0U)
	{
	  sorted_species.push_back(*it);
	  coordination += coord;
	}
    }
  // sort the species sequence
  CH_STD::sort(sorted_species.begin(), sorted_species.end());
}

// ctor: copy
ensemble::ensemble(const ensemble& original)
  : sorted_species(original.sorted_species),
    coordination(original.coordination)
{}

// dtor: do nothing
ensemble::~ensemble()
{}

// less than operator required for map
bool
ensemble::operator<(const ensemble& right) const
{
  // use the operator< for the vector
  return sorted_species < right.sorted_species;
}

// return true if the sorted_species sequence is empty
// i.e., return true if reaction contained no surface species
unsigned int
ensemble::get_size() const
{
  return sorted_species.size();
}

// return total coordination of ensemble
unsigned int
ensemble::get_coordination() const
{
  return coordination;
}

// return iterator to the beginning of the species sequence
model_species::seq_citer
ensemble::begin() const
{
  return sorted_species.begin();
}

// return iterator to the end of the species sequences
model_species::seq_citer
ensemble::end() const
{
  return sorted_species.end();
}

CH_END_NAMESPACE

/* $Id: ensemble.cc,v 1.1.1.1 2004/11/25 20:24:08 banjo Exp $ */
