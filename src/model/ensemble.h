// -*- C++ -*-
// This class maintains the ensembles required for reactions.
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
#ifndef CH_MODEL_ENSEMBLE_H
#define CH_MODEL_ENSEMBLE_H 1

#include <map>
#include <vector>
#include "point.h"
#include "species.h"

// set namespace to avoid possible clashes
CH_BEGIN_NAMESPACE

// this class describes the species required for a reaction
class ensemble
{
public:
  // typedef's
  typedef CH_STD::vector<ensemble*> seq;
  typedef seq::iterator seq_iter;
  typedef seq::const_iterator seq_citer;
  typedef CH_STD::deque<ensemble*> deq;
  typedef deq::iterator deq_iter;
  typedef deq::const_iterator deq_citer;

private:
  model_species::seq sorted_species; // the surface reactants
  unsigned int coordination;	// the total coordination of the ensemble

private:
  // prevent assignment
  ensemble& operator=(const ensemble&);
public:
  // ctor: sort the model_species list and insert into sorted species
  explicit ensemble(const model_species::seq& speciess);
  // ctor: copy
  ensemble(const ensemble&);
  // dtor: do nothing
  ~ensemble();

  // less than operator required for map
  bool operator<(const ensemble& right) const;
  // return number of model_species pointers in ensemble (surface species)
  unsigned int get_size() const;
  // return total coordination of ensemble
  unsigned int get_coordination() const;
  // return iterator to the beginning of the species sequence
  model_species::seq_citer begin() const;
  // return iterator to the end of the species sequences
  model_species::seq_citer end() const;
}; // end class ensemble

CH_END_NAMESPACE

#endif // not CH_MODEL_ENSEMBLE_H

/* $Id: ensemble.h,v 1.1.1.1 2004/11/25 20:24:08 banjo Exp $ */
