// -*- C++ -*-
// Class which sets the desired precision for the calculation.
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
#ifndef CH_PRECISION_H
#define CH_PRECISION_H 1

// set namespace to avoid possible clashes
CH_BEGIN_NAMESPACE

// class with single instance which provides methods with precision desired
class precision
{
  static precision single;	// singleton instance
  double dbl_precision;		// precision for generic doubles
  double pressure;		// precision for pressure
  double concentration;		// precision for concentration
  double flow;			// precision for flow
  double coverage;		// precision for coverage

private:
  // declare all ctors private
  // ctor: set default values
  precision();
  // ctor: prevent copy construction and assignment
  precision(const precision&);
  precision& operator=(const precision&);
public:
  // dtor: do nothing
  ~precision();

  // return reference to singleton instance
  static precision& get();
  // return desired precision for double calculations
  double get_double() const;
  // return desired precision for pressure
  double get_pressure() const;
  // return desired precision for concentration
  double get_concentration() const;
  // return desired precision for flow
  double get_flow() const;
  // return desired precision for coverage
  double get_coverage() const;
  // change the double precision, return old value
  double set_double(double dbl_precision_);
  // change the pressure precision, return old value
  double set_pressure(double pressure_);
  // change the concentration precision, return old value
  double set_concentration(double concentration_);
  // change the flow precision, return old value
  double set_flow(double flow_);
  // change the coverage precision, return old value
  double set_coverage(double coverage_);
}; // end class precision

CH_END_NAMESPACE

#endif // not CH_PRECISION_H

/* $Id: precision.h,v 1.1.1.1 2004/11/25 20:24:06 banjo Exp $ */
