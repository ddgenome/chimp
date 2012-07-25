// -*- C++ -*-
// Program task information base class.
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
#ifndef CH_TASK_H
#define CH_TASK_H 1

#include <fstream>
#include <map>
#include <string>
#include <vector>
#include "except.h"
#include "model_mech.h"
#include "token.h"

// set namespace to avoid possible clashes
CH_BEGIN_NAMESPACE

// abstract base class to define interface for the tasks CHIMP is capable of
class task
{
public:
  // set up containers and iterators for tasks
  typedef CH_STD::vector<task*> seq;
  typedef seq::iterator seq_iter;
  typedef seq::const_iterator seq_citer;

private:
  CH_STD::string name;		// user given name of task
  CH_STD::string out_file;	// output file name
  bool opened;			// whether file has been opened yet
protected:
  CH_STD::ofstream out;         // output file stream

private:
  // prevent copy construction and assignment
  task(const task&);
  task& operator=(const task&);
protected:
  // create a copy
  void copy(const task& original);
  // set the file name, do not open yet
  void set_out_file(const CH_STD::string& path);
  // open output file for appending
  void open_out_file()
    throw (bad_file); // this, file_stat()
  // set everything up from the base class standpoint
  void initialize()
    throw (bad_file); // open_out_file()
public:
  // ctor: give me the task name, set default output name
  explicit task(const CH_STD::string name_)
    throw (bad_file, bad_pointer); // file_name(),
				// task_manager::get_current_mechanism()
  // dtor: do nothing
  virtual ~task();

  // start parsing a task file
  static seq parse_file(const CH_STD::string& input_file)
    throw (bad_file, bad_input, bad_pointer, bad_request, bad_value, bad_type);
				// this, tokenizer(), model_task(),
				// model_task::parse()
  // return name of task
  CH_STD::string get_name() const;
  // pure virtual function to parse a given task
  // start up the parsing, call the appropriate derived class parser
  // increment iterator and return pointer to task
  virtual void parse(token_seq_citer& token_it, token_seq_citer end) = 0;
  // (pure virtual) perform the task required
  virtual void perform(model_mechanism& mm) = 0;
}; // end class task

CH_END_NAMESPACE

#endif // not CH_TASK_H

/* $Id: task.h,v 1.1.1.1 2004/11/25 20:24:06 banjo Exp $ */
