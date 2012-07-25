// Methods for setting intial state of reactor and output points.
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

#include "state.h"
#include <cstdlib>
#include <typeinfo>
#include "compare.h"
#include "manager.h"
#include "t_string.h"

// set namespace to avoid possible clashes
CH_BEGIN_NAMESPACE

// output class methods
// ctor: (default) create and empty list
output::output()
  : out(), done(false)
{}

// ctor: copy
output::output(const output& original)
  : out(original.out), done(false)
{}

// dtor: do nothing
output::~output()
{}

// output private methods
// add a new output point to the end of the list
void
output::push_back(double last)
  throw (bad_input, bad_value)
{
  if (done)
    {
      throw bad_input(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		      ":output::push_back(): these outputs have already been "
		      "used, so you can not add any more output points");
    }
  if (!out.empty())
    {
      // get pointer to last element
      double current_last(out.back());
      // make sure the new final value is greater than the previous
      if (last <= current_last)
	{
	  throw bad_value(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			  ":output::push_back(): output points are not "
			  "monotonically increasing; current value("
			  + t_string(last) + ") is less than previous("
			  + t_string(current_last) + ")");
	}
    }
  // actually insert the element
  out.push_back(last);
  return;
}

// add a loop to the desired outputs, return number of points inserted
// defaults step = 1.0e0
int
output::push_loop(double first, double max, double step)
  throw (bad_input, bad_value)
{
  if (done)
    {
      throw bad_input(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		      ":output::push_loop(): these outputs have already been "
		      "used, so you can not add any more output points");
    }
  return loop(first, max, step);
}

// create output points from loop parameters, return number of points inserted
int
output::loop(double first, double max, double step)
  throw (bad_value)
{
  if (step <= 0.0e0)
    {
      throw bad_value(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		      ":output::output(): specified loop increment is "
		      "non-positive");
    }
  if (max < first)
    {
      throw bad_value(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		      ":output::output(): specified loop maximum is less than "
		      "initial output value (minimum)");
    }
  // insert the desired output points into the sequence
  int count(0);			// how many points have been inserted?
  for (double d = first; d <= max; d += step)
    {
      ++count;
      out.push_back(d);
    }
  return count;
}

// disallow any more entries
void
output::no_more()
{
  done = true;
  return;
}

// output public methods
void
output::parse(token_seq_citer& token_it, token_seq_citer end)
  throw (bad_input, bad_value, bad_type)
{
  // loop through input
  while (token_it != end)
    {
      // check if there is a loop specified
      if (*token_it == "(")
	{
	  // get the next two numbers
	  double first(CH_STD::atof((++token_it)->c_str()));
	  double max(CH_STD::atof((++token_it)->c_str()));
	  // see if next token is close parenthesis
	  if (*++token_it == ")")
	    {
	      // add the loop to the output
	      push_loop(first, max);
	    }
	  else
	    {
	      // add the loop to the output using the given step size
	      push_loop(first, max, CH_STD::atof(token_it->c_str()));
	      // make sure next token is a close parenthesis
	      if (*++token_it != ")")
		{
		  throw bad_input(PACKAGE ":" __FILE__ ":" + t_string(__LINE__)
			      + ":output::parse(): syntax error in "
			      "input for output: expecting closing "
			      "parenthesis but got " + *token_it);
		}
	    }
	  // increment one further
	  ++token_it;
	  continue;		// where ()
	}
      // see if it looks like a number
      else if (token_it->find_first_of("0123456789.-+") == 0)
	{
	  push_back(CH_STD::atof(token_it->c_str()));
	  // increment to next token
	  ++token_it;
	  continue;		// where ()
	}
      else if (icompare(*token_it, "end") == 0)
	{
	  // make sure it is the end of output input
	  if (icompare(*++token_it, "output") != 0)
	    {
	      throw bad_input(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			      ":output::parse(): syntax error in "
			      "input for output: corresponding end token "
			      "does not end a output: " + *token_it);
	    }
	  // make sure no more output points are inserted
	  no_more();
	  // increment one further
	  ++token_it;
	  // return to caller
	  return;
	}
      else
	{
	  throw bad_input(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			  ":output::parse(): syntax error in input "
			  "for output: unrecognized token: "
			  + *token_it);
	}
    }
  // end of file reached
  throw bad_input(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		  "output::parse(): syntax error in input for "
		  "output: end of file reached while parsing input");
  // shouldn't get here
  return;
}

