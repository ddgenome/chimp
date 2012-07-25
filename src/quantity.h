// -*- C++ -*-
// Classes to convert pressures, concentrations, and flow rates.
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
#ifndef CH_QUANTITY_H
#define CH_QUANTITY_H 1

#include <string>
#include "except.h"
#include "precision.h"

// set namespace to avoid possible clashes
CH_BEGIN_NAMESPACE

// These classes contain values of pressures, concentrations and flow
// rates.  By themself, they do not have enough information to do
// conversions between the quantity types, so it must be done in the
// class which contains quantity and reactor.
class quantity
{
protected:
  precision& prec;		// reference to singleton precision object
public:
  // enum type to use to make comparisons faster (refer to as quantity::Efoo)
  enum type { Econcentration, Epressure, Eflow };

private:
  // prevent assignment
  quantity& operator=(const quantity&);
public:
  // ctor: (default) get the precision singleton
  quantity();
  // ctor: copy, do nothing
  quantity(const quantity& original);
  // dtor: do nothing
  virtual ~quantity();

  // return false if argument is not an acceptable type (is static ok?)
  static bool is_type(type type_);
  // return the enumeration of the given type
  static type get_type(const CH_STD::string& type_string)
    throw (bad_type); // this
  // return pointer to copy of same type
  virtual quantity* copy() = 0;
  // return quantity specified by the argument
  virtual double get_quantity(type type_) const = 0;
  virtual double get_quantity(const CH_STD::string& type_) const = 0;
  // set the specified quantity, return old amount
  virtual double set_quantity(type type_, double amount = 0.0e0) = 0;
  virtual double set_quantity(const CH_STD::string& type_,
			      double amount = 0.0e0) = 0;
  // add INCREMENT to the amount of type TYPE_, return new value
  virtual double add_to_quantity(type type_, double increment = 0.0e0) = 0;
  // set all values to zero
  virtual void zero_quantity() = 0;
}; // end class quantity

class surface_quantity : public quantity
{
  double coverage;		// fractional surface coverage, [0, 1]

private:
  // prevent assignment
  surface_quantity& operator=(const surface_quantity&);
  // return current COVERAGE
  double get_coverage() const;
  // set coverage, return old coverage
  double set_coverage(double coverage_ = 0.0e0)
    throw (bad_value); // this
public:
  // ctor: set coverage to zero
  surface_quantity();
  // ctor: copy
  surface_quantity(const surface_quantity& original);
  // dtor: do nothing
  virtual ~surface_quantity();

  // return pointer to copy of same type
  virtual quantity* copy();
  // return coverage, ignore type argument
  virtual double get_quantity(type type_) const;
  virtual double get_quantity(const CH_STD::string& type_) const;
  // set the specified quantity, return old amount
  virtual double set_quantity(type type_, double amount = 0.0e0)
    throw (bad_value); // set_coverage()
  virtual double set_quantity(const CH_STD::string& type_,
			      double amount = 0.0e0)
    throw (bad_value); // set_coverage()
  // add INCREMENT to coverage, ignore type_, return new coverage
  virtual double add_to_quantity(type type_, double increment = 0.0e0)
    throw (bad_value); // set_coverage()
  // set all values to zero
  virtual void zero_quantity();
}; // end class surface_quantity

class fluid_quantity : public quantity
{
  double concentration;		// concentration of species
  double pressure;		// pressure of species
  // put flow in here so we don't have to create a new quantity when
  // switching between batch and flow reactors in the same model
  double flow;			// flow rate of species

private:
  // prevent assignment
  fluid_quantity& operator=(const fluid_quantity&);
  // return current CONCENTRATION
  double get_concentration() const;
  // return current PRESSURE
  double get_pressure() const;
  // return current FLOW
  double get_flow() const;
  // set the concentration, return old concentration
  double set_concentration(double concentration_ = 0.0e0)
    throw (bad_value); // this
  // set the pressure, return old pressure
  double set_pressure(double pressure_ = 0.0e0)
    throw (bad_value); // this
  // set the flow, return old flow
  double set_flow(double flow_ = 0.0e0);
public:
  // ctor: (default) set all values to zero
  fluid_quantity();
  // copy: copy
  fluid_quantity(const fluid_quantity& original);
  // dtor: do nothing
  virtual ~fluid_quantity();

  // return pointer to copy of same type
  virtual quantity* copy();
  // return quantity specified by the argument
  virtual double get_quantity(type type_) const
    throw (bad_type); // this
  virtual double get_quantity(const CH_STD::string& type_) const
    throw (bad_type); // this
  // set the specified quantity, return old amount
  virtual double set_quantity(type type_, double amount = 0.0e0)
    throw (bad_type, bad_value); // this, set_concentration(), set_pressure()
  virtual double set_quantity(const CH_STD::string& type_,
			      double amount = 0.0e0)
    throw (bad_type, bad_value); // this, set_concentration(), set_pressure()
  // add INCREMENT to the amount of type TYPE_, return new value
  virtual double add_to_quantity(type type_, double increment = 0.0e0)
    throw (bad_type, bad_value); // this, set_concentration(), set_pressure()
  // set all values to zero
  virtual void zero_quantity();
}; // end class fluid_quantity

CH_END_NAMESPACE

#endif // not CH_QUANTITY_H

/* $Id: quantity.h,v 1.1.1.1 2004/11/25 20:24:06 banjo Exp $ */
