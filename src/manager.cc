// Methods for the execution of tasks in proper order.
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

#include "manager.h"
#include <ctime>
#include <iostream>
#include <sys/times.h>
#include <unistd.h>
#include <utility>
#include "compare.h"
#include "debug.h"
#include "file.h"
#include "par_task.h"
#include "t_string.h"
#include "token.h"

// declare the mechanism parser
extern int yyparse();

// set namespace to avoid possible clashes
CH_BEGIN_NAMESPACE

// initialize static instance variables
// initialize singleton instance of task_manger
task_manager task_manager::master;

// task_manager methods
// ctor: (default) create and empty task manager
task_manager::task_manager()
  : mechanisms(), tasks(), current(0)
{}

// dtor: delete mechanisms and tasks
task_manager::~task_manager()
{
  current = 0;
  // in case we missed any
  for (mechanism_tasks_iter mi = tasks.begin(); mi != tasks.end(); ++mi)
    {
      // loop over tasks
      for (task::seq_iter ti = mi->second.begin(); ti != mi->second.end(); ++ti)
	{
	  // delete tasks
	  delete *ti;
	  *ti = 0;
	}
      // delete mechanisms
      delete mi->first;
      //mi->first = 0;
    }
}

// class task_manager private methods
// parse given control file
void
task_manager::parse_control(CH_STD::string path)
  throw (bad_file, bad_input, bad_pointer, bad_request, bad_value, bad_type)
{
  // check file extension
  file_name name(path);
  if (name.get_extension() != ".chimp")
    {
      // tack `.chimp' onto the end of the file name
      CH_STD::string ext(".chimp");
      path += ext;
    }
  // create an object to interface with the tokenizer
  tokenizer control_tokens(path);
  // start stepping through the input tokens
  token_seq_citer token_it(control_tokens.begin());
  while (token_it != control_tokens.end())
    {
      if (icompare(*token_it, "mechanism") == 0)
	{
	  // create mechanism giving input file (next token)
	  new_mechanism(*++token_it);
	  // advance to next token
	  ++token_it;
	}
      else if (icompare(*token_it, "parameter") == 0)
	{
	  // call the parameter task handling method
	  parameter_file(*++token_it);
	  // next!
	  ++token_it;
	}
      else if (icompare(*token_it, "task") == 0)
	{
	  // call method which talks to tasks and handles the return value
	  task_file(*++token_it);
	  // advance to next token
	  ++token_it;
	}
      else			// unknown directive
	{
	  throw bad_input(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			  ":task_manager::parse_control(): unknown control "
			  "file directive (" + *token_it + ") in file "
			  + path);
	}
    }
  return;
}

// create an empty mechanism, put it into list, make current
void
task_manager::new_mechanism(const CH_STD::string& name)
  throw (bad_file, bad_input, bad_request)
{
  // create it, ctor sets up and parses file
  mechanism* mp = new mechanism(name);
  // put into list of mechanisms
  mechanisms.push_back(mp);
  // put it into tasks with an empty task list
  tasks.insert(make_pair(mp, task::seq()));
  // make it current
  current = mp;
  // parse the mechanism (must be done after current is set)
  mp->parse();
  return;
}

// create a new parameter task and parse input
void
task_manager::parameter_file(const CH_STD::string& path)
  throw (bad_pointer, bad_file, bad_input)
{
  if (current == 0)
    {
      throw bad_pointer(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			"task_manager::add_task(): currently selected "
			"mechanism is invalid (null pointer), so parameter "
			"values cannot be added");
    }
  // else
  // create a parameter_task giving the input file name
  parameter_task* ptp = new parameter_task(path);
  // parse the input
  ptp->parse();
  // add the task to the list
  tasks[current].push_back(ptp);
  return;
}

// add the given task to the current mechanism
void
task_manager::task_file(const CH_STD::string& file_name)
  throw (bad_pointer, bad_file, bad_input, bad_request, bad_value, bad_type)
{
  if (current == 0)
    {
      throw bad_pointer(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			"task_manager::add_task(): currently selected "
			"mechanism is invalid (null pointer), so task cannot "
			"be added");
    }
  // else
  // call task file parser and get sequence of tasks
  task::seq ts(task::parse_file(file_name));
  // loop through the list an add them to the current mechanism
  for (task::seq_citer it(ts.begin()); it != ts.end(); ++it)
    {
      tasks[current].push_back(*it);
    }
  return;
}

