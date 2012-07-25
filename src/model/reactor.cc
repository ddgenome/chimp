// Reactor configuration and solution methods.
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

#include "reactor.h"
#include <cstdlib>
#include <cmath>
#include <utility>
#include "batch.h"
#include "compare.h"
#include "constant.h"
#include "cstr.h"
#include "pfr.h"
#include "precision.h"
#include "reaction.h"
#include "t_string.h"

// set namespace to avoid possible clashes
CH_BEGIN_NAMESPACE

// FIXME: fix the consistency checks, if we can update manual
// reactor methods
// ctor: (default) set instance variables to default values
reactor::reactor()
  : temperature(2.7315e2), heating_rate(0.0e0), pressure(1.0e0),
    volume(1.0e0), weight(0.0e0), sites(0.0e0),
    amount_type(Emoles), size_type(Evolume),
    fluid_type(quantity::Econcentration)
{}

// ctor: copy
reactor::reactor(const reactor& o)
  : temperature(o.temperature), heating_rate(o.heating_rate),
    pressure(o.pressure), volume(o.volume), weight(o.weight), sites(o.sites),
    amount_type(o.amount_type), size_type(o.size_type),
    fluid_type(o.fluid_type)
{}

// dtor: do nothing
reactor::~reactor()
{}

// reactor class private methods
// set how the amount is expressed in the rate expr, return old value
CH_STD::string
reactor::set_rate_amount_type(const CH_STD::string& amount_type_)
  throw (bad_type)
{
  // determine old type
  CH_STD::string old_type;
  switch (amount_type)
    {
    case Emoles:
      old_type = "moles";
      break;

    case Emolecules:
      old_type = "molecules";
      break;

    default:			// if it gets here something is very wrong
      throw bad_type(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		     ":reactor::set_rate_amount_type(): the old amount_type "
		     "is invalid; this is a bug, please report");
      break;
    }
  // do a case insensitive comparison
  if (icompare(amount_type_, "molecule", 8) == 0)
    {
      amount_type = Emolecules;
    }
  else if (icompare(amount_type_, "mole", 4) == 0)
    {
      amount_type = Emoles;
    }
  else				// throw an exception
    {
      throw bad_type(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		     ":reactor::set_rate_amount_type(): the amount_type "
		     "specified (" + amount_type_ + ") is invalid");
    }
  return old_type;
}

// set how the reactor size is expressed in the rate expr, return old value
CH_STD::string
reactor::set_rate_size_type(const CH_STD::string& size_type_)
  throw (bad_type)
{
  // determine old type
  CH_STD::string old_type;
  switch (size_type)
    {
    case Evolume:
      old_type = "volume";
      break;

    case Eweight:
      old_type = "weight";
      break;

    case Esites:
      old_type = "sites";
      break;

    default:			// something is very wrong
      throw bad_type(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		     ":reactor::set_rate_size_type(): the old size_type is "
		     "invalid; this is a bug, please report");
      break;
    }
  // only test the first character
  try
    {
      switch(size_type_.at(0))	// might throw out_of_range
	{
	case 'v': case 'V':
	  size_type = Evolume;
	  break;
	case 'w': case 'W':
	  size_type = Eweight;
	  break;
	case 's': case 'S':
	  size_type = Esites;
	  break;
	default:			// throw an exception
	  throw bad_type(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			 ":reactor::set_rate_size_type(): the size_type "
			 "specified (" + size_type_ + ") is invalid");
	}
    }
  catch (CH_STD::out_of_range& e)
    {
      throw bad_type(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		     ":reactor::set_rate_size_type(): no size type was "
		     "specified (null string): " + e.what());
    }
  return old_type;
}

// set the quantity type for fluid derivatives, return old value
quantity::type
reactor::set_fluid_type(const CH_STD::string& fluid_type_)
  throw (bad_type)
{
  // save the old value
  quantity::type old(fluid_type);
  // get the new value
  fluid_type = quantity::get_type(fluid_type_);
  // make sure it is not flow
  if (fluid_type == quantity::Eflow)
    {
      throw bad_type(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		     ":reactor::set_fluid_type(): can not set fluid type to "
		     "flow");
    }
  return old;
}

