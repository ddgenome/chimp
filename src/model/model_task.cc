// Methods to translate input into a working model solution.
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

#include "model_task.h"
#include <typeinfo>
#include "compare.h"
#include "manager.h"
#include "t_string.h"

// set namespace to avoid possible clashes
CH_BEGIN_NAMESPACE

// model_task methods
// ctor: set up a task with the given name
model_task::model_task(const CH_STD::string& name_)
  throw (bad_file, bad_pointer)
  : task(name_), integ(0)
{
  // set up default objects (we own these, so when they change we can
  // just delete them)
  integ = integrator::new_integrator();
}

// dtor: destroy all objects
model_task::~model_task()
{
  // delete the integrator
  delete integ;
  integ = 0;
}

// model_task private methods
// copy information from last model into this one
void
model_task::copy(const model_task& original)
  throw (bad_pointer)
{
  task::copy(original);
  copy_integrator(original);
  return;
}

// copy the integrator type from the original
void
model_task::copy_integrator(const model_task& original)
  throw (bad_pointer)
{
  // see what type of integrator we have
  if (original.integ == 0)
    {
      throw bad_pointer(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			":model_task::copy_integrator(): the integrator of "
			"the original model_task points to null");
    }
  // make a copy
  integ = original.integ->copy();
  return;
}

// create and assign the integrator
void
model_task::set_integrator(const CH_STD::string& type)
  throw (bad_type)
{
  // delete old one
  delete integ;
  // get the new integrator
  integ = integrator::new_integrator(type);
  // make sure it went ok
  if (integ == 0)
    {
      throw bad_type(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		     ":model_task::set_integrator(): the requested integrator "
		     "type (" + type + ") is invalid");
    }
  return;
}

// model_task public methods
// parse a task input, update given iterator
void
model_task::parse(token_seq_citer& token_it, token_seq_citer end)
  throw (bad_input, bad_pointer, bad_request, bad_value, bad_type)
{
  // step through tokens until end of them
  while (token_it != end)
    {
      if (icompare(*token_it, "copy") == 0)
	{
	  // find the task to copy
	  const task* t(task_manager::get().find_task(*++token_it));
	  // see if a task was found
	  if (t == 0)
	    {
	      throw bad_input(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			      ":model_task::parse(): syntax error in input "
			      "for model task " + get_name() + ": "
			      "request to copy input from nonexistent task: "
			      + *token_it);
	    }
	  // try to cast to a model
	  const model_task* mt(dynamic_cast<const model_task*>(t));
	  // make sure dynamic cast worked
	  if (mt == 0)
	    {
	      throw bad_input(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			      ":model_task::parse(): syntax error in input "
			      "for model task " + get_name() + ": "
			      "request to copy input from non-model_task: "
			      + *token_it + " (type: " + typeid(*t).name()
			      + ")");
	    }
	  // actually copy it
	  copy(*mt);
	}
      // output file
      else if (icompare(*token_it, "output") == 0)
	{
	  // set the output file
	  set_out_file(*++token_it);
	  // next!
	  ++token_it;
	  continue;
	}
      // change default integrator
      else if (icompare(*token_it, "begin") == 0)
	{
	  // next!
	  ++token_it;
	  // see what to begin
	  if (icompare(*token_it, "integrator") == 0)
	    {
	      // create a new integrator of the given type
	      set_integrator(*++token_it);
	      // call the integrator parser
	      integ->parse(++token_it, end);
	      continue;		// while ()
	    }
	  else
	    {
	      throw bad_input(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			      ":model_task::parse(): syntax error in input "
			      "for model task " + get_name() + ": "
			      "do not know how to begin a " + *token_it);
	    }
	}
      // terminate task information
      else if (icompare(*token_it, "end") == 0)
	{
	  // make sure next token ends a model
	  if (icompare(*++token_it, "model") != 0)
	    {
	      throw bad_input(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			      ":model_task::parse(): syntax error in input "
			      "for model task " + get_name() + ": "
			      "corresponding end token does not end a model: "
			      + *token_it);
	    }
	  // increment one further
	  ++token_it;
	  // return to caller
	  return;
	}
      else
	{
	  throw bad_input(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			  ":model_task::parse(): syntax error in input for "
			  "model task " + get_name() + ": unrecognized token: "
			  + *token_it);
	}
    }
  // end of file reached
  throw bad_input(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		  ":model_task::parse(): syntax error in input for model task "
		  + get_name() + ": end of file reached while parsing input");
  // shouldn't get here
  return;
}

// return pointer to the integrator
integrator*
model_task::get_integrator() const
{
  return integ;
}

// perform the task on the given model_mechanism
void
model_task::perform(model_mechanism& mm)
  throw (bad_file, bad_pointer, bad_input, bad_value, bad_type, bad_request)
{
  // call base class method to open file, etc.
  initialize();
  // give output file to integrator
  integ->set_output(&out);
  // call the integrator
  integ->solve(&mm);
  return;
}

CH_END_NAMESPACE

/* $Id: model_task.cc,v 1.1.1.1 2004/11/25 20:24:08 banjo Exp $ */
