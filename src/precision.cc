// Methods for manipulating calculation precision.
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

#include "precision.h"
#include <cfloat>

// set namespace to avoid possible clashes
CH_BEGIN_NAMESPACE

// instantiate static singleton instance
precision precision::single;

// precision class methods
// ctor: set default values
precision::precision()
  : dbl_precision(DBL_EPSILON), pressure(DBL_EPSILON),
    concentration(DBL_EPSILON), flow(DBL_EPSILON), coverage(DBL_EPSILON)
{}

// dtor: do nothing
precision::~precision()
{}

// return reference to singleton instance
precision&
precision::get()
{
  return single;
}

// return desired precision for double calculations
double
precision::get_double() const
{
  return dbl_precision;
}

// return desired precision for pressure
double
precision::get_pressure() const
{
  return pressure;
}

// return desired precision for concentration
double
precision::get_concentration() const
{
  return concentration;
}

// return desired precision for flow
double
precision::get_flow() const
{
  return flow;
}

// return desired precision for coverage
double
precision::get_coverage() const
{
  return coverage;
}

// change the double precision, return old value
double
precision::set_double(double dbl_precision_)
{
  double old(dbl_precision);
  // make sure new values is ok
  dbl_precision = (dbl_precision_ < DBL_EPSILON) ? DBL_EPSILON : dbl_precision_;
  return old;
}

// change the pressure precision, return old value
double
precision::set_pressure(double pressure_)
{
  double old(pressure);
  // make sure new values is ok
  pressure = (pressure_ < DBL_EPSILON) ? DBL_EPSILON : pressure_;
  return old;
}

// change the concentration precision, return old value
double
precision::set_concentration(double concentration_)
{
  double old(concentration);
  // make sure new values is ok
  concentration = (concentration_ < DBL_EPSILON) ? DBL_EPSILON : concentration_;
  return old;
}

// change the flow precision, return old value
double
precision::set_flow(double flow_)
{
  double old(flow);
  // make sure new values is ok
  flow = (flow_ < DBL_EPSILON) ? DBL_EPSILON : flow_;
  return old;
}

// change the coverage precision, return old value
double
precision::set_coverage(double coverage_)
{
  double old(coverage);
  // make sure new values is ok
  coverage = (coverage_ < DBL_EPSILON) ? DBL_EPSILON : coverage_;
  return old;
}

CH_END_NAMESPACE

/* $Id: precision.cc,v 1.1.1.1 2004/11/25 20:24:06 banjo Exp $ */
