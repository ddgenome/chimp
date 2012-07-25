// -*- C++ -*-
// Classes defining the intial state of reactor and output points.
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
#ifndef CH_MODEL_STATE_H
#define CH_MODEL_STATE_H 1

#include <map>
#include <vector>
#include <utility>
#include "except.h"
#include "quantity.h"
#include "reactor.h"
#include "species.h"
#include "token.h"

// set namespace to avoid possible clashes
CH_BEGIN_NAMESPACE

// set up some typedef's to make everyone's life easier
typedef CH_STD::vector<double> double_seq;
typedef double_seq::iterator output_iter;
typedef double_seq::const_iterator output_citer;

// class to manage desired output points
class output
{
  double_seq out;		// output values
  bool done;			// can any more points be added

private:
  // prevent assignment
  output& operator=(const output&);
  // add a new output point to the end of the list
  void push_back(double last)
    throw (bad_input, bad_value); // this
  // add a loop to the desired outputs, return number of points inserted
  int push_loop(double first, double max, double step = 1.0e0)
    throw (bad_input, bad_value); // this, loop()
  // insert loop values
  int loop(double first, double max, double increment)
    throw (bad_value); // this
  // disallow any more entries
  void no_more();
public:
  // ctor: (default) create empty output list
  output();
  // ctor: copy
  output(const output& original);
  // dtor: do nothing
  ~output();

  // parse the output input (huh?)
  void parse(token_seq_citer& token_it, token_seq_citer end)
    throw (bad_input, bad_value, bad_type); // this, push_back(), push_loop(),
				// reactor::parse()
  // return pointer to first element
  output_citer begin();
  // return pointer past the last element
  output_citer end();
}; // end class output

// set up intial value map for species
typedef CH_STD::map<species*,quantity*> init_val_map;
typedef init_val_map::iterator init_val_map_iter;
typedef init_val_map::const_iterator init_val_map_citer;

// generic integrator information
class state
{
  reactor* rxtor;		// reactor information
  output* out;			// output times|weights|lengths|sites
  init_val_map values;		// initial species values
  bool values_done;		// can we add more values?
  bool previous_values;		// whether to use previous task final values

private:
  // prevent assignment
  state& operator=(const state&);
  // add a species initial value pair
  void add_initial_value(species* sp, quantity* value);
  // set the reactor, return pointer to old one
  reactor* set_reactor(const CH_STD::string& type)
    throw (bad_type); // this
  // parse the quantity section of the input
  void parse_quantity(token_seq_citer& token_it, token_seq_citer end)
    throw (bad_input, bad_pointer, bad_type, bad_value); // this,
				// parse_init_val(),
				// initial_fluid_quantity(), initial_coverage()
  // parse a species and value specification
  CH_STD::pair<species*,double> parse_init_val(token_seq_citer& token_it)
    throw (bad_input, bad_pointer); // this,
				// task_manager::get_current_mechanism()
  // assign initial value for the given species
  void initial_fluid_quantity(species* sp_p, double value, quantity::type type)
    throw (bad_input, bad_type, bad_value); // this,
				// fluid_quantity::set_quantity()
  // assign the initial coverage for the given species
  void initial_coverage(species* sp_p, double value)
    throw (bad_input, bad_value); // this, surface_quantity::set_quantity()
  // set whether to use the final values from a previous task, return old value
  bool set_previous_values(bool previous_values_);
public:
  // ctor: (default) create output and default reactor
  state();
  // ctor: copy
  explicit state(const state& original)
    throw (bad_pointer); // this
  // dtor: delete reactor and output
  ~state();

  // parse state input
  void parse(token_seq_citer& token_it, token_seq_citer end)
    throw (bad_input, bad_value, bad_type, bad_pointer); // this,
				// output::parse(), set_reactor(),
				// parse_quantity()
  // prepare this object for solution
  void initialize(model_species::seq_citer species_begin,
		  model_species::seq_citer species_end)
    throw (bad_type, bad_value); // reactor::initialize(),
				// flow_reactor::initialize(),
				// cstr::initialize()
  // return pointer to the reactor
  reactor* get_reactor() const;
  // return pointer to the output sequence
  output* get_output() const;
  // return pointer to beginning of initial value list
  init_val_map_citer init_val_begin();
  // return pointer to end of initial value list
  init_val_map_citer init_val_end();
  // return status of previous_values
  bool get_previous_values() const;
}; // end class state

CH_END_NAMESPACE

#endif // not CH_MODEL_STATE_H

/* $Id: state.h,v 1.1.1.1 2004/11/25 20:24:08 banjo Exp $ */
