// Continuous stirred tank reactor solution methods.
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

#include "cstr.h"
#include <utility>
#include "constant.h"
#include "precision.h"
#include "t_string.h"

// set namespace to avoid possible clashes
CH_BEGIN_NAMESPACE

// cstr methods
// ctor: (default) call flow_reactor ctor
cstr::cstr()
  : flow_reactor(), flow_in()
{}

cstr::cstr(const cstr& original)
  : flow_reactor(original), flow_in(original.flow_in)
{}

// dtor: do nothing
cstr::~cstr()
{}

// cstr public methods
// create a copy of this cstr, return pointer to it
reactor*
cstr::copy()
{
  return new cstr(*this);
}

// initialize the reactor and its solution variables
void
cstr::initialize(model_species::seq_citer species_begin,
		 model_species::seq_citer species_end)
  throw (bad_type, bad_value)
{
  // call base class initializer
  flow_reactor::initialize(species_begin, species_end);
  // loop through the species and set their initial flow rates
  for (model_species::seq_citer it(species_begin); it != species_end; ++it)
    {
      // make sure it is not a surface species
      if ((*it)->get_surface_coordination() < 1U)
	{
	  // get this species flow rate
	  double species_flow((*it)->get_quantity(quantity::Eflow));
	  // see if flow is non-zero
	  if (species_flow > precision::get().get_flow())
	    {
	      // insert this species and its flow rate into initial flow map
	      flow_in.insert(CH_STD::make_pair(*it, species_flow));
	    }
	}
    }
  return;
}

// reactor equations for CSTR
double
cstr::reactor_eqn(model_species* species)
  throw (bad_type)
{
  // set up variables to reduce function calls
  double rate(species->get_derivative());
  double yprime(0.0e0);

  // FIXME: put in reactor design equations
  yprime = rate;

  // set the derivative to the calculated value
  species->set_derivative(yprime);
  // return the corrected derivative value
  return yprime;
}

// update everything given the time increment
void
cstr::kmc_step(const model_species::seq_citer species_begin,
	       const model_species::seq_citer species_end, double dt)
  throw (bad_type, bad_value)
{
  // call the base class method (which call cstr::kmc_step() below)
  reactor::kmc_step(species_begin, species_end, dt);
  if (fluid_type == quantity::Epressure)
    {
      // loop through all the species in the model to calculate total pressure
      double total_pressure(0.0e0);
      for (model_species::seq_citer it(species_begin); it != species_end; ++it)
	{
	  // make sure it is not a surface species
	  if ((*it)->get_surface_coordination() < 1U)
	    {
	      total_pressure += (*it)->get_quantity(quantity::Epressure);
	    }
	}
      // get the ratio of what pressure should be to sum of partial pressures
      double pressure_ratio(get_pressure() / total_pressure);
      // reset the total flow
      set_flow(0.0e0);
      // get proper constant (k or R) to use
      double gas_constant(constant::k);
      // amount type has same units as the numerator of flow
      if (amount_type == Emoles)
	{
	  gas_constant = constant::r;
	}
      // loop through and adjust each species
      for (model_species::seq_citer it(species_begin); it != species_end; ++it)
	{
	  // make sure it is not a surface species
	  if ((*it)->get_surface_coordination() < 1U)
	    {
	      // adjust each species pressure through out flow
	      // get current pressure
	      double p_current((*it)->get_quantity(quantity::Epressure));
	      // calculate what pressure should be
	      double p_goal(p_current * pressure_ratio);
	      // set the pressure to what it should be
	      (*it)->set_quantity(quantity::Epressure, p_goal);
	      // calculate the out flow which will yield the goal pressure
	      // F = V dp / RT / dt
	      double out_flow((p_current - p_goal) * get_volume()
			      / (gas_constant * get_temperature()) / dt);
	      // set it
	      (*it)->set_quantity(quantity::Eflow, out_flow);
	      // increment the total flow with it
	      add_to_flow(out_flow);
	    }
	}
    }
  // concentration equations
  else if (fluid_type == quantity::Econcentration)
    {
      // get the total concentration
      double total_concentration(0.0e0);
      for (model_species::seq_citer it(species_begin); it != species_end; ++it)
	{
	  // make sure it is not a surface species
	  if ((*it)->get_surface_coordination() < 1U)
	    {
	      total_concentration +=
		(*it)->get_quantity(quantity::Econcentration);
	    }
	}
      // loop through and set each species out flow
      for (model_species::seq_citer it(species_begin); it != species_end; ++it)
	{
	  // make sure it is not a surface species
	  if ((*it)->get_surface_coordination() < 1U)
	    {
	      (*it)->set_quantity(quantity::Eflow,
				  ((*it)->get_quantity(quantity::Econcentration)
				   * get_flow() / total_concentration));
	    }
	}
    }
  else				// invalid
    {
      throw bad_type(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		     ":cstr::kmc_step(): the units of the "
		     "derivative are invalid for a CSTR");
    }
  return;
}

// update an individual gas-phase species
void
cstr::kmc_step(model_species* msp, double dt, double T0, double T1)
  throw (bad_type, bad_value)
{
  // flow in the correct amount
  flow_map_iter f_in(flow_in.find(msp));
  // see if this species has in input flow
  if (f_in != flow_in.end())
    {
      // determine what to add the in flow to
      if (fluid_type == quantity::Epressure)
	{
	  // get proper constant (k or R) to use
	  // (amount_type should also be the units of flow)
	  double gas_constant(constant::k);
	  if (amount_type == Emoles)
	    {
	      gas_constant = constant::r;
	    }
	  // increment the pressure
	  // p1 = p0 + (F * dt) RT/V
	  f_in->first->add_to_quantity(quantity::Epressure,
				       (f_in->second * dt * gas_constant
					* get_temperature() / get_volume()));
	}
      else if (fluid_type == quantity::Econcentration)
	{
	  // determine if we have to convert to moles
	  double na(1.0e0);
	  if (amount_type == Emolecules)
	    {
	      na = constant::avogadro;
	    }
	  // increment the concentration
	  // c1 = c0 + (F * dt)/V
	  f_in->first->add_to_quantity(quantity::Econcentration,
				       (f_in->second * dt /
					(na * get_volume())));
	}
      // no else, the check later will catch a bad fluid type
    }
  // update pressure due to any temperature change
  if (fluid_type == quantity::Epressure)
    {
      // increment pressure for any temperature change
      msp->set_quantity(quantity::Epressure,
			msp->get_quantity(quantity::Epressure) * T1 / T0);
    }
  // don't need to do anything for concentration
  else if (fluid_type != quantity::Econcentration)
    {
      throw bad_type(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		     ":cstr::kmc_step(): the units of the "
		     "derivative are invalid for a CSTR");
    }
  return;
}

CH_END_NAMESPACE

/* $Id: cstr.cc,v 1.1.1.1 2004/11/25 20:24:08 banjo Exp $ */
