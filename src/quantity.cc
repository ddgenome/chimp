// Methods for manipulation of species quantites.
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

#include "quantity.h"
#include <typeinfo>
#include "compare.h"
#include "t_string.h"

// set namespace to avoid possible clashes
CH_BEGIN_NAMESPACE

// quantity methods
// ctor: (default) get the precision singleton
quantity::quantity()
  : prec(precision::get())
{}

// ctor: copy, do nothing
quantity::quantity(const quantity& original)
  : prec(original.prec)
{}

// dtor: do nothing
quantity::~quantity()
{}

// return false if argument is not an acceptable type
bool
quantity::is_type(type type_)
{
  if (type_ == Econcentration || type_ == Epressure || type_ == Eflow)
    {
      return true;
    }
  return false;
}

// return the enumeration of the given type
quantity::type
quantity::get_type(const CH_STD::string& type_string)
  throw (bad_type)
{
  if (icompare(type_string, "concentration") == 0)
    {
      return Econcentration;
    }
  else if (icompare(type_string, "pressure") == 0)
    {
      return Epressure;
    }
  else if (icompare(type_string, "flow") == 0)
    {
      return Eflow;
    }
  else				// not a recognized type
    {
      throw bad_type(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		     ":quantity::get_type(): unknown quantity type requested: "
		     + type_string);
    }
}

// surface_quantity methods
// ctor: set coverage to zero
surface_quantity::surface_quantity()
  : coverage(0.0e0)
{}

// ctor: copy
surface_quantity::surface_quantity(const surface_quantity& original)
  : quantity(original), coverage(original.coverage)
{}

// dtor: do nothing
surface_quantity::~surface_quantity()
{}

// surface_quantity private methods
// return current COVERAGE
double
surface_quantity::get_coverage() const
{
  return coverage;
}

// set the coverage, return old coverage
// defaults coverage_ = 0.0e0
double
surface_quantity::set_coverage(double coverage_)
  throw (bad_value)
{
  if (coverage_ < 0.0e0 - prec.get_coverage()
      || coverage_ > 1.0e0 + prec.get_coverage())
    {
      // throw exception
      throw bad_value(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		      ":surface_quantity::set_coverage(): coverage value (" +
		      t_string(coverage_) + ") is not in range [0, 1]");
    }
  double old(coverage);
  coverage = coverage_;
  return old;
}

// surface_quantity public methods
// return pointer to copy of same type
quantity*
surface_quantity::copy()
{
  return new surface_quantity(*this);
}

// return coverage, ignore type argument
double
surface_quantity::get_quantity(type type_) const
{
  // ignore type_
  return get_coverage();
}

// return coverage, ignore type argument
double
surface_quantity::get_quantity(const CH_STD::string& type_) const
{
  // ignore type_
  return get_coverage();
}

// set the coverage, ignore type_, return old value
double
surface_quantity::set_quantity(type type_, double amount)
  throw (bad_value)
{
  // ignore type_
  return set_coverage(amount);
}

// set the coverage, ignore type_, return old value
double
surface_quantity::set_quantity(const CH_STD::string& type_, double amount)
  throw (bad_value)
{
  // ignore type_
  return set_coverage(amount);
}

// add INCREMENT to coverage, ignore type_, return new value
// default increment = 0.0e0
double
surface_quantity::add_to_quantity(type type_, double increment)
  throw (bad_value)
{
  return set_coverage(get_coverage() + increment);
}

// set all values (just coverage in this case) to zero
void
surface_quantity::zero_quantity()
{
  // ignore return value
  set_coverage();		// zero is default
  return;
}

// fluid_quantity methods
// ctor: (default) set values to zero
fluid_quantity::fluid_quantity()
  : concentration(0.0e0), pressure(0.0e0), flow(0.0e0)
{}

// ctor: copy
fluid_quantity::fluid_quantity(const fluid_quantity& original)
  : quantity(original), concentration(original.concentration),
    pressure(original.pressure), flow(original.flow)
{}

// dtor: do nothing
fluid_quantity::~fluid_quantity()
{}

// fluid_quantity private methods
// return current CONCENTRATION
double
fluid_quantity::get_concentration() const
{
  return concentration;
}

// return current PRESSURE
double
fluid_quantity::get_pressure() const
{
  return pressure;
}

// return current FLOW
double
fluid_quantity::get_flow() const
{
  return flow;
}