// return a pointer to the first element
output_citer
output::begin()
{
  done = true;			// do not allow insertion of more points
  return out.begin();
}

// return a pointer past the last element
output_citer
output::end()
{
  done = true;			// do not allow insertion of more points
  return out.end();
}

// state class methods
// ctor: (default) create output sequence and default reactor
state::state()
  : rxtor(0), out(0), values(), previous_values(false)
{
  // create default reactor
  rxtor = reactor::new_reactor();
  // create empty sequence
  out = new output();
}

// ctor: copy everything we own
state::state(const state& original)
  throw (bad_pointer)
  : rxtor(0), out(0), values(), previous_values(original.previous_values)
{
  // make sure reactor on original was set
  if (original.rxtor == 0)
    {
      throw bad_pointer(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			":state::state(): reactor for original state has "
			"gotten corrupted, it points to zero");
    }
  // make a copy
  rxtor = original.rxtor->copy();
  // make a new output vector
  out = new output(*original.out);
  // must copy the initial values one by one
  for (init_val_map_citer it(original.values.begin());
       it != original.values.end(); ++it)
    {
      // make sure original quantity was set
      if (it->second == 0)
	{
	  throw bad_pointer(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			    ":state::state(): quantity for species " +
			    it->first->get_name() + " is zero, so it cannot "
			    "be copied");
	}
      // copy the quantity
      quantity* qp(it->second->copy());
      // add the species and quantity to our map
      add_initial_value(it->first, qp);
    }
}

// dtor: delete pointers and the quantities
state::~state()
{
  // delete the reactor and output
  delete rxtor;
  rxtor = 0;
  delete out;
  out = 0;
}

// state class private methods
// add a species initial value pair
void
state::add_initial_value(species* sp, quantity* value)
{
  // see if entry for this species already exists
  init_val_map_iter it(values.find(sp));
  if (it == values.end())
    {
      // entry for this species does not exist so create one
      values.insert(CH_STD::make_pair(sp, value));
    }
  else
    {
      // delete the pointed to quantity (we own it for now)
      delete it->second;
      // assign the quantity pointer to the one give
      it->second = value;
    }
  return;
}

// set the reactor
reactor*
state::set_reactor(const CH_STD::string& type)
  throw (bad_type)
{
  // delete old reactor
  delete rxtor;
  // get new one
  rxtor = reactor::new_reactor(type);
  // make sure it is ok
  if (rxtor == 0)
    {
      throw bad_type(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		     ":state::set_reactor(): unable to create a reactor "
		     " of type " + type);
    }
  return rxtor;
}

