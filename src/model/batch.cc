// Batch reactor solution methods.
// Copyright (C) 2004 David Dooling <banjo@users.sourceforge.net>
//
// This file is part of CHIMP.
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

#include "batch.h"
#include "compare.h"
#include "constant.h"
#include "t_string.h"

// set namespace to avoid possible clashes
CH_BEGIN_NAMESPACE

// batch_reactor methods
// ctor: (default) call reactor ctor
batch_reactor::batch_reactor()
  : reactor()
{}

// ctor: copy
batch_reactor::batch_reactor(const batch_reactor& original)
  : reactor(original)
{}

// dtor: do nothing
batch_reactor::~batch_reactor()
{}

// batch_reactor private methods
#if 0				// don't need this right now
// see if inputs are consistent with rate and derivative units
CH_STD::string
batch_reactor::check_consistency()
  throw (bad_type)
{
  CH_STD::string error(reactor::check_consistency());
  // batch reactor specific requirements
  switch (size_type)
    {
    case Evolume:
      if (surface_species_present())
	{
	  if (get_volume() < 0.0e0)
	    {
	      error += "reactor volume; ";
	    }
	  if (get_sites() < 0.0e0)
	    {
	      error += "number of catalytic sites; ";
	    }
	}
      break;

    case Eweight:
      if (get_volume() < 0.0e0)
	{
	  error += "reactor volume; ";
	}
      if (get_weight() < 0.0e0)
	{
	  error += "catalyst weight; ";
	}
      if (surface_species_present())
	{
	  if (get_sites() < 0.0e0)
	    {
	      error += "number of catalytic sites; ";
	    }
	}
      break;

    case Esites:
      if (get_volume() < 0.0e0)
	{
	  error += "reactor volume; ";
	}
      if (get_sites() < 0.0e0)
	{
	  error += "number of catalytic sites; ";
	}
      break;

    default:			// throw an exception
      throw bad_type(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		     ":batch_reactor::check_consistency(): the denominator of "
		     "the rate expression has invalid units");
      break;
    }
  check_done = true;
  return error;
}
#endif // 0

// batch_reactor public methods
// parse batch reactor input
void
batch_reactor::parse(token_seq_citer& token_it, token_seq_citer end)
  throw (bad_input, bad_value, bad_type)
{
  // loop through input
  while (token_it != end)
    {
      // call the base class parser, returns on a token it does not recognize
      reactor::parse(token_it, end);
      // check for end of reactor input
      if (icompare(*token_it, "end") == 0)
	{
	  // make sure it is the end of reactor input
	  if (icompare(*++token_it, "reactor") != 0)
	    {
	      throw bad_input(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			      ":batch_reactor::parse(): syntax error in "
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
			  ":batch_reactor::parse(): syntax error in input "
			  "for reactor: unrecognized token: "
			  + *token_it);
	}
    }
  // end of file reached
  throw bad_input(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		  "batch_reactor::parse(): syntax error in input for "
		  "reactor: end of file reached while parsing input");
  // shouldn't get here
  return;
}

// create a copy of this batch reactor, return pointer to it
reactor*
batch_reactor::copy()
{
  return new batch_reactor(*this);
}

// modify derivative according to reactor design equations, return new value
double
batch_reactor::reactor_eqn(model_species* species)
  throw (bad_type)
{
  // set up variables to reduce function calls
  double rate(species->get_derivative());
  double yprime(0.0e0);

  // surface species equations
  if (species->get_surface_coordination() > 0U)
    {
      if (amount_type == Emoles)
	{
	  // multiply rate by Avogadro's number
	  // think of coverage as molecules/site (N_i/Ns)
	  rate *= constant::avogadro;
	}
      switch (size_type)
	{
	case Evolume:
	  // y_i' = r_i * V / Ns
	  yprime = rate * get_volume() / get_sites();
	  break;

	case Eweight:
	  // y_i' = r_i * W / Ns
	  yprime = rate * get_weight() / get_sites();
	  break;

	case Esites:
	  // no modifications: y_i' = r_i
	  break;

	default:		// throw an exception
	  throw bad_type(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			 ":batch_reactor::reactor_eqn(): the denominator of "
			 "the rate expression has invalid units");
	  break;
	}
    }
  // pressure equations
  else if (fluid_type == quantity::Epressure)
    {
      if (amount_type == Emolecules)
	{
	  // divide rate by Avogadro's number
	  rate /= constant::avogadro;
	}
      switch (size_type)
	{
	case Evolume:
	  // y_i' = r_i * RT + (p_i / T) dT/dt
	  yprime = rate * constant::r * get_temperature();
	  break;

	case Eweight:
	  // y_i' = r_i * RTW/V + (p_i / T) dT/dt
	  yprime = rate * constant::r * get_temperature() * get_weight()
	    / get_volume();
	  break;

	case Esites:
	  // y_i' = r_i * RTNs/V + (p_i / T) dT/dt
	  yprime = rate * constant::r * get_temperature() * get_sites()
	    / get_volume();
	  break;

	default:		// throw an exception
	  throw bad_type(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			 ":batch_reactor::reactor_eqn(): the denominator of "
			 "the rate expression has invalid units");
	  break;
	}
      // correct for any change in temperature
      // all pressure equations have a `+ (p_i / T) * dT/dt' term
      yprime += species->get_quantity(quantity::Epressure) / get_temperature()
	* get_heating_rate();
    }
  // concentration equations
  else if (fluid_type == quantity::Econcentration)
    {
      if (amount_type == Emolecules)
	{
	  // divide rate by Avogadro's number
	  rate /= constant::avogadro;
	}
      switch (size_type)
	{
	case Evolume:
	  // no modifications for Evolume: y_i' = r_i
	  break;

	case Eweight:
	  // y_i' = r_i * (W / V)
	  yprime = rate * get_weight() / get_volume();
	  break;

	case Esites:
	  // y_i' = r_i * (Ns / V)
	  yprime = rate * get_sites() / get_volume();
	  break;

	default:		// throw an exception
	  throw bad_type(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			 ":batch_reactor::reactor_eqn(): the denominator of "
			 "the rate expression has invalid units");
	}
    }
  else				// throw an exception
    {
      throw bad_type(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		     ":batch_reactor::reactor_eqn(): the units of the "
		     "derivative are invalid for a batch reactor");
    }
  // set the derivative to the calculated value
  species->set_derivative(yprime);
  // return the corrected derivative value
  return yprime;
}

// update an individual gas-phase species
void
batch_reactor::kmc_step(model_species* msp, double dx, double T0, double T1)
  throw (bad_type, bad_value)
{
  // pressure equations
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
		     ":batch_reactor::kmc_step(): the units of the "
		     "derivative are invalid for a batch reactor");
    }
  return;
}

CH_END_NAMESPACE

/* $Id: batch.cc,v 1.1.1.1 2004/11/25 20:24:08 banjo Exp $ */
