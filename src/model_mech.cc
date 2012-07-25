// Methods for mechanisms used for model soluion.
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

#include "model_mech.h"
#include <utility>

// set namespace to avoid possible clashes
CH_BEGIN_NAMESPACE

// model_mechanism class methods
// ctor: convert input into model-usable classes
model_mechanism::model_mechanism(const mechanism& mech)
  throw (bad_pointer)
  : speciess(), reactions(), s2m()
{
  // make sure the sequences are allocated big enough
  speciess.reserve(mech.get_total_species());
  reactions.reserve(mech.get_total_reactions());
  // create list of model_species from list of species
  for (species::map_citer it(mech.species_map_begin());
       it != mech.species_map_end(); ++it)
    {
      // make a model_species* from a species
      model_species* ms_tmp = new model_species(*it->second);
      // add pointer to model_species::seq
      speciess.push_back(ms_tmp);
      // add pointers to mapping
      s2m.insert(CH_STD::make_pair(it->second, ms_tmp));
    }
  // create model_reactions from list of reactions
  for (reaction::seq_citer it(mech.reaction_seq_begin());
       it != mech.reaction_seq_end(); ++it)
    {
      // create model_reaction pointer and add it to list
      reactions.push_back(new model_reaction(**it, s2m));
    }
}

// dtor: delete everything we created
model_mechanism::~model_mechanism()
{
  // delete all the model_reactions
  for (model_reaction::seq_iter it(reactions.begin()); it != reactions.end();
       ++it)
    {
      delete *it;
      *it = 0;
    }
  // delete all the model_species
  for (model_species::seq_iter it(speciess.begin()); it != speciess.end(); ++it)
    {
      delete *it;
      *it = 0;
    }
}

// model_mechanism public methods
// return a pointer to the model_species which corresponds to to given species
// return zero if not found
model_species*
model_mechanism::get_species(species* sp)
{
  // try to find the model_species
  species2model_citer s2m_it(s2m.find(sp));
  if (s2m_it == s2m.end())
    {
      // not found
      return 0;
    }
  // return the model_species pointer
  return s2m_it->second;
}

// return pointer to model species that corresponds to the name, zero if
// not found
model_species*
model_mechanism::get_species(const CH_STD::string& name)
{
  // this might be kind of slow
  for (model_species::seq_citer it(speciess.begin()); it != speciess.end();
       ++it)
    {
      if ((*it)->get_name() == name)
	{
	  return *it;
	}
    }
  // not found
  return 0;
}

// return iterator to beginning of species list
model_species::seq_citer
model_mechanism::species_seq_begin() const
{
  return speciess.begin();
}

// return iterator to beginning of species list
model_species::seq_citer
model_mechanism::species_seq_end() const
{
  return speciess.end();
}

// return iterator to beginning of reaction list
model_reaction::seq_citer
model_mechanism::reaction_seq_begin() const
{
  return reactions.begin();
}

// return iterator to beginning of reaction list
model_reaction::seq_citer
model_mechanism::reaction_seq_end() const
{
  return reactions.end();
}

// set all the model_species values to zero
void
model_mechanism::zero_quantities()
{
  for (model_species::seq_iter it(speciess.begin()); it != speciess.end(); ++it)
    {
      (*it)->zero_quantity();
    }
  return;
}

CH_END_NAMESPACE

/* $Id: model_mech.cc,v 1.1.1.1 2004/11/25 20:24:05 banjo Exp $ */
