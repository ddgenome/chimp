// -*- C++ -*-
// Class which controls what tasks are executed.
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
#ifndef CH_MANAGER_H
#define CH_MANAGER_H 1

#include <map>
#include <string>
#include <vector>
#include "except.h"
#include "mechanism.h"
#include "unique.h"
#include "task.h"

// set namespace to avoid possible clashes
CH_BEGIN_NAMESPACE

// set of tasks for a specific mechanism
typedef CH_STD::map<mechanism*,task::seq> mechanism_tasks;
typedef mechanism_tasks::iterator mechanism_tasks_iter;
typedef mechanism_tasks::const_iterator mechanism_tasks_citer;

typedef CH_STD::vector<CH_STD::string> input_seq;
typedef input_seq::iterator input_seq_iter;
typedef input_seq::const_iterator input_seq_citer;

// class to parse input file and perform tasks
class task_manager
{
  static task_manager master;	// singleton instance of task_manager
  mechanism::seq mechanisms;	// all mechanisms in correct order
  mechanism_tasks tasks;	// the to do list for each mechanism
  mechanism* current;		// current active mechanism

private:
  // ctor: (default) make private for singleton
  task_manager();
  // prevent copy construction and assignment
  task_manager(const task_manager&);
  task_manager& operator=(const task_manager&);
  // parse a single control file
  void parse_control(CH_STD::string path)
    throw (bad_file, bad_input, bad_pointer, bad_request, bad_value, bad_type);
				// this, tokenizer(), file_name(),
				// parameter_file(), task_file(),
				// new_mechanism()
  // create an empty mechanism, put it into list, make current
  void new_mechanism(const CH_STD::string& name)
    throw (bad_file, bad_input, bad_request); // mechanism(),
				// mechanism::parse()
  // create a new parameter task and parse input
  void parameter_file(const CH_STD::string& path)
    throw (bad_pointer, bad_file, bad_input); // this, parameter_task(),
				// parameter_task::parse()
  // send the file to the task parser and insert return values
  void task_file(const CH_STD::string& file_name)
    throw (bad_pointer, bad_file, bad_input, bad_request, bad_value, bad_type);
				// this, task::parse_file()
public:
  // dtor: erase name from list and delete tasks
  ~task_manager();

  // return a reference to the singleton
  static task_manager& get();
  // parse a list of control files
  void parse_control(const input_seq& input_files)
    throw (bad_file, bad_input, bad_pointer, bad_request, bad_value, bad_type);
				// parse_control()
  // return pointer to current mechanism
  mechanism* get_current_mechanism() const
    throw (bad_pointer); // this
  // find the task of the given name for current mechanism
  const task* find_task(const CH_STD::string& name)
    throw (bad_pointer); // this
  // perform the given tasks
  void perform()
    throw (bad_pointer, bad_file, bad_input, bad_value, bad_type, bad_request);
				// this, model_reaction(), model_task::perform()
}; // end class task_manager

CH_END_NAMESPACE

#endif // not CH_MANAGER_H

/* $Id: manager.h,v 1.1.1.1 2004/11/25 20:24:05 banjo Exp $ */