// parse the quantity section of the input
void
state::parse_quantity(token_seq_citer& token_it, token_seq_citer end)
  throw (bad_input, bad_pointer, bad_type, bad_value)
{
  // loop through input
  while (token_it != end)
    {
      // check first letter of the token
      if (icompare(*token_it, "p") == 0) // pressure
	{
	  // get the species pointer and value
	  CH_STD::pair<species*,double> sp_val(parse_init_val(++token_it));
	  // check and insert the species and value into the map
	  initial_fluid_quantity(sp_val.first, sp_val.second,
				 quantity::Epressure);
	  continue;		// where ()
	}
      else if (icompare(*token_it, "c") == 0) // concentration
	{
	  // get the species pointer and value
	  CH_STD::pair<species*,double> sp_val(parse_init_val(++token_it));
	  // check and insert the species and value into the map
	  initial_fluid_quantity(sp_val.first, sp_val.second,
				 quantity::Econcentration);
	  continue;		// where ()
	}
      else if (icompare(*token_it, "f") == 0) // flow
	{
	  // get the species pointer and value
	  CH_STD::pair<species*,double> sp_val(parse_init_val(++token_it));
	  // check and insert the species and value into the map
	  initial_fluid_quantity(sp_val.first, sp_val.second, quantity::Eflow);
	  continue;		// where ()
	}
      else if (icompare(*token_it, "@") == 0) // coverage
	{
	  // get the species pointer and value
	  CH_STD::pair<species*,double> sp_val(parse_init_val(++token_it));
	  // check and insert the species and value into the map
	  initial_coverage(sp_val.first, sp_val.second);
	  continue;		// where ()
	}
      else if (icompare(*token_it, "end") == 0)
	{
	  // make sure it is the end of quantity input
	  if (icompare(*++token_it, "quantity") != 0)
	    {
	      throw bad_input(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			      ":state::parse_quantity(): syntax error in "
			      "input for quantity: corresponding end token "
			      "does not end a quantity: " + *token_it);
	    }
	  // increment one further
	  ++token_it;
	  // return to caller
	  return;
	}
      else
	{
	  throw bad_input(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			  ":state::parse_quantity(): syntax error in input "
			  "for quantity: unrecognized token: "
			  + *token_it);
	}
    }
  // end of file reached
  throw bad_input(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		  "state::parse_quantity(): syntax error in input for "
		  "quantity: end of file reached while parsing input");
  // shouldn't get here
  return;
}

// parse a species and value specification
CH_STD::pair<species*,double>
state::parse_init_val(token_seq_citer& token_it)
  throw (bad_input, bad_pointer)
{
  // make sure next token is an open bracket
  if (icompare(*token_it, "[") != 0)
    {
      throw bad_input(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		      ":state::parse_init_val(): syntax error in "
		      "quantity specification: expected '[' but found "
		      + *token_it);
    }
  // find the species given by the next token
  species* species_p(task_manager::get().get_current_mechanism()->get_species(*++token_it));
  // make sure species is in mechanism
  if (species_p == 0)
    {
      throw bad_input(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		      ":state::parse_init_val(): syntax error in "
		      "quantity specification: species " + *token_it
		      + " does not exist in the current mechanism");
    }
  // make sure next token is a close bracket
  if (icompare(*++token_it, "]") != 0)
    {
      throw bad_input(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		      ":state::parse_init_val(): syntax error in "
		      "quantity specification: expected ']' but found "
		      + *token_it);
    }
  // make cure next token is an equal sign
  if (icompare(*++token_it, "=") != 0)
    {
      throw bad_input(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		      ":state::parse_init_val(): syntax error in "
		      "quantity specification: expected '=' but found "
		      + *token_it);
    }
  // convert next token into value
  double value(CH_STD::atof((++token_it)->c_str()));
  // increment token past the current species/value pair
  ++token_it;
  return CH_STD::make_pair(species_p, value);
}

// assign initial value for the given species
void
state::initial_fluid_quantity(species* sp_p, double value, quantity::type type)
  throw (bad_input, bad_type, bad_value)
{
  // make sure species is not a surface species
  if (sp_p->get_surface_coordination() > 0U)
    {
      throw bad_input(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		      ":state::initial_fluid_quantity(): syntax error in "
		      "input for quantity: pressure value given to "
		      "surface species " + sp_p->get_name());
    }
  // see if entry for this species already exists
  init_val_map_iter val_it(values.find(sp_p));
  if (val_it == values.end())	// does not exist
    {
      // create and insert a quantity into the map, assign iterator to it
      val_it =
	values.insert(CH_STD::make_pair(sp_p, new fluid_quantity())).first;
    }
  // set the value of the quantity type
  val_it->second->set_quantity(type, value);
  return;
}

