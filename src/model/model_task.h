// -*- C++ -*-
// Method to conatin information and relationships for model solution.
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
#ifndef CH_MODEL_MODEL_TASK_H
#define CH_MODEL_MODEL_TASK_H 1

#include <string>
#include "except.h"
#include "integrate.h"
#include "model_mech.h"
#include "reactor.h"
#include "task.h"

// set namespace to avoid possible clashes
CH_BEGIN_NAMESPACE

// mechanism solution class input holder
class model_task : public task
{
  integrator* integ;		// method of integration

private:
  // prevent copy construction and assignment
  model_task(const model_task&);
  model_task& operator=(const model_task&);
  // copy information from last model into this one
  void copy(const model_task& original)
    throw (bad_pointer); // copy_integrator()
  // duplicate what pointer points to
  void copy_integrator(const model_task& original)
    throw (bad_pointer); // this, kmc::copy()
  // create and assign the integrator
  void set_integrator(const CH_STD::string& type)
    throw (bad_type); // this
public:
  // ctor: set up a task with the given name
  explicit model_task(const CH_STD::string& name_)
    throw (bad_file, bad_pointer); // task()
  // dtor: destroy all objects
  virtual ~model_task();

  // parse a task input, update given token
  virtual void parse(token_seq_citer& token_it, token_seq_citer end)
    throw (bad_input, bad_pointer, bad_request, bad_value, bad_type); // this,
				// find_task(), set_integrator(), kmc::parse()
  // return pointer to the integrator
  integrator* get_integrator() const;
  // perform the task on the given model_mechanism
  virtual void perform(model_mechanism& mm)
    throw (bad_file, bad_pointer, bad_input, bad_value, bad_type, bad_request);
				// integrator::set_output(),
				// integrator::solve(), task::initialize()
}; // end class model_task

CH_END_NAMESPACE

#endif // not CH_MODEL_MODEL_TASK_H

/* $Id: model_task.h,v 1.1.1.1 2004/11/25 20:24:08 banjo Exp $ */