// reactor protected methods
// set the temperature, return old
double
reactor::set_temperature(double temperature_)
  throw (bad_value)
{
  if (temperature < - precision::get().get_double())
    {
      // throw an exception
      throw bad_value(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		      ":reactor::set_temperature(): absolute temperature "
		      "cannot be < 0.0e0");
    }
  double old(temperature);
  temperature = temperature_;
  return old;
}

// set the heating rate (temperature/time), return old
double
reactor::set_heating_rate(double  heating_rate_)
{
  double old(heating_rate);
  heating_rate = heating_rate_;
  return old;
}

// set the pressure, return old
double
reactor::set_pressure(double pressure_)
  throw (bad_value)
{
  if (pressure_ < - precision::get().get_pressure())
    {
      // throw an exception
      throw bad_value(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		      ":reactor::set_pressure(): absolute pressure cannot be "
		      "< 0.0e0");
    }
  double old(pressure);
  pressure = pressure_;
  return old;
}

// add INCREMENT to total PRESSURE, return new total
double
reactor::add_to_pressure(double increment)
  throw (bad_value)
{
  set_pressure(get_pressure() + increment);
  return get_pressure();
}

// set the volume of the reactor, return old
double
reactor::set_volume(double volume_)
  throw (bad_value)
{
  if (volume_ < - precision::get().get_double())
    {
      // throw an exception
      throw bad_value(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		      ":reactor::set_volume(): reactor volume cannot "
		      "be < 0.0e0");
    }
  double old(volume);
  volume = volume_;
  return old;
}

// set the catalyst weight, return old
double
reactor::set_weight(double weight_)
  throw (bad_value)
{
  if (weight_ < - precision::get().get_double())
    {
      // throw an exception
      throw bad_value(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		      ":reactor::set_weight(): catalyst weight cannot be < "
		      "0.0e0");
    }
  double old(weight);
  weight = weight_;
  return old;
}

// set the number of catalytic sites, return old
double
reactor::set_sites(double sites_)
  throw (bad_value)
{
  if (sites_ < - precision::get().get_coverage())
    {
      // throw an exception
      throw bad_value(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		      ":reactor::set_sites(): number of catalytic sites "
		      "cannot be < 0.0e0");
    }
  double old(sites);
  sites = sites_;
  return old;
}

// individual species kmc equation, return unsigned change in quantity
// molecules is how many molecules are reacting
double
reactor::kmc_reaction(model_species* msp) const
{
  // see if it is a surface species
  if (msp->get_surface_coordination() > 0U)
    {
      // return the change in coverage
      return 1.0e0 / get_sites();
    }
  // gas-phase species
  else if (fluid_type == quantity::Epressure)
    {
      // return relative change in pressure
      // dp = (dN)kT/V
      return constant::k * get_temperature() / get_volume();
    }
  // concentration equations
  else if (fluid_type == quantity::Econcentration)
    {
      // add MOLECULES to the concentration
      // dc = dN/(Na * V)
      return 1.0e0 / (constant::avogadro * get_volume());
    }
  // else throw an exception
  throw bad_type(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		 ":reactor::kmc_reaction(): the units of the "
		 "derivative are invalid for reactor");
  // should not get here
  return 0.0e0;
}

