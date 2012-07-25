// -*- C++ -*-
// Memory management functions.
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
#ifndef CH_HANDLER_H
#define CH_HANDLER_H 1

#include <exception>
#include <new>
#include <string>

// set namespace to avoid possible clashes
CH_BEGIN_NAMESPACE

// specific unavailable memory exception
class bad_allocation : public CH_STD::bad_alloc
{
  CH_STD::string _what;
public:
  bad_allocation(const CH_STD::string& what_arg)
    : CH_STD::bad_alloc(), _what(what_arg) {}
  ~bad_allocation() throw () {}
  virtual const char* what() const throw () { return _what.c_str(); }
}; // end class bad_allocation

// class to hand memory allocation errors
// note: have to make everything static or we can't set the new-handler in the
//       ctor; is there a way around this?
//       because of this, there is no need to create a singleton or a function
//       that returns a reference to the singleton
class handler
{
  static CH_STD::new_handler orig; // original new-handler (probably 0)
  static CH_STD::string info;	// error information

private:
  // make all ctor's private to ensure only one instance is created
  handler();
  handler(const handler&);
  handler& operator=(const handler&);
public:
  // function operator to be called as new-handler: void (*f)()
  static void no_memory()
    throw(bad_allocation); // this
  static void set_info(const CH_STD::string& info_);
}; // end class handler

// class to handle unexpected exceptions
class unexpected
{
  static CH_STD::unexpected_handler orig; // original unexpected handler (0)

private:
  // make all ctor's private
  unexpected();
  unexpected(const unexpected&);
  unexpected& operator=(const unexpected&);
public:
  // function to be called when an unexpected exception is thrown
  static void not_expected();
}; // end class unexpected

CH_END_NAMESPACE

#endif // not CH_HANDLER_H

/* $Id: handler.h,v 1.1.1.1 2004/11/25 20:24:05 banjo Exp $ */
