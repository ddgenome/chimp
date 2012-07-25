// -*- C++ -*-
// Mechanism information required by model solution.
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
#ifndef CH_MODEL_MECH_H
#define CH_MODEL_MECH_H 1

#include <string>
#include "except.h"
#include "mechanism.h"
#include "reaction.h"
#include "species.h"

// set namespace to avoid possible clashes
CH_BEGIN_NAMESPACE

// mechanism with model solution entries
class model_mechanism
{
  model_species::seq speciess;	// list of species
  model_reaction::seq reactions; // list of reactions
  species2model s2m;		// species to mode_species mapping

private:
  // prevent copy construction and assignment
  model_mechanism(const model_mechanism&);
  model_mechanism& operator=(const model_mechanism&);
public:
  // ctor: convert input into model-usable classes
  model_mechanism(const mechanism& mech)
    throw (bad_pointer); // model_reaction()
  // dtor: clean house
  ~model_mechanism();

  // return pointer to model species that corresponds to the base species,
  // return zero if not found
  model_species* get_species(species* sp);
  // return pointer to model species that corresponds to the name, zero if
  // not found
  model_species* get_species(const CH_STD::string& name);
  // return iterator to beginning of reaction list
  model_species::seq_citer species_seq_begin() const;
  // return iterator to beginning of reaction list
  model_species::seq_citer species_seq_end() const;
  // return iterator to beginning of reaction list
  model_reaction::seq_citer reaction_seq_begin() const;
  // return iterator to beginning of reaction list
  model_reaction::seq_citer reaction_seq_end() const;
  // set all species quantities to zero
  void zero_quantities();
}; // end class model_mechanism

CH_END_NAMESPACE

#endif // not CH_MODEL_MECH_H

/* $Id: model_mech.h,v 1.1.1.1 2004/11/25 20:24:05 banjo Exp $ */