// reactor public methods
// parse reactor input
void
reactor::parse(token_seq_citer& token_it, token_seq_citer end)
  throw (bad_input, bad_value, bad_type)
{
  // loop through input
  while (token_it != end)
    {
      // lots of stuff to check
      if (icompare(*token_it, "temperature") == 0)
	{
	  set_temperature(CH_STD::atof((++token_it)->c_str()));
	  // increment one further
	  ++token_it;
	  continue;		// where ()
	}
      else if (icompare(*token_it, "heating_rate") == 0)
	{
	  set_heating_rate(CH_STD::atof((++token_it)->c_str()));
	  // increment one further
	  ++token_it;
	  continue;		// where ()
	}
      else if (icompare(*token_it, "pressure") == 0)
	{
	  set_pressure(CH_STD::atof((++token_it)->c_str()));
	  // increment one further
	  ++token_it;
	  continue;		// where ()
	}
      else if (icompare(*token_it, "volume") == 0)
	{
	  set_volume(CH_STD::atof((++token_it)->c_str()));
	  // increment one further
	  ++token_it;
	  continue;		// where ()
	}
      else if (icompare(*token_it, "weight") == 0
	       || icompare(*token_it, "catalyst_weight") == 0)
	{
	  set_weight(CH_STD::atof((++token_it)->c_str()));
	  // increment one further
	  ++token_it;
	  continue;		// where ()
	}
      else if (icompare(*token_it, "sites") == 0
	       || icompare(*token_it, "catalyst_sites") == 0)
	{
	  set_sites(CH_STD::atof((++token_it)->c_str()));
	  // increment one further
	  ++token_it;
	  continue;		// where ()
	}
      else if (icompare(*token_it, "rate_numerator") == 0)
	{
	  // set the quantity type for fluid species
	  set_rate_amount_type(*++token_it);
	  // increment one further
	  ++token_it;
	  continue;		// where ()
	}
      else if (icompare(*token_it, "rate_denominator") == 0)
	{
	  // set the quantity type for fluid species
	  set_rate_size_type(*++token_it);
	  // increment one further
	  ++token_it;
	  continue;		// where ()
	}
      else if (icompare(*token_it, "fluid_quantity") == 0)
	{
	  // set the quantity type for fluid species
	  set_fluid_type(*++token_it);
	  // increment one further
	  ++token_it;
	  continue;		// where ()
	}
      else			// unknown token
	{
	  // return it to derived class parser
	  return;
	}
    }
  // end of file reached
  throw bad_input(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		  "reactor::parse(): syntax error in input for "
		  "reactor: end of file reached while parsing input");
  // shouldn't get here
  return;
}

// create a new reactor of appropriate type, return pointer or zero
// this needs to be updated whenever a new class is derived from reactor
// default type = "batch"
reactor*
reactor::new_reactor(const CH_STD::string& type)
{
  // see what type of reactor they want
  if (icompare(type, "batch") == 0)
    {
      return new batch_reactor();
    }
  else if (icompare(type, "pfr") == 0)
    {
      return new pfr();
    }
  else if (icompare(type, "cstr") == 0)
    {
      return new cstr();
    }
  // else
  return 0;
}

// initialize the reactor and its solution variables
void
reactor::initialize(model_species::seq_citer species_begin,
		    model_species::seq_citer species_end)
  throw (bad_type, bad_value)
{
  // set the quantity type we want to use in rate calculations
  model_reaction::set_amount_type(fluid_type);
  // find the total pressure
  double total_pressure(0.0e0);
  // loop through all the species in the model
  for (model_species::seq_citer it(species_begin); it != species_end; ++it)
    {
      // make sure it is not a surface species
      if ((*it)->get_surface_coordination() < 1U)
	{
	  total_pressure += (*it)->get_quantity(quantity::Epressure);
	}
    }
  // set the pressure to proper value
  set_pressure(total_pressure);
  return;
}

// set stuff up for a kmc run, return scale
double
reactor::kmc_initialize(unsigned int kmc_sites, double scale)
  throw (bad_value)
{
  // see if reactor sites was set in input (overrides scale)
  if (get_sites() > precision::get().get_double())
    {
      // see if a lattice is being used
      if (kmc_sites > 0U)
	{
	  // return the scaling factor to use
	  return get_sites() / kmc_sites;
	}
      else			// no lattice
	{
	  // scale unchanged
	  return scale;
	}
    }
  // else reactor sites not set
  // use use kmc scale to calculate the number of actual sites
  set_sites(kmc_sites * scale);	// could be zero
  // scale unchanged
  return scale;
}

// return the TEMPERATURE
double
reactor::get_temperature() const
{
  return temperature;
}

// return the heating rate (temperature/time), return old
double
reactor::get_heating_rate() const
{
  return heating_rate;
}

// return the pressure
double
reactor::get_pressure() const
{
  return pressure;
}

// return the volume of the reactor
double
reactor::get_volume() const
{
  return volume;
}

// return catalyst weight
double
reactor::get_weight() const
{
  return weight;
}

// return number of catalytic sites
double
reactor::get_sites() const
{
  return sites;
}

// return the type of quantity used for fluids
quantity::type
reactor::get_fluid_type() const
{
  return fluid_type;
}

