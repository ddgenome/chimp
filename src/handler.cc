// Better ways to deal with memory failures.
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

#include "handler.h"
#include <cstdlib>
#include <iostream>
#include "t_string.h"

// set namespace to avoid possible clashes
CH_BEGIN_NAMESPACE

// initialize static instance variables of handler
CH_STD::new_handler handler::orig(CH_STD::set_new_handler(handler::no_memory));
CH_STD::string handler::info("new:");
CH_STD::unexpected_handler unexpected::orig(CH_STD::set_unexpected(unexpected::not_expected));

// handler methods
// this is the new-handler
void
handler::no_memory()
  throw(bad_allocation) // this
{
  throw bad_allocation(info + "memory allocation error");
}

void
handler::set_info(const CH_STD::string& info_)
{
  info = info_;
}

// unexpected methods
// function to be called when an unexpected exception is thrown
void
unexpected::not_expected()
{
  CH_STD::cerr << PACKAGE ":" __FILE__ ":" + t_string(__LINE__)
    + ":unexpected::not_expected(): an exception has been thrown "
    "which was not specified in the exception specification" << CH_STD::endl
	       << "This is a bug, please report (see README)" << CH_STD::endl
	       << "Aborting..." << CH_STD::endl;
  CH_STD::abort();
  return;
}

CH_END_NAMESPACE

/* $Id: handler.cc,v 1.1.1.1 2004/11/25 20:24:05 banjo Exp $ */
