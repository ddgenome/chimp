// Task management methods.
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

#include "task.h"
#include "compare.h"
#include "file.h"
#include "manager.h"
#include "model/model_task.h"
#include "t_string.h"

// set namespace to avoid possible clashes
CH_BEGIN_NAMESPACE

// task methods
// ctor: user given name, set up default output file name
task::task(const CH_STD::string name_)
  throw (bad_file, bad_pointer)
  : name(name_), out_file(), opened(false), out()
{
  // set file name using mechanism name and task name
  // get mechanism name
  file_name mech(task_manager::get().get_current_mechanism()->get_name());
  // use only base name information
  out_file = mech.get_base() + "." + name + ".out";
}

// dtor: do nothing (ofstream dtor closes it)
task::~task()
{}

// class task protected methods
// copy certain members from original to this object
void
task::copy(const task& original)
{
  // name should be original
  // set the ouput file name
  set_out_file(original.out_file);
  // just in case
  opened = false;
  return;
}

// set the output file name, do not open
void
task::set_out_file(const CH_STD::string& path)
{
  out_file = path;
  return;
}

// safely open the output file
void
task::open_out_file()
  throw (bad_file)
{
  // open file for appending (in case someone wants to use the same
  // output file for several tasks)
  out.open(out_file.c_str(), CH_STD::ios::app);
  // make sure it opened ok
  if (!out)
    {
      // find out why
      file_stat fail(out_file);
      throw bad_file(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		     ":task::open_out_file(): could not open file "
		     + out_file + ":" + fail.why_no_write());
    }
  // use scientific notation for float fields
  out.setf(CH_STD::ios::scientific, CH_STD::ios::floatfield);
  // set flag
  opened = true;
  // print out this tasks name to ouput file
  out << "# " + name << CH_STD::endl;
  return;
}

// set everything up from the base class standpoint
void
task::initialize()
  throw (bad_file)
{
  // open the ouput file
  open_out_file();
  return;
}

// class task public methods
// parse (or at least start) the parsing of task input
// return sequence of pointers to tasks generated
task::seq
task::parse_file(const CH_STD::string& input_file)
  throw (bad_file, bad_input, bad_pointer, bad_request, bad_value, bad_type)
{
  // create the empty sequence of tasks
  seq tasks;
  // tokenize the input file
  tokenizer input(input_file);
  // start stepping through the tokens
  token_seq_citer token_it(input.begin());
  while (token_it != input.end())
    {
      // make sure input is valid
      if (icompare(*token_it, "begin") == 0)
	{
	  // next!
	  ++token_it;
	  if (icompare(*token_it, "model") == 0)
	    {
	      // create a new model_task with next token as its name
	      model_task* mt = new model_task(*++token_it);
	      // call the model_task parser
	      mt->parse(++token_it, input.end());
	      // insert the task into the end of the sequence
	      tasks.push_back(mt);
	    }
	  else			// unknown task type
	    {
	      throw bad_input(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			      ":task::parse_file(): syntax error in task "
			      "input file " + input_file + ": unrecognized "
			      "task type: " + *token_it);
	    }
	}
      else
	{
	  throw bad_input(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			  ":task::parse_file(): syntax error in task input "
			  "file " + input_file + ": unrecognized input "
			  "outside of task context: " + *token_it);
	}
    }
  return tasks;
}

// return name of task
CH_STD::string
task::get_name() const
{
  return name;
}

CH_END_NAMESPACE

/* $Id: task.cc,v 1.1.1.1 2004/11/25 20:24:06 banjo Exp $ */
