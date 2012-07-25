// Methods for setting up and executing model solutions.
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

#include <cmath>
#include "integrate.h"
#include "compare.h"
#include "kmc.h"
#include "quantity.h"
#include "precision.h"
#include "reaction.h"
#include "reactor.h"
#include "t_string.h"

// set namespace to avoid possible clashes
CH_BEGIN_NAMESPACE

// integrator class methods
// ctor: (default) set pointers to zero
integrator::integrator()
  : x0(0.0e0), mech(0), state_info(0), out_file(0)
{
  // create default initial state
  state_info = new state();
}

// ctor: copy, create copy of state
integrator::integrator(const integrator& original)
  throw (bad_pointer)
  : x0(original.x0), mech(0), state_info(0), out_file(0)
{
  // create a copy of the given state
  state_info = new state(*original.state_info);
}

// dtor: delete the state pointer
integrator::~integrator()
{
  // delete state pointer
  delete state_info;
}

// integrator private methods
// set up initial values
void
integrator::initial_values()
  throw (bad_input)
{
  // see if we should just use the final values from previous integration
  if (state_info->get_previous_values())
    {
      // these values should already be set
      return;
    }
  // reset all quantities
  mech->zero_quantities();
  // loop through the list in state_info
  for (init_val_map_citer it(state_info->init_val_begin());
       it != state_info->init_val_end(); ++it)
    {
      // get model_species which corresponds to the species
      model_species* ms(mech->get_species(it->first));
      if (ms == 0)
	{
	  // the species does not have a correspondence in this model
	  throw bad_input(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			  ":integrator::initial_values(): one of the species("
			  + it->first->get_name() + ") to which an initial "
			  "value was assigned does not exist in this model");
	}
      // set the quantity for the model_species
      ms->set_quantity(it->second);
    }
  return;
}

// integrator protected methods
// virtual method to initialize integrator (must be called from derived class)
void
integrator::initialize()
  throw (bad_pointer, bad_input, bad_value, bad_type, bad_request)
{
  // make sure mech got set
  if (mech == 0)
    {
      throw bad_pointer(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			":integrator::initialize(): asking to initialize but "
			"the model mechanism pointer has not been set");
    }
  // initialize the reactor
  state_info->initialize(mech->species_seq_begin(), mech->species_seq_end());
  return;
}

// set the initial value of the independent variable, return old
// default x0_ = 0.0e0
double
integrator::set_initial(double x0_)
{
  double old(x0);
  x0 = x0_;
  return old;
}

// output the current output point and its values
void
integrator::output_header()
{
  // output the current output point column header
  *out_file << "# x";
  // loop through the species and output their names
  for (model_species::seq_citer sp_it(mech->species_seq_begin());
       sp_it != mech->species_seq_end(); ++sp_it)
    {
      *out_file << '\t' << (*sp_it)->get_name();
    }
  // see if we should output the flow
  flow_reactor* fr = dynamic_cast<flow_reactor*>(state_info->get_reactor());
  // if cast was successful
  if (fr != 0)
    {
      // output the total flow rate
      *out_file << "\tflow";
    }
  // see if we should output the temperature
  if (CH_STD::fabs(state_info->get_reactor()->get_heating_rate())
      > precision::get().get_double())
    {
      *out_file << "\ttemperature";
    }
  return;
}

// output the current output point and its values to the given stream
void
integrator::output(double x)
  throw (bad_type, bad_request, bad_value)
{
  output(x, *out_file);
  return;
}

// output the current output point and its values (no new line) to the stream
void
integrator::output(double x, CH_STD::ostream& output_stream)
  throw (bad_type, bad_request, bad_value)
{
  // output the current output point
  output_stream << x;
  // get the interesting amount type
  quantity::type type(model_reaction::get_amount_type());
  // loop through the species and output their values
  for (model_species::seq_citer sp_it(mech->species_seq_begin());
       sp_it != mech->species_seq_end(); ++sp_it)
    {
      output_stream << '\t' << (*sp_it)->get_quantity(type);
    }
  // see if we should output the flow
  flow_reactor* fr = dynamic_cast<flow_reactor*>(state_info->get_reactor());
  // if cast was successful
  if (fr != 0)
    {
      // output the total flow rate
      output_stream << '\t' << fr->get_flow();
    }
  // see if we should output the temperature
  if (CH_STD::fabs(state_info->get_reactor()->get_heating_rate())
      > precision::get().get_double())
    {
      output_stream << '\t' << state_info->get_reactor()->get_temperature();
    }
  return;
}

// integrator public methods
// create a new integrator of the given type, return pointer or zero
// must be changed when new classes are derived from integrator
// default type = "kmc"
integrator*
integrator::new_integrator(const CH_STD::string& type)
{
  // see what type they want
  if (icompare(type, "kmc") == 0)
    {
      return new kmc();
    }
  // else
  return 0;
}

// return pointer to the state variable
state*
integrator::get_state() const
{
  return state_info;
}

// set the output stream
void
integrator::set_output(CH_STD::ofstream* out_file_)
  throw (bad_file)
{
  // make sure given stream is ok and open
  if (*out_file_ && out_file_->is_open())
    {
      // attach our output stream to the one given (can't assign)
      out_file = out_file_;
    }
  else				// something wrong
    {
      throw bad_file(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		     ":integrator::set_output(): the given output file is "
		     "either corrupt or has not been opened");
    }
  return;
}

// generic integration method
void
integrator::solve(model_mechanism* mm)
  throw (bad_pointer, bad_input, bad_value, bad_type, bad_request)
{
  // set the mechanism so we don't have to pass it around to everything
  mech = mm;
  // make sure state information has been set
  if (state_info == 0)
    {
      throw bad_pointer(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			":integrator::solve(): you are attempting to integrate "
			"without setting the state information");
    }
  // set up initial values
  initial_values();
  // call derived class initialize routines
  initialize();
  // output species names, etc
  output_header();
  // output the initial values
  output(x0);
  // start loop over output points
  double x_init(x0);
  // loop through desired output times
  for (output_citer it = state_info->get_output()->begin();
       it != state_info->get_output()->end(); ++it)
    {
      // call the method to step from current output to next
      x_init = step(x_init, *it);
      // output current values
      output(x_init);
    }
  return;
}  

CH_END_NAMESPACE

/* $Id: integrate.cc,v 1.1.1.1 2004/11/25 20:24:08 banjo Exp $ */
