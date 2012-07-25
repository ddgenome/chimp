// -*- C++ -*-
// Model solution information and methods.
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
#ifndef CH_MODEL_INTEGRATE_H
#define CH_MODEL_INTEGRATE_H 1

#include <fstream>
#include <string>
#include "except.h"
#include "model_mech.h"
#include "state.h"
#include "token.h"

// set namespace to avoid possible clashes
CH_BEGIN_NAMESPACE

// class defining solution method and holding solution information
class integrator
{
protected:
  double x0;			// initial value of independent variable
  model_mechanism* mech;	// species and reactions
  state* state_info;		// reactor state, init conc. and output points
  CH_STD::ofstream* out_file;	// where to put the output

private:
  // prevent assignment
  integrator& operator=(const integrator&);
  // set up initial values of species
  void initial_values()
    throw (bad_input); // this
  // pure virtual method to step from one output point to the next
  virtual double step(double ti, double tf) = 0;
protected:
  // virtual method to initialize integrator (must be called from derived class)
  virtual void initialize()
    throw (bad_pointer, bad_input, bad_value, bad_type, bad_request); // this,
                                // this, state::initialize()
  // set initial value of the independent variable, return old
  double set_initial(double x0_ = 0.0e0);
  // output first row of file (species names, etc.) (no new line)
  virtual void output_header();
  // output the current output point and its values (no new line) to *out_file
  virtual void output(double x)
    throw (bad_type, bad_request, bad_value); // output()
  // output the current output point and its values (no new line) to the stream
  virtual void output(double x, CH_STD::ostream& output_stream)
    throw (bad_type, bad_request, bad_value); // integrate::output(),
                                // model_species::get_quantity()
public:
  // ctor: (default) create default state, set other pointers to zero
  integrator();
  // ctor: copy, mech and out_file should be zero, copy state
  explicit integrator(const integrator& original)
    throw (bad_pointer); // state::state()
  // dtor: delete state
  virtual ~integrator();

  // parse integrator input
  virtual void parse(token_seq_citer& token_it, token_seq_citer end) = 0;
  // copy this into appropriate object and return pointer to new object
  virtual integrator* copy() const = 0;
  // create a new integrator of the given type, return pointer or zero
  static integrator* new_integrator(const CH_STD::string& type = "kmc");
  // return a pointer to the state variable
  state* get_state() const;
  // set the output file stream
  void set_output(CH_STD::ofstream* out_file_)
    throw (bad_file); // this
  // generic solving method
  void solve(model_mechanism* mm)
    throw (bad_pointer, bad_input, bad_value, bad_type, bad_request); // this,
				// initial_values(), kmc::initialize(),
				// output(), kmc::output(), kmc::step()
}; // end class integrator

CH_END_NAMESPACE

#endif // not CH_MODEL_INTEGRATE_H

/* $Id: integrate.h,v 1.1.1.1 2004/11/25 20:24:08 banjo Exp $ */