// loop through species and modify derivatives using reactor design eqns
void
reactor::reactor_eqn(const model_species::seq& species_list)
  throw (bad_type)
{
#if 0				// do not need this (maybe later)
  // make sure all required inputs have been specified
  if (!check_done)
    {
      CH_STD::string error(check_consistency());
      if (error.length() > 0)
	{
	  // inputs are not consistent, throw an exception
	  throw bad_type(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			 ":reactor::reactor_eqn(): you have not given the "
			 "reactor information needed for the design "
			 "equations: " + error);
	}
    }
#endif // 0
  for (model_species::seq_citer it(species_list.begin());
       it != species_list.end(); ++it)
    {
      reactor_eqn(*it);
    }
  return;
}

// adjust rate according to units used in reactor to 1/time
// (actually more like molecule/time)
// default original_rate = 1.0e0
double
reactor::kmc_rate(double original_rate)
  throw (bad_type)
{
  if (amount_type == Emoles)
    {
      original_rate *= constant::avogadro;
    }
  if (size_type == Evolume)
    {
      original_rate *= get_volume();
    }
  else if (size_type == Eweight)
    {
      original_rate *= get_weight();
    }
  else if (size_type == Esites)
    {
      original_rate *= get_sites();
    }
  else				// corruption!
    {
      throw bad_type(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		     ":reactor::kmc_rate(): the denominator of "
		     "the rate expression has invalid units");
    }
  return original_rate;
}

// make sure reaction is possible, given current species amounts
bool
reactor::kmc_quantities(const stoich_map& net, double molecules) const
  throw (bad_type)
{
  // loop through each species and determine if there is enough
  for (stoich_map_citer it(net.begin()); it != net.end(); ++it)
    {
      // see how many molecules will be transformed
      double change(molecules * it->second.get_coefficient());
      // determine if this species is depleted by the reaction
      if (change < 0.0e0)
	{
	  // make sure quantity will not be < zero if rxn performed
	  if ((static_cast<model_species*>(it->first)->get_quantity(fluid_type)
	       < (- change
		  * kmc_reaction(static_cast<model_species*>(it->first)))))
	    {
	      return false;
	    }
	}
    }
  return true;
}

// reactor equation for kinetic Monte Carlo
// molecules is the number of gas phase molecules to change per reactant
// and product
void
reactor::kmc_reaction(const model_species::seq& reactants,
		      const model_species::seq& products, double molecules)
  throw (bad_type, bad_value)
{
  // loop through reactants
  for (model_species::seq_citer sp_it(reactants.begin());
       sp_it != reactants.end(); ++sp_it)
    {
      // remove the appropriate amount from the quantity
      (*sp_it)->add_to_quantity(fluid_type,
				- molecules * kmc_reaction(*sp_it));
    }
  // loop through products
  for (model_species::seq_citer sp_it(products.begin());
       sp_it != products.end(); ++sp_it)
    {
      // add the appropriate amount to the quantity
      (*sp_it)->add_to_quantity(fluid_type, molecules * kmc_reaction(*sp_it));
    }
  return;
}

// update everything given the time increment
void
reactor::kmc_step(const model_species::seq_citer species_begin,
		  const model_species::seq_citer species_end, double dx)
  throw (bad_type, bad_value)
{
  // calculate new temperature (if necessary)
  double T0(get_temperature());
  double T1(T0);
  if (CH_STD::fabs(get_heating_rate()) > precision::get().get_double())
    {
      // set new temperature
      double T1(T0 + get_heating_rate() * dx);
      set_temperature(T1);
    }
  // loop through all species in the model
  for (model_species::seq_citer sp_it(species_begin);
       sp_it != species_end; ++sp_it)
    {
      // this is only for non-surface species
      if ((*sp_it)->get_surface_coordination() < 1U)
	{
	  // update the pressure/concentration for the temperature change
	  kmc_step(*sp_it, dx, T0, T1);
	}
    }
  return;
}

// flow_reactor methods
// ctor: (default) call reactor ctor and set flow to zero
flow_reactor::flow_reactor()
  : reactor(), flow(0.0e0)
{}

// ctor: copy
flow_reactor::flow_reactor(const flow_reactor& original)
  : reactor(original), flow(original.flow)
{}

