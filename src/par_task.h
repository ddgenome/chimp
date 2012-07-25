// -*- C++ -*-
// A simple task which sets the values of the parameters.
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
#ifndef CH_PAR_TASK_H
#define CH_PAR_TASK_H 1

#include "except.h"
#include "model_mech.h"
#include "parameter.h"
#include "task.h"
#include "token.h"

// set namespace to avoid possible clashes
CH_BEGIN_NAMESPACE

// set up containers
typedef CH_STD::map<parameter*,double> par_val_map;
typedef par_val_map::iterator par_val_map_iter;
typedef par_val_map::const_iterator par_val_map_citer;

// class which stores parameter values and then assigns them when performed
class parameter_task : public task
{
  par_val_map values;		// pointers to parameters and their values

private:
  // prevent copy-construction and assignment
  parameter_task(const parameter_task&);
  parameter_task& operator=(const parameter_task&);
  // parse the parameter input file
  virtual void parse(token_seq_citer& token_it, token_seq_citer end)
    throw (bad_input, bad_pointer); // this,
				// task_manager::get_current_mechanism()
public:
  // ctor: give me the file name
  explicit parameter_task(const CH_STD::string& path)
    throw (bad_file, bad_pointer); // task()
  // dtor: do nothing
  virtual ~parameter_task();

  // set up the file for parsing and call parse
  void parse()
    throw (bad_file, bad_input, bad_pointer); // tokenizer(), parse(),
				// model_task::get_current_mechanism()
  // set the input parameter values
  virtual void perform(model_mechanism& mm);
}; // end class parameter_task

CH_END_NAMESPACE

#endif // not CH_PAR_TASK_H

/* $Id: par_task.h,v 1.1.1.1 2004/11/25 20:24:05 banjo Exp $ */