// assign the initial coverage for the given species
void
state::initial_coverage(species* sp_p, double value)
  throw (bad_input, bad_value)
{
  // make sure species is a surface species
  if (sp_p->get_surface_coordination() < 1U)
    {
      throw bad_input(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		      ":state::initial_coverage(): syntax error in "
		      "input for quantity: coverage value given to "
		      "non-surface species " +
		      sp_p->get_name());
    }
  // see if entry for this species already exists
  init_val_map_iter val_it(values.find(sp_p));
  if (val_it == values.end())	// does not exist
    {
      // create and insert a quantity into the map, assign iterator to it
      val_it =
	values.insert(CH_STD::make_pair(sp_p, new surface_quantity())).first;
    }
  // set the value of the quantity (type argument is ignored)
  val_it->second->set_quantity(quantity::Econcentration, value);
  return;
}

// set whether to use the final values from a previous task, return old value
bool
state::set_previous_values(bool previous_values_)
{
  bool old(previous_values);
  previous_values = previous_values_;
  return old;
}

// state class public methods
// parse state input
void
state::parse(token_seq_citer& token_it, token_seq_citer end)
  throw (bad_input, bad_value, bad_type, bad_pointer)
{
  // loop through input
  while (token_it != end)
    {
      if (icompare(*token_it, "begin") == 0)
	{
	  ++token_it;		// next token
	  if (icompare(*token_it, "output") == 0)
	    {
	      // call the output parser
	      out->parse(++token_it, end);
	      continue;		// while ()
	    }
	  else if (icompare(*token_it, "reactor") == 0)
	    {
	      // change the reactor using the next token
	      set_reactor(*++token_it);
	      // call the reactor parser with the next token
	      rxtor->parse(++token_it, end);
	      continue;		// while ()
	    }
	  else if (icompare(*token_it, "quantity") == 0)
	    {
	      // call the quantity parser
	      parse_quantity(++token_it, end);
	      continue;		// while ()
	    }
	  else
	    {
	      throw bad_input(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			      ":state::parse(): syntax error in input "
			      "for integrator: do not know how to begin "
			      + *token_it);
	    }
	}
      else if (icompare(*token_it, "end") == 0)
	{
	  // make sure it is the end of state input
	  if (icompare(*++token_it, "state") != 0)
	    {
	      throw bad_input(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			      ":state::parse(): syntax error in input "
			      "for state: corresponding end token does "
			      "not end a state: " + *token_it);
	    }
	  // increment one further
	  ++token_it;
	  // return to caller
	  return;
	}
      else
	{
	  throw bad_input(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			  ":state::parse(): syntax error in input "
			  "for state: unrecognized token: "
			  + *token_it);
	}
    }
  // end of file reached
  throw bad_input(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		  "state::parse(): syntax error in input for state: "
		  "end of file reached while parsing input");
  // shouldn't get here
  return;
}

// prepare this object for solution
void
state::initialize(model_species::seq_citer species_begin,
		  model_species::seq_citer species_end)
  throw (bad_type, bad_value)
{
  // initialize the reactor
  rxtor->initialize(species_begin, species_end);
  return;
}

// return the pointer to the reactor (class too big to mirror)
reactor*
state::get_reactor() const
{
  return rxtor;
}

// return pointer to output sequence
output*
state::get_output() const
{
  return out;
}

// return pointer to beginning of initial value list
init_val_map_citer
state::init_val_begin()
{
  return values.begin();
}

// return pointer to end of initiali value list
init_val_map_citer
state::init_val_end()
{
  return values.end();
}

// return status of previous_values
bool
state::get_previous_values() const
{
  return previous_values;
}

CH_END_NAMESPACE

/* $Id: state.cc,v 1.1.1.1 2004/11/25 20:24:08 banjo Exp $ */
