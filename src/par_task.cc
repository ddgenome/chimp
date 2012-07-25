// Methods to handle parameters value inputs.
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

#include "par_task.h"
#include <cstdlib>
#include "manager.h"
#include "mechanism.h"
#include "t_string.h"

// set namespace to avoid possible clashes
CH_BEGIN_NAMESPACE

// parameter_task class methods
// ctor: the input file name is give, use that for a name
parameter_task::parameter_task(const CH_STD::string& path)
  throw (bad_file, bad_pointer)
  : task(path)
{}

// dtor: do nothing
parameter_task::~parameter_task()
{}

// class parameter_task private methods
void
parameter_task::parse(token_seq_citer& token_it, token_seq_citer end)
  throw (bad_input, bad_pointer)
{
  // loop through tokens
  while (token_it != end)
    {
      // find the parameter that matches the token
      parameter* par_p(task_manager::get().get_current_mechanism()->get_parameter(*token_it));
      // make sure the parameter is in this mechanism
      if (par_p == 0)
	{
	  throw bad_input(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			  ":parameter_task::parse(): syntax error in "
			  "parameter specification: paramter " + *token_it
			  + " does not exist in the current mechanism");
	}
      // the next token is the value
      // insert the pair into the map
      values[par_p] = CH_STD::atof((++token_it)->c_str());
      // next!
      ++token_it;
    }
  // done
  return;
}

// parameter_task public methods
// set up the file for parsing and call parse
void
parameter_task::parse()
  throw (bad_file, bad_input, bad_pointer)
{
  // tokenize the input file
  tokenizer input(get_name());
  // start stepping through the tokens at the beginning
  token_seq_citer token_it(input.begin());
  // call the actual parser
  parse(token_it, input.end());
  return;
}

// set the input parameter values
void
parameter_task::perform(model_mechanism& mm)
{
  // loop through and set all the parameters
  for (par_val_map_iter it(values.begin()); it != values.end(); ++it)
    {
      // set the value
      it->first->set_value(it->second);
    }
  return;
}

CH_END_NAMESPACE

/* $Id: par_task.cc,v 1.1.1.1 2004/11/25 20:24:05 banjo Exp $ */