// class task_manager public methods
// return reference to the singleton
task_manager&
task_manager::get()
{
  return master;
}

// parse a list of control files
void
task_manager::parse_control(const input_seq& input_files)
  throw (bad_file, bad_input, bad_pointer, bad_request, bad_value, bad_type)
{
  // loop through control input files
  for (input_seq_citer it(input_files.begin()); it != input_files.end();
       ++it)
    {
      parse_control(*it);
    }
}

// return pointer to currrent mechanism
mechanism*
task_manager::get_current_mechanism() const
  throw (bad_pointer)
{
  // make sure current has been set
  if (current == 0)
    {
      throw bad_pointer(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			"task_manager::get_current_mechanism(): there is no "
			"currently selected mechanism; current pointer points "
			"to null");
    }
  // else
  return current;
}

// find the task of the given name for the current mechanism
const task*
task_manager::find_task(const CH_STD::string& name)
  throw (bad_pointer)
{
  // make sure there is a current mechanism
  if (current == 0)
    {
      throw bad_pointer(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			"task_manager::find_task(): currently selected "
			"mechanism is invalid (null pointer), so cannot "
			"find task input for " + name);
    }
  // else
  // go backwards through the list (might be faster)
  for (task::seq::reverse_iterator rit(tasks[current].rbegin());
       rit != tasks[current].rend(); ++rit)
    {
      if (icompare((*rit)->get_name(), name) == 0)
	{
	  return *rit;
	}
    }
  // not found
  return 0;
}

// perform the given tasks
void
task_manager::perform()
  throw (bad_pointer, bad_file, bad_input, bad_value, bad_type, bad_request)
{
  // get the clock ticks for time reporting
  long int clktck(sysconf(_SC_CLK_TCK));
  // make sure this worked
  if (clktck < 0L)
    {
      // set it to something somewhat acceptable
      clktck = CLOCKS_PER_SEC;
    }
  // loop over mechanisms in proper order
  for (mechanism::seq_iter it = mechanisms.begin(); it != mechanisms.end();
       ++it)
    {
      // set the mechanism to the current
      current = *it;
      // create a model_mechanism for the given mechanism
      model_mechanism mi_model(*current);
      // find the current mechanism in the task map
      mechanism_tasks_iter mi(tasks.find(current));
      if (mi == tasks.end())
	{
	  throw bad_pointer(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			    "task_manager::perform(): currently selected "
			    "mechanism from list does not appear in map with "
			    "tasks, containers have been corrupted");
			
	}
      // loop over tasks
      for (task::seq_iter ti = mi->second.begin(); ti != mi->second.end(); ++ti)
	{
	  // pre-task timing information
	  struct tms before;
	  clock_t wall_before((clock_t) -1);
	  if (debug::get().get_level() > 0U)
	    {
	      // state your intention
	      CH_STD::cout << PACKAGE ": performing task " + (*ti)->get_name()
		+ "..." << CH_STD::flush;
	      // get time used so far
	      wall_before = times(&before);
	    }
	  // perform the task
	  (*ti)->perform(mi_model);
	  if (debug::get().get_level() > 0U)
	    {
	      // post task timing information
	      struct tms after;
	      clock_t wall_after((clock_t) -1);
	      // get time used now
	      wall_after = times(&after);
	      CH_STD::cout << " completed; ";
	      // output timing information
	      if (wall_before != (clock_t) -1 && wall_after != (clock_t) -1)
		{
		  CH_STD::cout << ((after.tms_utime - before.tms_utime)
				   / (double) clktck) << "u "
			       << ((after.tms_stime - before.tms_stime)
				   / (double) clktck) << "s "
			       << ((wall_after - wall_before)
				   / (double) clktck) << "w";
		}
	      CH_STD::cout << CH_STD::endl;
	    }
	  // delete the task
	  delete *ti;
	  *ti = 0;
	}
      // done with task so delete it
      delete current;
      current = 0;
      // erase the map entry pointing to it
      tasks.erase(mi);
    }
  return;
}

CH_END_NAMESPACE

/* $Id: manager.cc,v 1.1.1.1 2004/11/25 20:24:05 banjo Exp $ */
