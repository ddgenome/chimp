// -*- C++ -*-
// Exceptions thrown by CHIMP.
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
#ifndef CH_EXCEPT_H
#define CH_EXCEPT_H 1

#include <stdexcept>		// includes <exception>

// set namespace to avoid possible clashes
CH_BEGIN_NAMESPACE

// make exceptions inline to parallel those is the standard library

// throw when an invalid value is assigned to a variable
class bad_value : public CH_STD::invalid_argument
{
public:
  // ctor: mirrors that of base class
  bad_value(const CH_STD::string& what_arg)
    : CH_STD::invalid_argument(what_arg)
    {}
};

// throw when two instances of a class are created with same name
class bad_name : public CH_STD::invalid_argument
{
public:
  // ctor: mirrors that of the base class
  bad_name(const CH_STD::string& what_arg)
    : CH_STD::invalid_argument(what_arg)
    {}
};

// throw when an invalid type of parameter, reactor, etc. is specified
class bad_type : public CH_STD::invalid_argument
{
public:
  // ctor: mirrors that of the base class
  bad_type(const CH_STD::string& what_arg)
    : CH_STD::invalid_argument(what_arg)
    {}
};

// throw when the user-input is faulty
class bad_input : public CH_STD::logic_error
{
public:
  // ctor: mirrors that of the base class
  bad_input(const CH_STD::string& what_arg)
    : CH_STD::logic_error(what_arg)
    {}
};

// throw when the given file name is invalid
class bad_file : public CH_STD::invalid_argument
{
public:
  // ctor: mirrors that of the base class
  bad_file(const CH_STD::string& what_arg)
    : CH_STD::invalid_argument(what_arg)
    {}
};

// throw when method accesses a null pointer or refers to an invalid
// object
class bad_pointer : public CH_STD::domain_error
{
public:
  // ctor: mirrors that of the base class
  bad_pointer(const CH_STD::string& what_arg)
    : CH_STD::domain_error(what_arg)
    {}
};

// throw when the program is asked to do something it shouldn't
class bad_request : public CH_STD::logic_error
{
public:
  // ctor: mirrors that of the base class
  bad_request(const CH_STD::string& what_arg)
    : CH_STD::logic_error(what_arg)
    {}
};

CH_END_NAMESPACE

#endif // not CH_EXCEPT_H

/* $Id: except.h,v 1.1.1.1 2004/11/25 20:24:05 banjo Exp $ */
