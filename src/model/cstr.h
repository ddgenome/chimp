// -*- C++ -*-
// Continuous stirred reactor tank solution information.
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
#ifndef CH_MODEL_CSTR_H
#define CH_MODEL_CSTR_H 1

#include <map>
#include "except.h"
#include "reactor.h"
#include "species.h"

// set namespace to avoid possible clashes
CH_BEGIN_NAMESPACE

// class for solution of continuous stirred-tank reactor problems
class cstr : public flow_reactor
{
  // typedefs
  typedef CH_STD::map<model_species*,double> flow_map;
  typedef flow_map::iterator flow_map_iter;
  typedef flow_map::const_iterator flow_map_citer;

private:
  flow_map flow_in;		// input flow rates (units of amount_type)

public:
  // ctor: (default) call flow_reactor ctor
  cstr();
  // ctor: copy
  explicit cstr(const cstr& original);
  // dtor: do nothing
  virtual ~cstr();

  // create a copy of a cstr, return pointer to it
  virtual reactor* copy();
  // initialize the reactor and its solution variables
  virtual void initialize(model_species::seq_citer species_begin,
			  model_species::seq_citer species_end)
    throw (bad_type, bad_value); // flow_reactor::initialize(),
				// model_species::get_quantity()
  // modify derivative according to the reactor design equations
  virtual double reactor_eqn(model_species* species)
    throw (bad_type);
  // update everything given the time increment
  virtual void kmc_step(const model_species::seq_citer species_begin,
			const model_species::seq_citer species_end, double dx)
    throw (bad_type, bad_value); // this, reactor::kmc_step(),
				// model_species::get_quantity(),
				// model_species::set_quantity()
  // update an individual gas-phase species
  virtual void kmc_step(model_species* msp, double dx, double T0, double T1)
    throw (bad_type, bad_value); // this, model_species::add_to_quantity(),
				// model_species::set_quantity()
}; // end class cstr

CH_END_NAMESPACE

#endif // not CH_MODEL_CSTR_H

/* $Id: cstr.h,v 1.1.1.1 2004/11/25 20:24:08 banjo Exp $ */
