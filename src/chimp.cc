// CHIMP - CHIMP HIerarchical Modeling Program.
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

#include "chimp.h"
#include <cassert>
#include <cstdlib>		// exit and abort
#include <fstream>
#include <typeinfo>
#include <vector>
#include <getopt.h>		// GNU long option processing
#include "debug.h"
#include "except.h"
#include "manager.h"
#include "t_string.h"

// the start of it all, CHIMP's main function
int
main (int argc, char* argv[])
{
  // get program names
  CH_STD::string invoked_as(*argv); // name used to invoke program

  // process command line arguments
  // getopt_long variables
  int option_char;		// return value of getopt_long()
  char *short_options = "d::hqv";
  struct option long_options[] =
    {
      {"debug", optional_argument, 0, 'd'},
      {"debug-file", optional_argument, 0, 1},
      {"help", no_argument, 0, 'h'},
      {"quiet", no_argument, 0, 'q'},
      {"silent", no_argument, 0, 'q'},
      {"version", no_argument, 0, 'v'},
      {0, 0, 0, 0}		// must terminate the list
    };
  // option flags
  bool help(false);		// did they ask for --help?
  // start looping through the options
  while (true)
    {
      int option_index = 0;
      option_char = getopt_long(argc, argv, short_options, long_options,
				&option_index);
      // detect end of option processing
      if (option_char == -1)
	{
	  break;		// while (true) - no more options
	}
      switch (option_char)
	{
	case 0:
	  break;		// do nothing for these arguments

	case 1:			// debug file
	  {			// scoping
	    // set up default file name
	    CH_STD::string debug_file("chimp.debug");
	    // see if user supplied their own
	    if (optarg)
	    {
	      debug_file.assign(optarg);
	    }
	    // try to set the file for debugging output
	    try
	      {
		CH_CHIMP::debug::get().set_output(debug_file);
	      }
	    catch (CH_CHIMP::bad_file& e)
	      {
		CH_STD::cerr << PACKAGE ":" __FILE__ ":" +
		  CH_CHIMP::t_string(__LINE__) + ":main(): could not open debug "
		  " file " + debug_file + ": " + e.what() << CH_STD::endl;
		// see if we should abort or exit
		assert(0);
		// unsuccessful execution
		CH_STD::exit(1);
	      }
	  }
	  break;

	case 'd':
	  if (optarg)
	    {
	      CH_CHIMP::debug::get().set_level((unsigned int) CH_STD::atoi(optarg));
	    }
	  else
	    {
	      CH_CHIMP::debug::get().set_level(2U);
	    }
	  break;

	case 'h':
	  help = true;
	  break;

	case 'q':
	  CH_CHIMP::debug::get().set_level(0U);
	  break;

	case 'v':
	  // print out package and version (GNITS)
	  CH_STD::cout << CH_CHIMP::version() << CH_STD::endl;
	  CH_STD::exit(0);	// and exit with zero status
	  break;

	case '?':
	  CH_STD::cerr << "Try `" + invoked_as + " --help' for more information."
		       << CH_STD::endl;
	  CH_STD::exit(1);
	  break;

	default:
	  assert(0);		// should never get here
	  // unsuccessful execution
	  CH_STD::exit(1);
	}
    }
  // check for option flags that were set
  if (help)
    {
      // how to invoke the program (GNITS)
      CH_CHIMP::usage(CH_STD::cout, invoked_as);
      CH_STD::exit(0);		// and exit with zero status
    }
  if (CH_CHIMP::debug::get().get_level() > 0U)
    {
      // print out program, version, copyright, and warranty information
      CH_STD::cout << CH_CHIMP::gpl_rant();
    }
  // process non-options on command line, if any
  CH_CHIMP::input_seq input_files;
  while (optind < argc)
    {
      // insert filename into list
      input_files.push_back(argv[optind++]);
    }
  // make sure there at least one control file given
  if (input_files.empty())
    {
      // get control file name from terminal
      CH_STD::cout << PACKAGE ":please enter control file: ";
      CH_STD::string control;
      CH_STD::cin >> control;
      // make sure they just didn't press return
      if (control.size() == 0)
	{
	  CH_STD::cout << PACKAGE ":fine, have it your way... exiting"
		       << CH_STD::endl;
	  CH_STD::exit(1);
	}
      // else
      input_files.push_back(control);
    }

  // perform everything inside a try-catch construct
  try
    {
      // ok to use chimp namespace
      CH_USING_NAMESPACE;
      // get the singleton instance of the task_manager
      task_manager& tm(task_manager::get());
      // call the control file parser for the list of input files
      tm.parse_control(input_files);
      // perform the specified tasks
      tm.perform();
    }
  catch (CH_STD::exception& e)
    {
      CH_STD::cerr << PACKAGE ":" __FILE__ ":" + CH_CHIMP::t_string(__LINE__)
	+ ":main(): an exception of type " + typeid(e).name()
	+ " has been thrown:" << CH_STD::endl
		   << e.what() << CH_STD::endl;
      // see if we should abort or exit
      assert(0);
      // unsuccessful execution
      CH_STD::exit(1);
    }
  catch (...)			// catch anything
    {
      CH_STD::cerr << PACKAGE ":" __FILE__ ":" + CH_CHIMP::t_string(__LINE__)
	+ ":main(): an unknown exception has been thrown" << CH_STD::endl
		   << "This is a bug, please report (see README)"
		   << CH_STD::endl
		   << "Aborting..." << CH_STD::endl;
      CH_STD::abort();
    }
  // terminate program successfully
  CH_STD::exit(0);
}

// define these functions in the chimp namespace
CH_BEGIN_NAMESPACE

// GNITS compliant --version output, no new line
CH_STD::string
version()
{
  // PACKAGE and VERSION defined in config.h
  return PACKAGE " " VERSION;
}

// GNITS compliant --help output
void
usage(CH_STD::ostream& os, const CH_STD::string& program)
{
#ifndef CXX_NAMESPACE_STD_MISSING
  using namespace std;
#endif // not CXX_NAMESPACE_STD_MISSING
  // FIXME: need to list all options
  os << "Usage: " << program << " [OPTIONS]... [FILE]..." << endl
     << "If an argument to a long option is mandatory, it's also mandatory for"
     << endl
     << "the corresponding short option; the same is true for optional arguments."
     << endl << endl
     << "Synopsis: " PACKAGE " performs kinetic modeling on reaction mechanisms."
     << endl << endl
     << "Options:" << endl
     << "  -d, --debug[=N]    set debug level to N, default 2" << endl
     << "  --debug-file[=X]   debug output to file, default `chimp.debug'" << endl
     << "  -h, --help         display this help and exit" << endl
     << "  -q, --quiet        do not output any information" << endl
     << "  --silent           same as `--quiet'" << endl
     << "  -v, --version      output version information and exit" << endl
     << endl
     << "FILE is the master input file, see manual for details." << endl
     << endl
     << "Report bugs to http://sourceforge.net/projects/chimp/." << endl;
  return;
}

CH_STD::string
gpl_rant()
{
  return version() + "\n"
    "Copyright (C) 2004 David Dooling <banjo@users.sourceforge.net>\n\n"
    "There is ABSOLUTELY NO WARRANTY; see file COPYING for details.\n"
    "This is free software, and you are welcome to redistribute it\n"
    "under certain conditions; see file COPYING for details.\n\n"
    "Report bugs to http://sourceforge.net/projects/chimp/.\n";
}

CH_END_NAMESPACE

/* $Id: chimp.cc,v 1.1.1.1 2004/11/25 20:24:05 banjo Exp $ */