// set the concentration, return old concentration
// defaults concentration_ = 0.0e0
double
fluid_quantity::set_concentration(double concentration_)
  throw (bad_value)
{
  if (concentration_ < 0.0e0 - prec.get_concentration())
    {
      // throw exception
      throw bad_value(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		      ":fluid_quantity::set_concentration(): concentration "
		      "value (" + t_string(concentration_) + ") is less than "
		      "zero (0.0e0)");
    }
  double old(concentration);
  concentration = concentration_;
  return old;
}

// set the pressure, return old pressure
// defaults pressure_ = 0.0e0
double
fluid_quantity::set_pressure(double pressure_)
  throw (bad_value)
{
  if (pressure_ < 0.0e0 - prec.get_pressure())
    { 
      // throw exception
      throw bad_value(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		      ":fluid_quantity::set_pressure(): pressure value (" +
		      t_string(pressure_) + ") is less than zero (0.0e0)");
    }
  double old(pressure);
  pressure = pressure_;
  return old;
}

// set the flow, return old flow
// defaults flow_ = 0.0e0
double
fluid_quantity::set_flow(double flow_)
{
  // flow < zero is ok
  double old(flow);
  flow = flow_;
  return old;
}

// fluid_quantity public methods
// return pointer to copy of same type
quantity*
fluid_quantity::copy()
{
  return new fluid_quantity(*this);
}

// get the specified quantity
double
fluid_quantity::get_quantity(type type_) const
  throw (bad_type)
{
  switch(type_)
    {
    case Econcentration:
      return get_concentration();
      break;
    case Epressure:
      return get_pressure();
      break;
    case Eflow:
      return get_flow();
      break;
    default:			// unknown type
      throw bad_type(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		     ":quantity::get_quantity(): unknown quantity type, not "
		     "concetration, pressure, or flow");
      break;
    }
}

// get the specified quantity
double
fluid_quantity::get_quantity(const CH_STD::string& type_) const
  throw (bad_type)
{
  if (type_ == "concentration")
    {
      return get_quantity(Econcentration);
    }
  else if (type_ == "pressure")
    {
      return get_quantity(Epressure);
    }
  else if (type_ == "flow")
    {
      return get_quantity(Eflow);
    }
  else // invalid type_
    {
      throw bad_type(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		     ":quantity::get_quantity(): unknown quantity type - "
		     + type_);
    }
}

// set the specified quantity
// defaults amount = 0.0e0
double
fluid_quantity::set_quantity(type type_, double amount)
  throw (bad_type, bad_value)
{
  switch(type_)
    {
    case Econcentration:
      return set_concentration(amount);
      break;
    case Epressure:
      return set_pressure(amount);
      break;
    case Eflow:
      return set_flow(amount);
      break;
    default:			// unknown type
      throw bad_type(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		     ":quantity::set_quantity(): unknown quantity type, not "
		     "concentration, pressure, or flow");
      break;
    }
}

// set the specified quantity
// defaults amount = 0.0e0
double
fluid_quantity::set_quantity(const CH_STD::string& type_, double amount)
  throw (bad_type, bad_value)
{
  if (type_ == "concentration")
    {
      return set_quantity(Econcentration, amount);
    }
  else if (type_ == "pressure")
    {
      return set_quantity(Epressure, amount);
    }
  else if (type_ == "flow")
    {
      return set_quantity(Eflow, amount);
    }
  else // invalid type_
    {
      throw bad_type(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		     ":quantity::set_quantity(): unknown quantity type: "
		     + type_);
    }
}

// add INCREMENT to the amount of type TYPE_
// defaults increment = 0.0e0
double
fluid_quantity::add_to_quantity(type type_, double increment)
  throw (bad_type, bad_value)
{
  switch(type_)
    {
    case Econcentration:
      return set_concentration(get_concentration() + increment);
      break;
    case Epressure:
      return set_pressure(get_pressure() + increment);
      break;
    case Eflow:
      return set_flow(get_flow() + increment);
      break;
    default:			// unknown type
      throw bad_type(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		     ":quantity::add_to_quantity(): unknown quantity type, "
		     "not concentration, pressure, or flow");
      break;
    }
}

// set all values to zero
void
fluid_quantity::zero_quantity()
{
  // ignore return values, zero is default argument
  set_concentration();
  set_pressure();
  set_flow();
  return;
}

CH_END_NAMESPACE

/* $Id: quantity.cc,v 1.1.1.1 2004/11/25 20:24:06 banjo Exp $ */