// dtor: do nothing
flow_reactor::~flow_reactor()
{}

// flow_reactor public methods
// parse flow reactor input
void
flow_reactor::parse(token_seq_citer& token_it, token_seq_citer end)
  throw (bad_input, bad_value, bad_type)
{
  // loop through input
  while (token_it != end)
    {
      // call the base class parser, returns on a token it does not recognize
      reactor::parse(token_it, end);
      // check for flow rate input
      if (icompare(*token_it, "flow") == 0)
	{
	  // set the total flow rate with next token
	  set_flow(CH_STD::atof((++token_it)->c_str()));
	  // make sure flow is positive (initially)
	  if (get_flow() < - precision::get().get_flow())
	    {
	      throw bad_input(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			      ":flow_reactor::parse(): syntax error in "
			      "input for reactor: total flow is non-positive: "
			      + t_string(get_flow()));
	    }
	  // increment one further
	  ++token_it;
	  continue;		// where ()
	}
      // check for end of reactor input
      else if (icompare(*token_it, "end") == 0)
	{
	  // make sure it is the end of reactor input
	  if (icompare(*++token_it, "reactor") != 0)
	    {
	      throw bad_input(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			      ":flow_reactor::parse(): syntax error in "
			      "input for reactor: corresponding end token "
			      "does not end a reactor: " + *token_it);
	    }
	  // increment one further
	  ++token_it;
	  // return to caller
	  return;
	}
      else
	{
	  throw bad_input(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			  ":flow_reactor::parse(): syntax error in input "
			  "for reactor: unrecognized token: "
			  + *token_it);
	}
    }
  // end of file reached
  throw bad_input(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		  "flow_reactor::parse(): syntax error in input for "
		  "reactor: end of file reached while parsing input");
  // shouldn't get here
  return;
}

// initialize the reactor and its solution variables
void
flow_reactor::initialize(model_species::seq_citer species_begin,
			 model_species::seq_citer species_end)
  throw (bad_type, bad_value)
{
  // call base class to set up pressures
  reactor::initialize(species_begin, species_end);
  // find the total flow
  double total_flow(0.0e0);
  // loop through all the species in the model
  for (model_species::seq_citer it(species_begin); it != species_end; ++it)
    {
      // make sure it is not a surface species
      if ((*it)->get_surface_coordination() < 1U)
	{
	  total_flow += (*it)->get_quantity(quantity::Eflow);
	}
    }
  // see if any flows were set
  if (CH_STD::fabs(total_flow) > precision::get().get_flow())
    {
      // set the flow to proper value
      set_flow(total_flow);
    }
  else				// no individual flows set
    {
      // set individual flows from total flow
      // determine what to divide by to get individual flows
      double total(0.0e0);
      if (fluid_type == quantity::Epressure)
	{
	  total = get_pressure();
	}
      else if (fluid_type == quantity::Econcentration)
	{
	  // must calculate total concentration
	  // loop through the species and get their concentrations
	  for (model_species::seq_citer it(species_begin); it != species_end;
	       ++it)
	    {
	      // make sure it is not a surface species
	      if ((*it)->get_surface_coordination() < 1U)
		{
		  total += (*it)->get_quantity(quantity::Econcentration);
		}
	    }
	}
      // loop through the species and set their initial flow rates
      for (model_species::seq_citer it(species_begin); it != species_end; ++it)
	{
	  // make sure it is not a surface species
	  if ((*it)->get_surface_coordination() < 1U)
	    {
	      // convert pressure or concentration to flow
	      (*it)->set_quantity(quantity::Eflow,
				  ((*it)->get_quantity(fluid_type)
				   * get_flow() / total));
	    }
	}
    }
  return;
}

// return the total molar flow rate
double
flow_reactor::get_flow() const
{
  return flow;
}

// set the total molar flow rate, return old value
double
flow_reactor::set_flow(double flow_)
{
  double old(flow);
  flow = flow_;
  return old;
}

// add INCREMENT to total molar FLOW, return new total
double
flow_reactor::add_to_flow(double increment)
{
  return flow += increment;
}

CH_END_NAMESPACE

/* $Id: reactor.cc,v 1.1.1.1 2004/11/25 20:24:08 banjo Exp $ */
